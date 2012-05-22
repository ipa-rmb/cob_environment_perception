/****************************************************************
 *
 * Copyright (c) 2011
 *
 * Fraunhofer Institute for Manufacturing Engineering
 * and Automation (IPA)
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Project name: care-o-bot
 * ROS stack name: cob_environment_perception_intern
 * ROS package name: cob_3d_mapping_features
 * Description:
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Author: Georg Arbeiter, email:georg.arbeiter@ipa.fhg.de
 * Supervised by: Georg Arbeiter, email:georg.arbeiter@ipa.fhg.de
 *
 * Date of creation: 03/2012
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

// Boost:
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

// PCL:
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/visualization/point_cloud_handlers.h>

// Stack Includes:
#include "cob_3d_mapping_common/stop_watch.h"
#include "cob_3d_mapping_common/point_types.h"
#include "cob_3d_mapping_common/label_defines.h"
#include "cob_3d_mapping_features/organized_normal_estimation.h"
#include "cob_3d_mapping_features/organized_curvature_estimation.h"
#include "cob_3d_mapping_features/depth_segmentation.h"
#include "cob_3d_mapping_features/cluster_classifier.h"

using namespace std;
using namespace pcl;

string file_in_;
float d_th_, upper_;

void readOptions(int argc, char* argv[])
{
  using namespace boost::program_options;
  options_description options("Options");
  options.add_options()
    ("help", "produce help message")
    ("in,i", value<string>(&file_in_), "input pcd file")
    ("skip_distant_point,d", value<float>(&d_th_)->default_value(8), "threshold to ignore distant points in neighborhood")
    ("upper,u", value<float>(&upper_)->default_value(6.0), "upper curvature threshold (edge)")
    ;

  positional_options_description p_opt;
  p_opt.add("in", 1);
  variables_map vm;
  store(command_line_parser(argc, argv).options(options).positional(p_opt).run(), vm);
  notify(vm);

  if (vm.count("help") || argc == 1)
  {
    cout << "\"pns\" stands for \"pixel neighborhood size\" and refers to N x N mask for nearest neighbor search\n"
	 << "\t where N = 2 * pns + 1\n" << endl;
    cout << options << endl;
    exit(0);
  }
}

int main(int argc, char** argv)
{
  readOptions(argc, argv);

  PointCloud<PointXYZRGB>::Ptr p(new PointCloud<PointXYZRGB>);
  PointCloud<PointXYZRGB>::Ptr pt(new PointCloud<PointXYZRGB>);
  PointCloud<PointXYZRGB>::Ptr pbp(new PointCloud<PointXYZRGB>);
  PointCloud<Normal>::Ptr n(new PointCloud<Normal>);
  PointCloud<Normal>::Ptr n_org(new PointCloud<Normal>);
  PointCloud<Normal>::Ptr n_bp(new PointCloud<Normal>);
  PointCloud<PointLabel>::Ptr l(new PointCloud<PointLabel>);

  PCDReader r;
  if (r.read(file_in_, *p) == -1) return(0);
  *pt = *p;
  PrecisionStopWatch t;
  t.precisionStart();
  cob_3d_mapping_features::OrganizedNormalEstimation<PointXYZRGB, Normal, PointLabel>one;
  one.setInputCloud(p);
  one.setOutputLabels(l);
  one.setPixelSearchRadius(8,2,2); //radius,pixel,circle
  one.setSkipDistantPointThreshold(d_th_);
  one.compute(*n);
  cout << t.precisionStop() << "s\t for Organized Normal Estimation" << endl;
  *n_org = *n;
  
  typedef cob_3d_mapping_features::PredefinedSegmentationTypes SegTypes;
  t.precisionStart();
  SegTypes::Graph::Ptr g(new SegTypes::Graph);
  cob_3d_mapping_features::DepthSegmentation<SegTypes::Graph, SegTypes::Point, SegTypes::Normal, SegTypes::Label> seg;
  seg.setPointCloudIn(p);
  seg.setNormalCloudIn(n);
  seg.setLabelCloudOut(l);
  seg.setClusterGraphOut(g);
  std::cout << "initial segmentation..." << std::endl;
  seg.performInitialSegmentation();
  std::cout << "refine segmentation.." << std::endl;
  seg.refineSegmentation();
  g->clusters()->mapClusterColor(p);
  g->edges()->mapBoundaryPoints(pbp,n_bp);
  cob_3d_mapping_features::ClusterClassifier<SegTypes::CH, SegTypes::Point, SegTypes::Normal, SegTypes::Label> cc;
  cc.setClusterHandler(g->clusters());
  cc.setPointCloudIn(p);
  cc.setNormalCloudInOut(n);
  cc.setLabelCloudIn(l);
  cc.classify();
  g->clusters()->mapTypeColor(pt);
  cout << t.precisionStop() << "s\t for depth segmentation" << endl;


  visualization::PCLVisualizer v;
  visualization::PointCloudColorHandlerRGBField<PointXYZRGB> chdl_p(p);
  visualization::PointCloudColorHandlerRGBField<PointXYZRGB> chdl_pt(pt);
  /* --- Viewports: ---
   *  1y
   *    | 1 | 2 |
   * .5 ----+----
   *    | 3 | 4 |
   *  0    .5    1x
   * 1:
   */
  // xmin, ymin, xmax, ymax
  int v1(0);
  v.createViewPort(0.0, 0.5, 0.5, 1.0, v1);
  v.setBackgroundColor(1, 1, 1, v1);
  v.addPointCloud<PointXYZRGB>(p, chdl_p, "segmented", v1);
  v.addPointCloudNormals<PointXYZRGB, Normal>(p, n_org, 4, 0.04, "normals_org", v1);
  v.setPointCloudRenderingProperties(visualization::PCL_VISUALIZER_COLOR, 0.7, 0.7, 0.7, "normals_org", v1);

  int v2(0);
  v.createViewPort(0.5, 0.5, 1.0, 1.0, v2);
  v.setBackgroundColor(1, 1, 1, v2);
  v.addPointCloud<PointXYZRGB>(pt, chdl_pt, "classified", v2);
  v.addPointCloudNormals<PointXYZRGB, Normal>(pt, n, 4, 0.04, "normals_new", v2);
  v.setPointCloudRenderingProperties(visualization::PCL_VISUALIZER_COLOR, 0.7, 0.7, 0.7, "normals_new", v2);
  //v.addPointCloudNormals<PointXYZRGB, Normal>(p,n,4,0.04,"normals2", v2);
  //v.addPointCloudNormals<PointXYZ, Normal>(centroids1st,normals1st,1,0.1,"normals2", v2);

  int v3(0);
  v.createViewPort(0.0, 0.0, 0.5, 0.5, v3);
  v.setBackgroundColor(1, 1, 1, v3);
  v.addPointCloud<PointXYZRGB>(p, chdl_p, "boundary_points", v3);
  v.addPointCloudNormals<PointXYZRGB, Normal>(pbp, n_bp, 2, 0.04, "boundary_normals", v3);
  //v.addPointCloud<PointXYZRGB>(cp2nd, col_hdl_2nd, "segmented2nd", v3);

  int v4(0);
  v.createViewPort(0.5, 0.0, 1.0, 0.5, v4);
  v.setBackgroundColor(1, 1, 1, v4);
  //v.addPointCloud<PointXYZRGB>(cp, col_hdl, "normals4", v4);
  //v.addPointCloudNormals<PointXYZRGB, Normal>(bp,bp_n,1,0.04,"normals3", v4);

  while(!v.wasStopped())
  {
    v.spinOnce(100);
    usleep(100000);
  }

  return 0;
}
