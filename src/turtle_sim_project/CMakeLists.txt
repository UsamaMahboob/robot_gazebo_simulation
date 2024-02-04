cmake_minimum_required(VERSION 3.8)
project(turtle_sim_project)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(turtlesim REQUIRED)

add_executable(turtle_controller_node src/turtle_controller_node.cpp)
ament_target_dependencies(turtle_controller_node rclcpp turtlesim geometry_msgs)

add_executable(turtle_spawner_node src/turtle_spawner.cpp)
ament_target_dependencies(turtle_spawner_node rclcpp turtlesim)

install(TARGETS
  turtle_controller_node
  turtle_spawner_node
  DESTINATION lib/${PROJECT_NAME})

ament_package()
