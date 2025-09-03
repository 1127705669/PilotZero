#!/usr/bin/env python3

from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode


def generate_launch_description():
    """
    完整系统Launch文件 - 演示混合通信架构
    
    数据流：
    CARLA → LocalizationComponent (ROS→Proto) → PlannerComponent (Proto→Proto) → 
    ControllerComponent (Proto→Proto) → VisualizerComponent (Proto→ROS)
    
    特性演示：
    1. ROS原始消息接收（CARLA传感器数据）
    2. 高性能Proto内部通信
    3. ROS可视化消息输出（RViz）
    4. 混合QoS配置
    5. 进程内零拷贝通信
    """
    
    # Localization component - 接收CARLA ROS数据，输出Proto
    localization_node = ComposableNode(
        package='localizationros',
        plugin='localizationros::LocalizationComponent',
        name='localization',
        parameters=[{
            'map_frame': 'map',
            'base_link_frame': 'base_link',
            'publish_tf': True,
            'enable_visualization': True,
            'fusion_frequency': 30.0,
        }],
        extra_arguments=[{'use_intra_process_comms': True}]
    )
    
    # Planning component - 接收Proto定位，输出Proto轨迹
    planning_node = ComposableNode(
        package='planning',
        plugin='planning::PlannerComponent', 
        name='planner',
        parameters=[{
            'planning_horizon': 50.0,
            'max_speed': 10.0,
            'max_acceleration': 2.0,
        }],
        extra_arguments=[{'use_intra_process_comms': True}]
    )
    
    # Control component - 接收Proto轨迹，输出Proto控制命令
    control_node = ComposableNode(
        package='control',
        plugin='control::ControllerComponent',
        name='controller', 
        parameters=[{
            'control_gain_p': 0.8,
            'control_gain_i': 0.1,
            'control_gain_d': 0.05,
            'max_steering_angle': 0.5,
        }],
        extra_arguments=[{'use_intra_process_comms': True}]
    )
    
    # 主要处理容器 - 高性能Proto通信
    main_container = ComposableNodeContainer(
        name='pilot_main_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            localization_node,
            planning_node, 
            control_node,
        ],
        output='screen',
        parameters=[{
            'use_sim_time': True,  # 适配CARLA仿真时间
        }]
    )
    
    return LaunchDescription([
        main_container,
    ])
