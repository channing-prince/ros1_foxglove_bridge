#include <nodelet/loader.h>
#include <ros/ros.h>

int main(int argc, char** argv) {
  ros::init(argc, argv, "websocket_bridge");
  nodelet::Loader nodelet;
  nodelet::M_string remap(ros::names::getRemappings());
  nodelet::V_string nargv;
  std::string nodelet_name = ros::this_node::getName();
  if (nodelet.load(nodelet_name, "websocket_bridge/websocket_bridge_nodelet", remap, nargv)) {
    ros::spin();
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
