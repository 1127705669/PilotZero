#!/usr/bin/env python3
"""
Launch planning component
"""

from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode


def generate_launch_description():
    """Generate launch description"""
    
    # Create component container and load planning component
    container = ComposableNodeContainer(
        name='planning_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            ComposableNode(
                package='planning',
                plugin='planning::PlannerComponent',
                name='planning_node',
                parameters=[
                    {'planning_horizon': 50.0},
                    {'max_speed': 10.0},
                    {'trajectory_dt': 0.1}
                ]
            ),
        ],
        output='screen',
    )
    
    return LaunchDescription([
        container,
    ])
