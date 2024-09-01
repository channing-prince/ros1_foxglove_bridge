cmake_minimum_required(VERSION 3.10.2)

if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
endif()
if(POLICY CMP0024)
  cmake_policy(SET CMP0024 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0024 NEW)
endif()

project(websocket_bridge LANGUAGES CXX VERSION 0.8.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

macro(enable_strict_compiler_warnings target)
  if (MSVC)
    target_compile_options(${target} PRIVATE /WX /W4)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wold-style-cast -Wfloat-equal -Wmost -Wunused-exception-parameter)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wold-style-cast -Wfloat-equal)
  endif()
endmacro()

find_package(nlohmann_json QUIET)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(websocketpp REQUIRED)
find_package(ZLIB REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# Determine wheter to use standalone or boost asio
option(USE_ASIO_STANDALONE "Build with standalone ASIO" ON)
if(USE_ASIO_STANDALONE)
  message(STATUS "Using standalone ASIO")
  add_definitions(-DASIO_STANDALONE)
else()
  message(STATUS "Using Boost ASIO")
  find_package(Boost REQUIRED)
endif(USE_ASIO_STANDALONE)

# Detect big-endian architectures
include(TestBigEndian)
TEST_BIG_ENDIAN(ENDIAN)
if (ENDIAN)
  add_compile_definitions(ARCH_IS_BIG_ENDIAN=1)
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/websocket_bridge_base/src/version.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/websocket_bridge_base/src/version.cpp
)
# Build the websocket_bridge_base library
add_library(websocket_bridge_base SHARED
  websocket_bridge_base/src/base64.cpp
  websocket_bridge_base/src/websocket_bridge.cpp
  websocket_bridge_base/src/parameter.cpp
  websocket_bridge_base/src/serialization.cpp
  websocket_bridge_base/src/server_factory.cpp
  websocket_bridge_base/src/test/test_client.cpp
  # Generated:
  ${CMAKE_CURRENT_BINARY_DIR}/websocket_bridge_base/src/version.cpp
)
target_include_directories(websocket_bridge_base
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/websocket_bridge_base/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(websocket_bridge_base
  OpenSSL::Crypto
  OpenSSL::SSL
  ZLIB::ZLIB
  ${CMAKE_THREAD_LIBS_INIT}
)
if(nlohmann_json_FOUND)
  target_link_libraries(websocket_bridge_base nlohmann_json::nlohmann_json)
else()
  message(STATUS "nlohmann_json not found, will search at compile time")
endif()
enable_strict_compiler_warnings(websocket_bridge_base)

message(STATUS "ROS_VERSION: " $ENV{ROS_VERSION})
message(STATUS "ROS_DISTRO: " $ENV{ROS_DISTRO})
message(STATUS "ROS_ROOT: " $ENV{ROS_ROOT})
if("$ENV{ROS_VERSION}" STREQUAL "1")
  # ROS 1
  if(CATKIN_DEVEL_PREFIX OR catkin_FOUND OR CATKIN_BUILD_BINARY_PACKAGE)
    message(STATUS "Building with catkin")
    set(ROS_BUILD_TYPE "catkin")

    find_package(catkin REQUIRED COMPONENTS nodelet resource_retriever ros_babel_fish rosgraph_msgs roslib roscpp)
    find_package(Boost REQUIRED)

    catkin_package(
      INCLUDE_DIRS websocket_bridge_base/include
      LIBRARIES websocket_bridge_base websocket_bridge_nodelet
      CATKIN_DEPENDS nodelet resource_retriever ros_babel_fish rosgraph_msgs roslib roscpp
      DEPENDS Boost
    )

    add_library(websocket_bridge_nodelet
      src/websocket_bridge_nodelet.cpp
      src/param_utils.cpp
      src/service_utils.cpp
    )
    target_include_directories(websocket_bridge_nodelet
      SYSTEM PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/websocket_bridge_base/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${catkin_INCLUDE_DIRS}
    )
    target_link_libraries(websocket_bridge_nodelet websocket_bridge_base ${catkin_LIBRARIES})
    enable_strict_compiler_warnings(websocket_bridge_nodelet)

    add_executable(websocket_bridge src/websocket_bridge_node.cpp)
    target_include_directories(websocket_bridge SYSTEM PRIVATE ${catkin_INCLUDE_DIRS})
    target_link_libraries(websocket_bridge ${catkin_LIBRARIES})
    enable_strict_compiler_warnings(websocket_bridge)
  else()
    message(FATAL_ERROR "Could not find catkin")
  endif()
elseif("$ENV{ROS_VERSION}" STREQUAL "2")
  # ROS 2
  if(DEFINED ENV{AMENT_PREFIX_PATH})
    message(STATUS "Building with ament_cmake")
    set(ROS_BUILD_TYPE "ament_cmake")

    find_package(ament_cmake REQUIRED)
    find_package(rosgraph_msgs REQUIRED)
    find_package(rclcpp REQUIRED)
    find_package(rclcpp_components REQUIRED)
    find_package(resource_retriever REQUIRED)
    find_package(rosx_introspection REQUIRED)

    add_library(websocket_bridge_component SHARED
      ros2_websocket_bridge/src/message_definition_cache.cpp
      ros2_websocket_bridge/src/param_utils.cpp
      ros2_websocket_bridge/src/ros2_websocket_bridge.cpp
      ros2_websocket_bridge/src/parameter_interface.cpp
      ros2_websocket_bridge/src/generic_client.cpp
    )

    target_compile_definitions(websocket_bridge_component
      PRIVATE
        RCLCPP_VERSION_MAJOR=${rclcpp_VERSION_MAJOR}
        RCLCPP_VERSION_MINOR=${rclcpp_VERSION_MINOR}
        RCLCPP_VERSION_PATCH=${rclcpp_VERSION_PATCH}
    )

    target_include_directories(websocket_bridge_component
      PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/websocket_bridge_base/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ros2_websocket_bridge/include>
        $<INSTALL_INTERFACE:include>
    )
    ament_target_dependencies(websocket_bridge_component ament_index_cpp rclcpp rclcpp_components resource_retriever rosgraph_msgs rosx_introspection)
    target_link_libraries(websocket_bridge_component websocket_bridge_base)
    rclcpp_components_register_nodes(websocket_bridge_component "websocket_bridge::WebsocketBridge")
    enable_strict_compiler_warnings(websocket_bridge_component)
    add_executable(websocket_bridge
      ros2_websocket_bridge/src/ros2_websocket_bridge_node.cpp
    )
    target_include_directories(websocket_bridge SYSTEM PRIVATE ${rclcpp_INCLUDE_DIRS})
    ament_target_dependencies(websocket_bridge rclcpp rclcpp_components)
    enable_strict_compiler_warnings(websocket_bridge)
  else()
    message(FATAL_ERROR "Could not find ament_cmake")
  endif()
else()
  message(FATAL_ERROR "ROS_VERSION environment variable must be 1 or 2")
endif()

#### TESTS #####################################################################

if(ROS_BUILD_TYPE STREQUAL "catkin")
  if (CATKIN_ENABLE_TESTING)
    message(STATUS "Building tests with catkin")

    find_package(catkin REQUIRED COMPONENTS roscpp std_msgs std_srvs)
    if(NOT "$ENV{ROS_DISTRO}" STREQUAL "melodic")
      find_package(GTest REQUIRED)
    endif()
    find_package(rostest REQUIRED)
    find_package(Boost REQUIRED COMPONENTS system)

    catkin_add_gtest(version_test websocket_bridge_base/tests/version_test.cpp)
    target_link_libraries(version_test websocket_bridge_base ${Boost_LIBRARIES})
    enable_strict_compiler_warnings(version_test)

    catkin_add_gtest(serialization_test websocket_bridge_base/tests/serialization_test.cpp)
    target_link_libraries(serialization_test websocket_bridge_base ${Boost_LIBRARIES})
    enable_strict_compiler_warnings(websocket_bridge)

    catkin_add_gtest(base64_test websocket_bridge_base/tests/base64_test.cpp)
    target_link_libraries(base64_test websocket_bridge_base ${Boost_LIBRARIES})
    enable_strict_compiler_warnings(websocket_bridge)

    add_rostest_gtest(smoke_test tests/smoke.test tests/smoke_test.cpp)
    target_include_directories(smoke_test SYSTEM PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/websocket_bridge_base/include>
      ${catkin_INCLUDE_DIRS}
      $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(smoke_test websocket_bridge_base ${catkin_LIBRARIES})
    enable_strict_compiler_warnings(smoke_test)
  endif()
elseif(ROS_BUILD_TYPE STREQUAL "ament_cmake")
  if(BUILD_TESTING)
    message(STATUS "Building tests with ament_cmake")

    find_package(ament_cmake_gtest REQUIRED)
    find_package(ament_lint_auto REQUIRED)
    ament_lint_auto_find_test_dependencies()

    ament_add_gtest(version_test websocket_bridge_base/tests/version_test.cpp)
    target_link_libraries(version_test websocket_bridge_base)
    enable_strict_compiler_warnings(version_test)

    ament_add_gtest(serialization_test websocket_bridge_base/tests/serialization_test.cpp)
    target_link_libraries(serialization_test websocket_bridge_base)
    enable_strict_compiler_warnings(serialization_test)

    ament_add_gtest(base64_test websocket_bridge_base/tests/base64_test.cpp)
    target_link_libraries(base64_test websocket_bridge_base)
    enable_strict_compiler_warnings(base64_test)

    ament_add_gtest(smoke_test ros2_websocket_bridge/tests/smoke_test.cpp)
    ament_target_dependencies(smoke_test rclcpp rclcpp_components std_msgs std_srvs)
    target_link_libraries(smoke_test websocket_bridge_component)
    enable_strict_compiler_warnings(smoke_test)

    ament_add_gtest(utils_test ros2_websocket_bridge/tests/utils_test.cpp)
    target_link_libraries(utils_test websocket_bridge_component)
    enable_strict_compiler_warnings(utils_test)
  endif()
endif()

#### INSTALL ###################################################################

if(ROS_BUILD_TYPE STREQUAL "catkin")
    install(TARGETS websocket_bridge
      RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
    )
    install(TARGETS websocket_bridge_base websocket_bridge_nodelet
      ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
      LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
      RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION}
    )
    install(FILES nodelets.xml
      DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
    )
    install(DIRECTORY launch/
      DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch
    )
elseif(ROS_BUILD_TYPE STREQUAL "ament_cmake")
    install(FILES ros2_websocket_bridge/include/websocket_bridge/ros2_websocket_bridge.hpp
      DESTINATION include/${PROJECT_NAME}/
    )
    install(TARGETS websocket_bridge
      DESTINATION lib/${PROJECT_NAME}
    )
    install(TARGETS websocket_bridge_base websocket_bridge_component
      ARCHIVE DESTINATION lib
      LIBRARY DESTINATION lib
      RUNTIME DESTINATION bin
    )
    install(DIRECTORY ros2_websocket_bridge/launch
      DESTINATION share/${PROJECT_NAME}/
    )
    install(FILES ros2_websocket_bridge/include/websocket_bridge/utils.hpp
      DESTINATION include/${PROJECT_NAME}/
    )
    ament_export_libraries(websocket_bridge_base websocket_bridge_component)
    ament_package()
endif()
