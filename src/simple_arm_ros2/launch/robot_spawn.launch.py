from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess, IncludeLaunchDescription, RegisterEventHandler
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessExit
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command, FindExecutable, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    pkg_share = FindPackageShare("simple_arm_ros2")
    use_simple_mover = LaunchConfiguration("use_simple_mover")

    controllers_file = PathJoinSubstitution(
        [pkg_share, "config", "controllers.yaml"]
    )

    xacro_file = PathJoinSubstitution(
        [pkg_share, "urdf", "simple_arm_urdf.xacro"]
    )

    robot_description = ParameterValue(
        Command([
            FindExecutable(name="xacro"),
            " ",
            xacro_file,
            " ",
            "controllers_file:=",
            controllers_file
        ]),
        value_type=str
    )

    world_file = PathJoinSubstitution(
        [pkg_share, "worlds", "simple_arm.world"]
    )

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution(
                [FindPackageShare("gazebo_ros"), "launch", "gazebo.launch.py"]
            )
        ),
        launch_arguments={
            "world": world_file,
            "verbose": "true",
        }.items(),
    )

    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        name="robot_state_publisher",
        output="screen",
        parameters=[{"robot_description": robot_description, "use_sim_time": True}],
    )

    spawn_entity = Node(
        package="gazebo_ros",
        executable="spawn_entity.py",
        arguments=[
            "-topic", "robot_description",
            "-entity", "simple_arm",
            "-x", "0.0",
            "-y", "0.0",
            "-z", "0.1"
        ],
        output="screen",
    )

    joint_state_broadcaster_spawner = ExecuteProcess(
        cmd=[
            "ros2", "run", "controller_manager", "spawner",
            "joint_state_broadcaster",
            "--controller-manager", "/controller_manager"
        ],
        output="screen"
    )

    arm_position_controller_spawner = ExecuteProcess(
        cmd=[
            "ros2", "run", "controller_manager", "spawner",
            "arm_position_controller",
            "--controller-manager", "/controller_manager"
        ],
        output="screen"
    )

    start_controllers = RegisterEventHandler(
        OnProcessExit(
            target_action=spawn_entity,
            on_exit=[
                joint_state_broadcaster_spawner,
                arm_position_controller_spawner,
            ],
        )
    )

    arm_command_bridge = Node(
        package="simple_arm_ros2",
        executable="arm_command_bridge",
        name="arm_command_bridge",
        output="screen"
    )

    joint_state_relay = Node(
        package="simple_arm_ros2",
        executable="joint_state_relay",
        name="joint_state_relay",
        output="screen"
    )

    arm_mover = Node(
        package="simple_arm_ros2",
        executable="arm_mover",
        name="arm_mover",
        output="screen",
        parameters=[
            {"min_joint_1_angle": -1.57},
            {"max_joint_1_angle": 1.57},
            {"min_joint_2_angle": -1.57},
            {"max_joint_2_angle": 1.57},
        ]
    )

    look_away = Node(
        package="simple_arm_ros2",
        executable="look_away",
        name="look_away",
        output="screen"
    )

    simple_mover = Node(
        package="simple_arm_ros2",
        executable="simple_mover",
        name="simple_mover",
        output="screen",
        condition=IfCondition(use_simple_mover)
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            "use_simple_mover",
            default_value="false",
            description="Run simple_mover continuously"
        ),
        gazebo,
        robot_state_publisher,
        spawn_entity,
        start_controllers,
        arm_command_bridge,
        joint_state_relay,
        arm_mover,
        look_away,
        simple_mover,
    ])