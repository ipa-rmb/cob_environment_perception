cmake_minimum_required(VERSION 2.8.3)
project(cob_3d_segmentation)

set(catkin_RUN_PACKAGES 
  actionlib
  actionlib_msgs
  cob_3d_features
  cob_3d_mapping_common
  cob_3d_mapping_filters
  cob_3d_mapping_msgs
  dynamic_reconfigure
  geometry_msgs
  nodelet
  pcl_ros
  libpolypartition
  roscpp
  rosbag
)

set(catkin_BUILD_PACKAGES 
	${catkin_RUN_PACKAGES}
	cmake_modules
	message_generation
)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  ${catkin_BUILD_PACKAGES}
)

#find_package(TinyXML REQUIRED)
find_package(VTK REQUIRED COMPONENTS vtkCommon vtkFiltering vtkRendering)
#find_package(Boost REQUIRED COMPONENTS program_options)
find_package(OpenCV REQUIRED)
#find_package(PCL REQUIRED)
include(cmake/OptimizeForArchitecture.cmake)
OptimizeForArchitecture()

generate_dynamic_reconfigure_options(cfg/plane_extraction_nodelet.cfg
  cfg/segmentation_nodelet.cfg)

#######################################
## Declare ROS messages and services ##
#######################################


## Generate actions in the 'action' folder
add_action_files(
  DIRECTORY
    action
  FILES
    ObjectWatch.action
)

## Generate added messages and services with any dependencies listed here
generate_messages(
  DEPENDENCIES
    actionlib_msgs
    geometry_msgs
)

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if you package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
  INCLUDE_DIRS
    common/include
    ros/include
  LIBRARIES
    cob_3d_segmentation
    cob_3d_segmentation_nodelets
  CATKIN_DEPENDS
    ${catkin_RUN_PACKAGES}
    message_runtime
  DEPENDS
#    TinyXML
    VTK
#    Boost
    OpenCV
#    PCL
)



###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
# include_directories(include ${catkin_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
include_directories(
  common/include
  ros/include
  ${catkin_INCLUDE_DIRS}
#  ${TinyXML_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
#  ${Boost_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${PCL_INCLUDE_DIRS}
)

## Declare a cpp library
add_library(cob_3d_segmentation
	common/src/polygon_extraction.cpp
)
add_library(cob_3d_segmentation_nodelets
	ros/src/simple_segmentation_nodelet.cpp
)

## Declare a cpp executable
add_executable(qppf_node ros/src/qppf_node.cpp)
add_dependencies(qppf_node ${${PROJECT_NAME}_EXPORTED_TARGETS})
add_executable(qppf_node1 ros/src/qppf_node.cpp)
add_dependencies(qppf_node1 ${${PROJECT_NAME}_EXPORTED_TARGETS})
add_executable(qppf_node_ensenso ros/src/qppf_node.cpp)
add_dependencies(qppf_node_ensenso ${${PROJECT_NAME}_EXPORTED_TARGETS})

set_target_properties(qppf_node1 PROPERTIES COMPILE_DEFINITIONS "ONLY_PLANES_DEPTH")
set_target_properties(qppf_node_ensenso PROPERTIES COMPILE_DEFINITIONS "CAMERA_ENSENSO")


target_link_libraries(qppf_node
  ${catkin_LIBRARIES}
)

target_link_libraries(qppf_node1
  ${catkin_LIBRARIES}
)

target_link_libraries(qppf_node_ensenso
  ${catkin_LIBRARIES}
)


#target_link_libraries(test_segmentation
#  ${catkin_LIBRARIES}
#  gtest
#)

#target_link_libraries(evaluate_plane_segmentation
#  cob_3d_mapping_tools
#  ${catkin_LIBRARIES}
#  ${Boost_LIBRARIES}
#){SSE_FLAGS})

set(CMAKE_BUILD_TYPE Release)

