from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    drive_bot_node = Node(
        package="ball_chaser",
        executable="drive_bot",
        name="drive_bot",
        output="screen"
    )

    process_image_node = Node(
        package="ball_chaser",
        executable="process_image",
        name="process_image",
        output="screen"
    )

    return LaunchDescription([
        drive_bot_node,
        process_image_node,
    ])