cmake_minimum_required(VERSION 3.8)
project(simple_arm_ros2)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "srv/GoToPosition.srv"
)

add_executable(simple_mover src/simple_mover.cpp)
ament_target_dependencies(simple_mover rclcpp std_msgs)

add_executable(arm_mover src/arm_mover.cpp)
ament_target_dependencies(arm_mover rclcpp std_msgs)

add_executable(look_away src/look_away.cpp)
ament_target_dependencies(look_away rclcpp sensor_msgs)

add_executable(arm_command_bridge src/arm_command_bridge.cpp)
ament_target_dependencies(arm_command_bridge rclcpp std_msgs)

add_executable(joint_state_relay src/joint_state_relay.cpp)
ament_target_dependencies(joint_state_relay rclcpp sensor_msgs)

rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(arm_mover "${cpp_typesupport_target}")
target_link_libraries(look_away "${cpp_typesupport_target}")

install(
  TARGETS
    simple_mover
    arm_mover
    look_away
    arm_command_bridge
    joint_state_relay
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY launch urdf config worlds
  DESTINATION share/${PROJECT_NAME}
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()