#pragma once

#include <chrono>
#include <string>

namespace websocket_bridge {

/**
 * Opens a socket to the service server and retrieves the service type from the connection header.
 * This is necessary as the service type is not stored on the ROS master.
 */
std::string retrieveServiceType(const std::string& serviceName,
                                std::chrono::milliseconds timeout_ms);

}  // namespace websocket_bridge
