cmake_minimum_required(VERSION 3.8)
project(ball_chaser)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "srv/DriveToTarget.srv"
)

add_executable(drive_bot src/drive_bot.cpp)
ament_target_dependencies(drive_bot rclcpp geometry_msgs)

add_executable(process_image src/process_image.cpp)
ament_target_dependencies(process_image rclcpp sensor_msgs)

rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(drive_bot "${cpp_typesupport_target}")
target_link_libraries(process_image "${cpp_typesupport_target}")

install(
  TARGETS
    drive_bot
    process_image
  DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_export_dependencies(rosidl_default_runtime)
ament_package()