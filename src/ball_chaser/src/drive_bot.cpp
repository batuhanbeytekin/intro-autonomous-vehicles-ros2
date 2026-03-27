#include <memory>
#include <string>
#include <sstream>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "ball_chaser/srv/drive_to_target.hpp"

class DriveBot : public rclcpp::Node
{
public:
  DriveBot() : Node("drive_bot")
  {
    cmd_vel_publisher_ =
      this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

    command_service_ =
      this->create_service<ball_chaser::srv::DriveToTarget>(
        "/ball_chaser/command_robot",
        std::bind(
          &DriveBot::handle_drive_request,
          this,
          std::placeholders::_1,
          std::placeholders::_2
        )
      );

    RCLCPP_INFO(this->get_logger(), "drive_bot service is ready.");
  }

private:
  void handle_drive_request(
    const std::shared_ptr<ball_chaser::srv::DriveToTarget::Request> request,
    std::shared_ptr<ball_chaser::srv::DriveToTarget::Response> response)
  {
    geometry_msgs::msg::Twist cmd_msg;
    cmd_msg.linear.x = request->linear_x;
    cmd_msg.angular.z = request->angular_z;

    cmd_vel_publisher_->publish(cmd_msg);

    std::ostringstream ss;
    ss << "Published cmd_vel -> linear_x: " << request->linear_x
       << ", angular_z: " << request->angular_z;
    response->msg_feedback = ss.str();

    RCLCPP_INFO(this->get_logger(), "%s", response->msg_feedback.c_str());
  }

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_publisher_;
  rclcpp::Service<ball_chaser::srv::DriveToTarget>::SharedPtr command_service_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DriveBot>());
  rclcpp::shutdown();
  return 0;
}