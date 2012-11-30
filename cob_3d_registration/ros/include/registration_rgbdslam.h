/*!
 *****************************************************************
 * \file
 *
 * \note
 *   Copyright (c) 2012 \n
 *   Fraunhofer Institute for Manufacturing Engineering
 *   and Automation (IPA) \n\n
 *
 *****************************************************************
 *
 * \note
 *  Project name: TODO FILL IN PROJECT NAME HERE
 * \note
 *  ROS stack name: TODO FILL IN STACK NAME HERE
 * \note
 *  ROS package name: TODO FILL IN PACKAGE NAME HERE
 *
 * \author
 *  Author: TODO FILL IN AUTHOR NAME HERE
 * \author
 *  Supervised by: TODO FILL IN CO-AUTHOR NAME(S) HERE
 *
 * \date Date of creation: TODO FILL IN DATE HERE
 *
 * \brief
 *****************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     - Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer. \n
 *     - Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution. \n
 *     - Neither the name of the Fraunhofer Institute for Manufacturing
 *       Engineering and Automation (IPA) nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission. \n
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License LGPL as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License LGPL for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License LGPL along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************************/
/*
 * registration_rgbdslam.h
 *
 *  Created on: Nov 15, 2011
 *      Author: goa-jh
 */

#ifndef REGISTRATION_RGBDSLAM_H_
#define REGISTRATION_RGBDSLAM_H_

#include <registration/general_registration.h>
#include <pcl/point_cloud.h>
#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <cv_bridge/cv_bridge.h>
#include <tf/transform_datatypes.h>
#include <tf/tfMessage.h>
#include <ros/console.h>
#include <geometry_msgs/TransformStamped.h>

/**
 *
 * class which wraps the interface to rgbd slam
 *
 * topics needed:
 *      - /camera/rgb/image_mono
 *      - /camera/depth/image
 *      - /camera/rgb/camera_info
 *      - /camera/rgb/points
 * topics output:
 *      - /rgbdslam/cloud
 *      - /rgbdslam/transformed_slowdown_cloud
 *      - /rgbdslam/first_frame
 *      - /rgbdslam/transforms
 */
template<typename Point>
class Registration_RGBDSLAM : public GeneralRegistration<Point>
{

  bool got_tranf_;

public:
  Registration_RGBDSLAM(ros::NodeHandle &nh)
  {
    pub_img_ = nh.advertise<sensor_msgs::Image> (
        "rgbdslam/image_mono", 5);
    pub_img_depth_ = nh.advertise<sensor_msgs::Image> (
        "rgbdslam/image", 5);
    pub_camera_info_ = nh.advertise<sensor_msgs::CameraInfo> (
        "rgbdslam/camera_info", 5);
    pub_cloud_ = nh.advertise<sensor_msgs::PointCloud2> (
        "rgbdslam/points", 5);

    sub_trans_ = nh.subscribe("/tf", 1, &Registration_RGBDSLAM::transformationCallback, this);

  }

  boost::shared_ptr<pcl::PointCloud<Point> > getMap() {return register_.makeShared();}

protected:

  virtual bool compute_features() {return true;}
  virtual bool compute_corrospondences() {return true;}
  virtual bool compute_transformation();

  /// get last transformation and calculate the transformation matrix
  void transformationCallback(const tf::tfMessage &transform) {
    ROS_INFO("got transformation");
	#ifdef PCL_VERSION_COMPARE
	#else
    if(!transform.get_transforms_size())
      return;
	#endif

    std::vector< geometry_msgs::TransformStamped> tfs;
	#ifdef PCL_VERSION_COMPARE
	  tfs=transform.transforms;
	#else
      transform.get_transforms_vec(tfs);
	#endif

    Eigen::Matrix3f resR = Eigen::Matrix3f::Identity();
    Eigen::Vector3f rest;
    rest(0)=rest(1)=rest(2)=0.f;

    for(int i=0; i<tfs.size(); i++) {

      tf::StampedTransform T;
      tf::transformStampedMsgToTF(tfs[i], T);

      Eigen::Quaternionf q;
      q.w() = T.getRotation().getW();
      q.x() = T.getRotation().getX();
      q.y() = T.getRotation().getY();
      q.z() = T.getRotation().getZ();
      Eigen::Matrix3f R=q.toRotationMatrix();
      Eigen::Vector3f t;
      t(0)=T.getOrigin().getX();
      t(1)=T.getOrigin().getY();
      t(2)=T.getOrigin().getZ();

      try {
        if(R.diagonal().squaredNorm()<0.99)
          R=R.inverse().eval();
        t=-t;
      }
      catch(...)
      {
        return;
      }

      resR = R*resR;
      rest = rest+t;

      std::cout<<resR<<"\n";
    }

    Eigen::Vector3f vZ, vX;
    vZ(0)=0;
    vZ(1)=0;
    vZ(2)=1;
    vX(0)=1;
    vX(1)=0;
    vX(2)=0;
    Eigen::AngleAxisf a1(M_PI*0.5,vZ);
    Eigen::AngleAxisf a2(-M_PI*0.5,vX);

    rest = a1._transformVector(rest);
    rest = a2._transformVector(rest);
    resR = a1*resR;
    resR = a2*resR;

    resR.col(1)=-1*resR.col(1);

    for(int i=0; i<3; i++)
      this->transformation_.col(3)(i) = rest(i);
    for(int i=0; i<3; i++)
      for(int j=0; j<3; j++)
        this->transformation_(i,j) = resR(i,(j+1)%3);
    std::cout<<this->transformation_<<"\n";

    got_tranf_=true;

  }

  //internal states
  pcl::PointCloud<Point> register_;

  ros::Publisher pub_img_,  pub_img_depth_, pub_camera_info_, pub_cloud_;
  ros::Subscriber sub_trans_;
};

#include "impl/registration_rgbdslam.hpp"

#endif /* REGISTRATION_RGBDSLAM_H_ */
