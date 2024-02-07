from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    ld = LaunchDescription()

    turtlesim_node = Node(
        package="turtlesim",
        executable="turtlesim_node"
    )

    turtle_controller_node = Node(
        package="turtle_sim_project",
        executable="turtle_controller_node",
        parameters=[
            {"kill_closest_turtle_first": True}
        ]
    )

    turtle_spawner_node = Node(
        package="turtle_sim_project",
        executable="turtle_spawner_node",
        parameters=[
            {"turtle_spawn_period_in_milliseconds": 1000}
        ]
    )

    ld.add_action(turtlesim_node)
    ld.add_action(turtle_controller_node)
    ld.add_action(turtle_spawner_node)

    return ld