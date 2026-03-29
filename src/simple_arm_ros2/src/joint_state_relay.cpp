#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"

class JointStateRelay : public rclcpp::Node
{
public:
  JointStateRelay() : Node("joint_state_relay")
  {
    sub_ = this->create_subscription<sensor_msgs::msg::JointState>(
      "/joint_states",
      10,
      std::bind(&JointStateRelay::callback, this, std::placeholders::_1)
    );

    pub_ = this->create_publisher<sensor_msgs::msg::JointState>(
      "/simple_arm/joint_states",
      10
    );
  }

private:
  void callback(const sensor_msgs::msg::JointState::SharedPtr msg)
  {
    pub_->publish(*msg);
  }

  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr sub_;
  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr pub_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<JointStateRelay>());
  rclcpp::shutdown();
  return 0;
}