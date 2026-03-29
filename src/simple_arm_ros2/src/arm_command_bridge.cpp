#include <chrono>
#include <memory>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"

using namespace std::chrono_literals;

class ArmCommandBridge : public rclcpp::Node
{
public:
  ArmCommandBridge() : Node("arm_command_bridge"), joint_1_(0.0), joint_2_(0.0)
  {
    joint1_sub_ = this->create_subscription<std_msgs::msg::Float64>(
      "/simple_arm/joint_1_position_controller/command",
      10,
      std::bind(&ArmCommandBridge::joint1_callback, this, std::placeholders::_1)
    );

    joint2_sub_ = this->create_subscription<std_msgs::msg::Float64>(
      "/simple_arm/joint_2_position_controller/command",
      10,
      std::bind(&ArmCommandBridge::joint2_callback, this, std::placeholders::_1)
    );

    controller_pub_ = this->create_publisher<std_msgs::msg::Float64MultiArray>(
      "/arm_position_controller/commands",
      10
    );

    timer_ = this->create_wall_timer(
      50ms,
      std::bind(&ArmCommandBridge::publish_commands, this)
    );
  }

private:
  void joint1_callback(const std_msgs::msg::Float64::SharedPtr msg)
  {
    joint_1_ = msg->data;
  }

  void joint2_callback(const std_msgs::msg::Float64::SharedPtr msg)
  {
    joint_2_ = msg->data;
  }

  void publish_commands()
  {
    std_msgs::msg::Float64MultiArray msg;
    msg.data = {joint_1_, joint_2_};
    controller_pub_->publish(msg);
  }

  double joint_1_;
  double joint_2_;

  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr joint1_sub_;
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr joint2_sub_;
  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr controller_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ArmCommandBridge>());
  rclcpp::shutdown();
  return 0;
}