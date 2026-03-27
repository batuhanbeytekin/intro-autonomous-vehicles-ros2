#include <chrono>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "ball_chaser/srv/drive_to_target.hpp"

class ProcessImage : public rclcpp::Node
{
public:
  ProcessImage()
  : Node("process_image"),
    last_linear_x_(999.0),
    last_angular_z_(999.0)
  {
    image_subscriber_ = this->create_subscription<sensor_msgs::msg::Image>(
      "/camera/camera/image_raw",
      10,
      std::bind(&ProcessImage::process_image_callback, this, std::placeholders::_1)
    );

    command_client_ =
      this->create_client<ball_chaser::srv::DriveToTarget>("/ball_chaser/command_robot");

    RCLCPP_INFO(this->get_logger(), "process_image node started.");
  }

private:
  void process_image_callback(const sensor_msgs::msg::Image::SharedPtr msg)
  {
    if (!command_client_->wait_for_service(std::chrono::milliseconds(200))) {
      RCLCPP_WARN_THROTTLE(
        this->get_logger(),
        *this->get_clock(),
        2000,
        "Waiting for /ball_chaser/command_robot service..."
      );
      return;
    }

    if (msg->data.empty() || msg->width == 0 || msg->height == 0) {
      send_command_if_changed(0.0, 0.0);
      return;
    }

    const uint32_t width = msg->width;
    const uint32_t height = msg->height;
    const uint32_t step = msg->step;

    std::uint64_t white_pixel_count = 0;
    std::uint64_t x_sum = 0;

    // Sadece alt-orta bölgeyi tara.
    // Böylece üstteki açık renk duvarlar algıyı bozmaz.
    const uint32_t y_start = static_cast<uint32_t>(height * 0.35);
    const uint32_t y_end   = static_cast<uint32_t>(height * 0.95);
    const uint32_t x_start = static_cast<uint32_t>(width * 0.05);
    const uint32_t x_end   = static_cast<uint32_t>(width * 0.95);

    // Gazebo'da beyaz top kamerada gri gelebilir.
    // Bu yüzden saf beyaz değil, parlak piksel arıyoruz.
    const int brightness_threshold = 120;

    for (uint32_t y = y_start; y < y_end; ++y) {
    for (uint32_t x = x_start; x < x_end; ++x) {
        const std::size_t idx = y * step + x * 3;

        if (idx + 2 >= msg->data.size()) {
        continue;
        }

        const uint8_t r = msg->data[idx];
        const uint8_t g = msg->data[idx + 1];
        const uint8_t b = msg->data[idx + 2];

        const int brightness =
        (static_cast<int>(r) + static_cast<int>(g) + static_cast<int>(b)) / 3;

        if (brightness >= brightness_threshold) {
        white_pixel_count++;
        x_sum += x;
        }
    }
    }

    double linear_x = 0.0;
    double angular_z = 0.0;

    if (white_pixel_count < 50) {
    linear_x = 0.0;
    angular_z = 0.0;
    RCLCPP_INFO_THROTTLE(
        this->get_logger(),
        *this->get_clock(),
        2000,
        "No bright ball detected. Stopping."
    );
    } else {
      const double x_mean =
        static_cast<double>(x_sum) / static_cast<double>(white_pixel_count);

      const double left_boundary = static_cast<double>(width) / 3.0;
      const double right_boundary = 2.0 * static_cast<double>(width) / 3.0;

      if (x_mean < left_boundary) {
        linear_x = 0.0;
        angular_z = 0.4;
        RCLCPP_INFO_THROTTLE(
          this->get_logger(),
          *this->get_clock(),
          1000,
          "White ball on LEFT. Turning left."
        );
      } else if (x_mean < right_boundary) {
        linear_x = 0.25;
        angular_z = 0.0;
        RCLCPP_INFO_THROTTLE(
          this->get_logger(),
          *this->get_clock(),
          1000,
          "White ball in CENTER. Moving forward."
        );
      } else {
        linear_x = 0.0;
        angular_z = -0.4;
        RCLCPP_INFO_THROTTLE(
          this->get_logger(),
          *this->get_clock(),
          1000,
          "White ball on RIGHT. Turning right."
        );
      }
    }

    send_command_if_changed(linear_x, angular_z);
  }

  void send_command_if_changed(double linear_x, double angular_z)
  {
    if (std::fabs(linear_x - last_linear_x_) < 1e-6 &&
        std::fabs(angular_z - last_angular_z_) < 1e-6) {
      return;
    }

    auto request = std::make_shared<ball_chaser::srv::DriveToTarget::Request>();
    request->linear_x = linear_x;
    request->angular_z = angular_z;

    last_linear_x_ = linear_x;
    last_angular_z_ = angular_z;

    command_client_->async_send_request(request);
  }

  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_subscriber_;
  rclcpp::Client<ball_chaser::srv::DriveToTarget>::SharedPtr command_client_;

  double last_linear_x_;
  double last_angular_z_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ProcessImage>());
  rclcpp::shutdown();
  return 0;
}