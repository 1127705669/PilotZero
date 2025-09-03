#!/usr/bin/env python3
"""
PilotZero 系统测试Launch文件
用于验证系统配置和模块框架
"""

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess, TimerAction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    """生成launch描述"""
    
    # 获取包路径
    common_dir = get_package_share_directory('common')
    
    # 参数
    use_sim_time = LaunchConfiguration('use_sim_time', default='false')
    
    # 显示系统信息
    show_info = ExecuteProcess(
        cmd=[
            'bash', '-c', 
            'echo "🚀 PilotZero 系统测试启动..." && '
            'echo "📦 当前ROS2包:" && '
            'ros2 pkg list | grep -E "(common|planning|localization|perception|control|map|visualization)" && '
            'echo "" && '
            'echo "📋 系统配置文件:" && '
            f'ls -la {common_dir}/config/ && '
            'echo "" && '
            'echo "✅ 系统就绪！可以开始开发算法模块。"'
        ],
        output='screen'
    )
    
    # 话题列表
    show_topics = TimerAction(
        period=3.0,
        actions=[
            ExecuteProcess(
                cmd=[
                    'bash', '-c',
                    'echo "📡 当前系统话题:" && '
                    'ros2 topic list && '
                    'echo "" && '
                    'echo "💡 提示：可以使用以下命令测试系统:" && '
                    'echo "  ros2 topic pub /test_topic std_msgs/msg/String \\"data: hello\\"" && '
                    'echo "  ros2 topic echo /test_topic"'
                ],
                output='screen'
            )
        ]
    )
    
    return LaunchDescription([
        # 参数声明
        DeclareLaunchArgument(
            'use_sim_time',
            default_value='false',
            description='是否使用仿真时间'
        ),
        
        # 执行
        show_info,
        show_topics,
    ])
