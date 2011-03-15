/*
 * cpc_point.h
 *
 *  Created on: 26.08.2010
 *      Author: goa
 */

#ifndef CPC_POINT_H_
#define CPC_POINT_H_

#include <pcl/ros/register_point_struct.h>
#include <pcl/point_types.h>
#include <ros/ros.h>

using namespace pcl;

struct CPCPoint
{
	PCL_ADD_POINT4D;
	//uint32_t rgb;

	float confidence;
	float intensity;
	//uint8_t isFeature;
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
} EIGEN_ALIGN16;


POINT_CLOUD_REGISTER_POINT_STRUCT(
  CPCPoint,
  (float, x, x)
  (float, y, y)
  (float, z, z)
  //(uint32_t, rgb, rgb)
  (float, confidence, confidence)
  (float, intensity, intensity));
  //(uint8_t, isFeature, features));

#endif /* CPC_POINT_H_ */
