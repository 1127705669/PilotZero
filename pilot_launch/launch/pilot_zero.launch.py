#!/usr/bin/env python3
"""
PilotZero Launch File
"""

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, SetEnvironmentVariable
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node, ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    common_dir = get_package_share_directory('common')
    
    # 参数
    use_sim_time = LaunchConfiguration('use_sim_time', default='true')
    config_file = LaunchConfiguration('config', 
        default=os.path.join(common_dir, 'config', 'pipeline.yaml'))
    
    # DDS设置
    set_rmw = SetEnvironmentVariable(
        name='RMW_IMPLEMENTATION',
        value='rmw_fastrtps_cpp'
    )
    
    # 组件容器
    container = ComposableNodeContainer(
        name='pilot_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container_mt',
        parameters=[
            {'use_sim_time': use_sim_time},
            {'thread_num': 4},
        ],
        output='screen',
    )
    
    # 规划器节点
    planner = Node(
        package='planning',
        executable='planner_component',
        name='planner',
        parameters=[
            {'use_sim_time': use_sim_time},
            {'frequency': 10.0},
            {'horizon': 5.0},
        ],
        output='screen',
    )
    
    # 可视化桥接（可选）
    viz_bridge = Node(
        package='visualization',
        executable='visualization_bridge',
        name='visualization_bridge',
        parameters=[
            {'use_sim_time': use_sim_time},
            {'enable_visualization': True},
            {'topics_to_visualize': [
                '/ego_state',
                '/planning/trajectory',
                '/perception/objects'
            ]},
        ],
        output='screen',
    )
    
    return LaunchDescription([
        # 参数声明
        DeclareLaunchArgument('use_sim_time', default_value='true'),
        DeclareLaunchArgument('config', default_value=config_file),
        
        # 环境设置
        set_rmw,
        
        # 节点
        container,
        planner,
        viz_bridge,
    ])