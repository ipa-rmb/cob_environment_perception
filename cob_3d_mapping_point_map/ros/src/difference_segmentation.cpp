/****************************************************************
 *
 * Copyright (c) 2010
 *
 * Fraunhofer Institute for Manufacturing Engineering
 * and Automation (IPA)
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Project name: care-o-bot
 * ROS stack name: cob_environment_perception
 * ROS package name: cob_3d_mapping_point_cloud
 * Description:
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Author: Georg Arbeiter, email:georg.arbeiter@ipa.fhg.de
 * Supervised by: Georg Arbeiter, email:georg.arbeiter@ipa.fhg.de
 *
 * Date of creation: 12/2011
 * ToDo:
 *
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Fraunhofer Institute for Manufacturing
 *       Engineering and Automation (IPA) nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License LGPL as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License LGPL for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License LGPL along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************************/


//##################
//#### includes ####

// ROS includes
//#include <ros/ros.h>
#include <ros/console.h>
//#include <pcl/io/pcd_io.h>
//#include <pcl/point_types.h>
//#include <pcl_ros/point_cloud.h>
#include <pcl_ros/pcl_nodelet.h>
#include <pcl_ros/transforms.h>
#include <pcl/io/io.h>
#include <pluginlib/class_list_macros.h>
#include <tf/transform_listener.h>
#include <tf_conversions/tf_eigen.h>

#include <message_filters/subscriber.h>
#include <message_filters/synchronizer.h>
//#include <message_filters/sync_policies/approximate_time.h>
#include <pcl/segmentation/segment_differences.h>
#include <pcl/kdtree/kdtree_flann.h>




using namespace tf;

//####################
//#### node class ####
class DifferenceSegmentation : public pcl_ros::PCLNodelet//, protected Reconfigurable_Node<cob_3d_mapping_point_map::point_map_nodeletConfig>
{
  typedef pcl::PointXYZRGB Point;
  typedef pcl::PointCloud<Point> PointCloud;
  typedef message_filters::sync_policies::ExactTime<PointCloud, PointCloud > MySyncPolicy;

public:
  // Constructor
  DifferenceSegmentation()
  //: Reconfigurable_Node<cob_3d_mapping_point_map::point_map_nodeletConfig>("PointMapNodelet")
    {
    //setReconfigureCallback2(boost::bind(&callback, this, _1, _2), boost::bind(&callback_get, this, _1));
    }


  // Destructor
  ~DifferenceSegmentation()
  {
    /// void
  }



  /**
   * @brief initializes parameters
   *
   * initializes parameters
   *
   * @return nothing
   */
  void
  onInit()
  {
    PCLNodelet::onInit();
    n_ = getNodeHandle();

    map_diff_pub_ = n_.advertise<PointCloud >("output",1);
    //diff_maps_pub_ = n_.advertise<PointCloud >("diff_maps",1);
    map_sub_.subscribe(n_,"target",10);
    pc_aligned_sub_.subscribe(n_,"input",10);
    sync_ = boost::make_shared <message_filters::Synchronizer<MySyncPolicy> >(10);
    sync_->connectInput(map_sub_, pc_aligned_sub_);
    sync_->registerCallback(boost::bind(&DifferenceSegmentation::pointCloudSubCallback, this, _1, _2));

#ifdef PCL_VERSION_COMPARE //fuerte
    pcl::search::KdTree<Point>::Ptr tree (new pcl::search::KdTree<Point> ());
#else //electric
    pcl::KdTreeFLANN<Point>::Ptr tree (new pcl::KdTreeFLANN<Point> ());
#endif
    sd_.setSearchMethod(tree);
    sd_.setDistanceThreshold(0.01);
  }

  /**
   * @brief callback for point cloud subroutine
   *
   * callback for point cloud subroutine which stores the point cloud
   * for further calculation
   *
   * @param pc_in  new point cloud
   *
   * @return nothing
   */
  void
  pointCloudSubCallback(const PointCloud::ConstPtr& map, const PointCloud::ConstPtr& pc_aligned)
  {
    PointCloud::Ptr pc_trans(new PointCloud);
    tf::StampedTransform trf_map;
    try
    {
      std::stringstream ss2;
      tf_listener_.waitForTransform(map->header.frame_id, pc_aligned->header.frame_id, pc_aligned->header.stamp, ros::Duration(2.0));
      tf_listener_.lookupTransform(map->header.frame_id, pc_aligned->header.frame_id, pc_aligned->header.stamp, trf_map);
    }
    catch (tf::TransformException ex)
    {
      ROS_ERROR("[difference_segmentation] : %s",ex.what());
      return;
    }
    Eigen::Affine3d af;
    tf::TransformTFToEigen(trf_map, af);
    Eigen::Matrix4f trf = af.matrix().cast<float>();
    pcl::transformPointCloud(*pc_aligned, *pc_trans, trf);

    PointCloud pc_diff;
    /*std::cout << "map size:" << map->size() << std::endl;
    std::cout << map->header.stamp << std::endl;
    std::cout << "pc_aligned size:" << pc_aligned->size() << std::endl;
    std::cout << pc_aligned->header.stamp << std::endl;*/
    /*if(map->header.frame_id != pc_aligned->header.frame_id)
    {
      ROS_ERROR("Frame IDs do not match, aborting...");
      return;
    }*/
    //diff_maps_.header.frame_id=map->header.frame_id;
    sd_.setTargetCloud(map_.makeShared());
    sd_.setInputCloud(pc_trans);
    sd_.segment(pc_diff);
    pc_diff.header = map->header;
    std::cout << pc_diff.size() << std::endl;
    if(pc_diff.size()>0)
    {
      map_diff_pub_.publish(pc_diff);
    }
    pcl::copyPointCloud(*map, map_);
  }


  ros::NodeHandle n_;


protected:
  message_filters::Subscriber<PointCloud > map_sub_;
  message_filters::Subscriber<PointCloud > pc_aligned_sub_;
  ros::Publisher map_diff_pub_;		//publisher for map
  //ros::Publisher diff_maps_pub_;
  boost::shared_ptr<message_filters::Synchronizer<MySyncPolicy> > sync_;

  TransformListener tf_listener_;

  pcl::PointCloud<Point> map_;
  //pcl::PointCloud<Point> diff_maps_;
  pcl::SegmentDifferences<Point> sd_;


};

PLUGINLIB_DECLARE_CLASS(cob_3d_mapping_point_map, DifferenceSegmentation, DifferenceSegmentation, nodelet::Nodelet)

