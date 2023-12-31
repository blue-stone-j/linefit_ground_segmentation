#ifndef GROUND_SEGMENTATION_H_
#define GROUND_SEGMENTATION_H_

#include <mutex>

#include <glog/logging.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/visualization/pcl_visualizer.h>

#include "ground_segmentation/segment.h"

struct GroundSegmentationParams
{
  GroundSegmentationParams() : visualize(false),
                               r_min_square(0.3 * 0.3),
                               r_max_square(20 * 20),
                               n_bins(30),
                               n_segments(180),
                               max_dist_to_line(0.15),
                               max_slope(1),
                               n_threads(4),
                               max_error_square(0.01),
                               long_threshold(2.0),
                               max_long_height(0.1),
                               max_start_height(0.2),
                               sensor_height(0.2),
                               line_search_angle(0.2) {}

  // Visualize estimated ground.
  bool visualize;
  // Minimum range of segmentation.
  double r_min_square;
  // Maximum range of segmentation.
  double r_max_square;
  // Number of radial bins.
  int n_bins;
  // Number of angular segments.
  int n_segments;
  // Maximum distance to a ground line to be classified as ground.
  double max_dist_to_line;
  // Max slope to be considered ground line.
  double max_slope;
  // Max error for line fit.
  double max_error_square;
  // Distance at which points are considered far from each other.
  double long_threshold;
  // Maximum slope for
  double max_long_height;
  // Maximum heigh of starting line to be labelled ground.
  double max_start_height;
  // Height of sensor above ground.
  double sensor_height;
  // How far to search for a line in angular direction [rad].
  double line_search_angle;
  // Number of threads.
  int n_threads;
};

typedef pcl::PointCloud<pcl::PointXYZ> PointCloud;

typedef std::pair<pcl::PointXYZ, pcl::PointXYZ> PointLine; // two ends of a line segment

/*从linefit_ground_segmentation_ros中跳转到这里*/
class GroundSegmentation
{
  const GroundSegmentationParams params_;

  // Access with segments_[segment][bin].
  /*定义了一个划分*/
  std::vector<Segment> segments_;

  // Bin index of every point.
  /*设定了一个针对于所有点而言的bin的索引(segment_index, bin_index)*/
  std::vector<std::pair<int, int>> bin_index_;

  // 2D coordinates (d, z) of every point in its respective segment.
  /*我们设定了一个二维的坐标(d,z)对于各自的分割*/
  std::vector<Bin::MinZPoint> segment_coordinates_;

  // Visualizer.
  /*可视化的设定*/
  std::shared_ptr<pcl::visualization::PCLVisualizer> viewer_; // 构建一个pcl_viewer

  /*划分集群*/
  void assignCluster(std::vector<int> *segmentation);

  /*划分集群线程*/
  void assignClusterThread(const unsigned int &start_index,
                           const unsigned int &end_index,
                           std::vector<int> *segmentation);

  /*插入点云; devide points into bins and segments*/
  void insertPoints(const PointCloud &cloud);

  /*插入线程; devide points into bins and segments*/
  void insertionThread(const PointCloud &cloud, const size_t start_index, const size_t end_index);

  /*获得最小的z点*/
  void getMinZPoints(PointCloud *out_cloud);

  /*得到计算出来的线*/
  void getLines(std::list<PointLine> *lines);

  /*直线拟合线程*/
  void lineFitThread(const unsigned int start_index, const unsigned int end_index,
                     std::list<PointLine> *lines, std::mutex *lines_mutex);

  /*将最小z点转化到3D中*/
  pcl::PointXYZ minZPointTo3d(const Bin::MinZPoint &min_z_point, const double &angle);

  /*获取最小z点的线程*/
  void getMinZPointCloud(PointCloud *cloud);

  /*可视化点云*/
  void visualizePointCloud(const PointCloud::ConstPtr &cloud, const std::string &id = "point_cloud");

  /*可视化线*/
  void visualizeLines(const std::list<PointLine> &lines);

  /*可视化设定*/
  void visualize(const std::list<PointLine> &lines, const PointCloud::ConstPtr &cloud,
                 const PointCloud::ConstPtr &ground_cloud, const PointCloud::ConstPtr &obstacle_cloud);

 public:
  GroundSegmentation(const GroundSegmentationParams &params = GroundSegmentationParams());

  void segment(const PointCloud &cloud, std::vector<int> *segmentation);
};

#endif // GROUND_SEGMENTATION_H_
