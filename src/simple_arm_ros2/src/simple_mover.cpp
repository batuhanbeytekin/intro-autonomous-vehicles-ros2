#include <chrono>
#include <cmath>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64.hpp"

using namespace std::chrono_literals;

class SimpleMover : public rclcpp::Node
{
public:
  SimpleMover()
  : Node("simple_mover"), time_(0.0)
  {
    joint1_pub_ = this->create_publisher<std_msgs::msg::Float64>(
      "/simple_arm/joint_1_position_controller/command", 10);

    joint2_pub_ = this->create_publisher<std_msgs::msg::Float64>(
      "/simple_arm/joint_2_position_controller/command", 10);

    timer_ = this->create_wall_timer(
      50ms, std::bind(&SimpleMover::publish_joint_commands, this));

    RCLCPP_INFO(this->get_logger(), "simple_mover node started.");
  }

private:
  void publish_joint_commands()
  {
    std_msgs::msg::Float64 joint1_msg;
    std_msgs::msg::Float64 joint2_msg;

    const double amplitude = M_PI / 2.0;   // ±pi/2
    const double omega1 = 0.6;
    const double omega2 = 0.4;

    joint1_msg.data = amplitude * std::sin(omega1 * time_);
    joint2_msg.data = amplitude * std::sin(omega2 * time_);

    joint1_pub_->publish(joint1_msg);
    joint2_pub_->publish(joint2_msg);

    time_ += 0.05;
  }

  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr joint1_pub_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr joint2_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
  double time_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SimpleMover>());
  rclcpp::shutdown();
  return 0;
}