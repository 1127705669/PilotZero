#!/usr/bin/env python3

from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode


def generate_launch_description():
    """
    Launch file for localization component with TypeAdapter architecture
    
    演示特性：
    - 算法层使用纯Proto接口：get_latest("imu_data", proto_data)
    - TypeAdapter自动处理ROS ↔ Proto转换
    - 接收ROS传感器消息，内部转换为Proto数据
    - 配置驱动的Key到Topic映射
    """
    
    # Localization component
    localization_component = ComposableNode(
        package='localization',
        plugin='localization::LocalizationComponent',
        name='localization_node',
        parameters=[{
            'module_name': 'localization',  # 通用框架：指定模块名
            'enable_debug': True,
        }],
        extra_arguments=[{'use_intra_process_comms': True}]
    )
    
    # Container to manage components
    container = ComposableNodeContainer(
        name='localization_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            localization_component,
        ],
        output='screen',
    )
    
    return LaunchDescription([
        container,
    ])
