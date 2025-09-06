#!/usr/bin/env python3
"""
启动control组件
"""

from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode


def generate_launch_description():
    """生成launch描述"""
    
    # 创建组件容器并加载control组件
    container = ComposableNodeContainer(
        name='control_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            ComposableNode(
                package='control',
                plugin='control::ControlComponent',
                name='control_node',
                parameters=[
                    {'module_name': 'control'},
                    {'control_gain_d': 0.05}
                ]
            ),
        ],
        output='screen',
    )
    
    return LaunchDescription([
        container,
    ])
