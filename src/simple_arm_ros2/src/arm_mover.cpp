#include <algorithm>
#include <memory>
#include <sstream>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64.hpp"
#include "simple_arm_ros2/srv/go_to_position.hpp"

class ArmMover : public rclcpp::Node
{
public:
  ArmMover()
  : Node("arm_mover")
  {
    this->declare_parameter("min_joint_1_angle", -1.57);
    this->declare_parameter("max_joint_1_angle", 1.57);
    this->declare_parameter("min_joint_2_angle", -1.57);
    this->declare_parameter("max_joint_2_angle", 1.57);

    this->get_parameter("min_joint_1_angle", min_joint_1_angle_);
    this->get_parameter("max_joint_1_angle", max_joint_1_angle_);
    this->get_parameter("min_joint_2_angle", min_joint_2_angle_);
    this->get_parameter("max_joint_2_angle", max_joint_2_angle_);

    joint1_pub_ = this->create_publisher<std_msgs::msg::Float64>(
      "/simple_arm/joint_1_position_controller/command", 10);

    joint2_pub_ = this->create_publisher<std_msgs::msg::Float64>(
      "/simple_arm/joint_2_position_controller/command", 10);

    safe_move_service_ =
      this->create_service<simple_arm_ros2::srv::GoToPosition>(
        "/arm_mover/safe_move",
        std::bind(
          &ArmMover::handle_safe_move_request,
          this,
          std::placeholders::_1,
          std::placeholders::_2
        )
      );

    RCLCPP_INFO(
      this->get_logger(),
      "arm_mover ready. Joint1:[%.2f, %.2f], Joint2:[%.2f, %.2f]",
      min_joint_1_angle_, max_joint_1_angle_,
      min_joint_2_angle_, max_joint_2_angle_);
  }

private:
  double clamp_at_boundaries(double value, double lower, double upper)
  {
    return std::clamp(value, lower, upper);
  }

  void handle_safe_move_request(
    const std::shared_ptr<simple_arm_ros2::srv::GoToPosition::Request> request,
    std::shared_ptr<simple_arm_ros2::srv::GoToPosition::Response> response)
  {
    const double joint_1_clamped =
      clamp_at_boundaries(request->joint_1, min_joint_1_angle_, max_joint_1_angle_);
    const double joint_2_clamped =
      clamp_at_boundaries(request->joint_2, min_joint_2_angle_, max_joint_2_angle_);

    std_msgs::msg::Float64 joint1_msg;
    std_msgs::msg::Float64 joint2_msg;

    joint1_msg.data = joint_1_clamped;
    joint2_msg.data = joint_2_clamped;

    joint1_pub_->publish(joint1_msg);
    joint2_pub_->publish(joint2_msg);

    std::ostringstream ss;
    ss << "Requested: [" << request->joint_1 << ", " << request->joint_2
       << "] -> Clamped: [" << joint_1_clamped << ", " << joint_2_clamped << "]";
    response->msg_feedback = ss.str();

    RCLCPP_INFO(this->get_logger(), "%s", response->msg_feedback.c_str());
  }

  double min_joint_1_angle_;
  double max_joint_1_angle_;
  double min_joint_2_angle_;
  double max_joint_2_angle_;

  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr joint1_pub_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr joint2_pub_;
  rclcpp::Service<simple_arm_ros2::srv::GoToPosition>::SharedPtr safe_move_service_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ArmMover>());
  rclcpp::shutdown();
  return 0;
}