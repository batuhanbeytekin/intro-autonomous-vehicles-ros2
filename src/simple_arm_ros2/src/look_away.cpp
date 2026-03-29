#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "simple_arm_ros2/srv/go_to_position.hpp"

using namespace std::chrono_literals;

class LookAway : public rclcpp::Node
{
public:
  LookAway()
  : Node("look_away"),
    arm_is_moving_(false),
    has_previous_joint_state_(false),
    service_request_in_progress_(false)
  {
    joint_state_sub_ = this->create_subscription<sensor_msgs::msg::JointState>(
      "/simple_arm/joint_states",
      10,
      std::bind(&LookAway::joint_states_callback, this, std::placeholders::_1)
    );

    image_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
      "/rgb_camera/image_raw",
      10,
      std::bind(&LookAway::look_away_callback, this, std::placeholders::_1)
    );

    safe_move_client_ =
      this->create_client<simple_arm_ros2::srv::GoToPosition>("/arm_mover/safe_move");

    last_move_time_ = this->now();

    RCLCPP_INFO(this->get_logger(), "look_away node started.");
  }

private:
  void joint_states_callback(const sensor_msgs::msg::JointState::SharedPtr msg)
  {
    if (msg->position.size() < 2) {
      return;
    }

    if (!has_previous_joint_state_) {
      previous_joint_positions_ = msg->position;
      has_previous_joint_state_ = true;
      arm_is_moving_ = false;
      return;
    }

    const double threshold = 0.01;
    double max_diff = 0.0;

    const std::size_t n =
      std::min(previous_joint_positions_.size(), msg->position.size());

    for (std::size_t i = 0; i < n; ++i) {
      const double diff = std::fabs(msg->position[i] - previous_joint_positions_[i]);
      if (diff > max_diff) {
        max_diff = diff;
      }
    }

    arm_is_moving_ = (max_diff > threshold);
    previous_joint_positions_ = msg->position;
  }

  void look_away_callback(const sensor_msgs::msg::Image::SharedPtr msg)
  {
    if (!has_previous_joint_state_) {
      return;
    }

    if (arm_is_moving_) {
      return;
    }

    if (service_request_in_progress_) {
      return;
    }

    const auto now = this->now();
    if ((now - last_move_time_).seconds() < 2.0) {
      return;
    }

    if (image_is_uniform(msg)) {
      RCLCPP_INFO(this->get_logger(), "Uniform image detected while arm is stationary.");
      move_arm_center();
    }
  }

  bool image_is_uniform(const sensor_msgs::msg::Image::SharedPtr msg)
  {
    if (msg->data.empty() || msg->width == 0 || msg->height == 0) {
      return false;
    }

    if (msg->step < msg->width * 3) {
      return false;
    }

    const int tolerance = 18;

    const uint32_t sample_y = msg->height / 2;
    const uint32_t sample_x = msg->width / 2;
    const std::size_t ref_idx = sample_y * msg->step + sample_x * 3;

    if (ref_idx + 2 >= msg->data.size()) {
      return false;
    }

    const int ref_r = msg->data[ref_idx];
    const int ref_g = msg->data[ref_idx + 1];
    const int ref_b = msg->data[ref_idx + 2];

    int checked_pixels = 0;
    int matching_pixels = 0;

    for (uint32_t y = 0; y < msg->height; y += 10) {
      for (uint32_t x = 0; x < msg->width; x += 10) {
        const std::size_t idx = y * msg->step + x * 3;
        if (idx + 2 >= msg->data.size()) {
          continue;
        }

        const int r = msg->data[idx];
        const int g = msg->data[idx + 1];
        const int b = msg->data[idx + 2];

        const bool match =
          std::abs(r - ref_r) <= tolerance &&
          std::abs(g - ref_g) <= tolerance &&
          std::abs(b - ref_b) <= tolerance;

        checked_pixels++;
        if (match) {
          matching_pixels++;
        }
      }
    }

    if (checked_pixels == 0) {
      return false;
    }

    const double ratio =
      static_cast<double>(matching_pixels) / static_cast<double>(checked_pixels);

    return ratio > 0.95;
  }

  void move_arm_center()
  {
    if (!safe_move_client_->wait_for_service(500ms)) {
      RCLCPP_WARN(this->get_logger(), "safe_move service is not available.");
      return;
    }

    auto request = std::make_shared<simple_arm_ros2::srv::GoToPosition::Request>();
    request->joint_1 = 0.0;
    request->joint_2 = 0.0;

    service_request_in_progress_ = true;

    safe_move_client_->async_send_request(
      request,
      std::bind(&LookAway::safe_move_response_callback, this, std::placeholders::_1)
    );
  }

  void safe_move_response_callback(
    rclcpp::Client<simple_arm_ros2::srv::GoToPosition>::SharedFuture future)
  {
    service_request_in_progress_ = false;
    last_move_time_ = this->now();

    try {
      const auto response = future.get();
      RCLCPP_INFO(
        this->get_logger(),
        "safe_move response: %s",
        response->msg_feedback.c_str()
      );
    } catch (const std::exception & e) {
      RCLCPP_ERROR(this->get_logger(), "Service call failed: %s", e.what());
    }
  }

  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;
  rclcpp::Client<simple_arm_ros2::srv::GoToPosition>::SharedPtr safe_move_client_;

  std::vector<double> previous_joint_positions_;
  bool arm_is_moving_;
  bool has_previous_joint_state_;
  bool service_request_in_progress_;
  rclcpp::Time last_move_time_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<LookAway>());
  rclcpp::shutdown();
  return 0;
}