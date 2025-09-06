#!/bin/bash

# PilotZero 运行脚本
# 启动localization和control模块

set -e

echo "🚀 PilotZero Run Script"
echo "======================="

# 检查环境
if [ -z "$ROS_DISTRO" ]; then
    echo "❌ Error: ROS2 environment not set"
    echo "Run: source /opt/ros/humble/setup.bash"
    exit 1
fi

# 检查是否已构建
if [ ! -d "install" ]; then
    echo "❌ Error: Project not built yet"
    echo "Run: ./scripts/build.sh first"
    exit 1
fi

# Source环境
echo "📦 Sourcing environment..."
source install/setup.bash

# 检查carla_msgs
if [ -d "/home/ethan/workspace/ros-carla-msgs/install" ]; then
    echo "📦 Sourcing carla_msgs..."
    source /home/ethan/workspace/ros-carla-msgs/install/setup.bash
fi

echo "✅ Environment ready"
echo ""

# 显示菜单
show_menu() {
    echo "🚀 PilotZero 模块选择"
    echo "====================="
    echo "可用的模块:"
    echo "0) localization - 定位模块 (30Hz)"
    echo "1) control - 控制模块 (10Hz)" 
    echo ""
    echo "其他选项:"
    echo "2) 退出"
    echo ""
}

# 运行localization
run_localization() {
    echo "🎯 Starting localization module..."
    echo "   Launch file: pilot_launch/launch/localization.launch.py"
    echo ""
    ros2 launch pilot_launch localization.launch.py
}

# 运行control
run_control() {
    echo "🎮 Starting control module..."
    echo "   Launch file: pilot_launch/launch/control.launch.py"
    echo ""
    ros2 launch pilot_launch control.launch.py
}

# 同时运行两个模块
run_both() {
    echo "🚀 Starting both modules..."
    echo "   This will open two terminals"
    echo ""
    
    # 在新终端中运行localization
    gnome-terminal --title="PilotZero Localization" -- bash -c "
        cd $(pwd)
        source install/setup.bash
        if [ -d '/home/ethan/workspace/ros-carla-msgs/install' ]; then
            source /home/ethan/workspace/ros-carla-msgs/install/setup.bash
        fi
        echo '🎯 Starting localization module...'
        ros2 launch pilot_launch localization.launch.py
        read -p 'Press Enter to close...'
    " &
    
    sleep 2
    
    # 在新终端中运行control
    gnome-terminal --title="PilotZero Control" -- bash -c "
        cd $(pwd)
        source install/setup.bash
        if [ -d '/home/ethan/workspace/ros-carla-msgs/install' ]; then
            source /home/ethan/workspace/ros-carla-msgs/install/setup.bash
        fi
        echo '🎮 Starting control module...'
        ros2 launch pilot_launch control.launch.py
        read -p 'Press Enter to close...'
    " &
    
    echo "✅ Both modules started in separate terminals"
    echo "   Close the terminals to stop the modules"
}

# 解析用户输入
parse_choice() {
    local choice="$1"
    
    case $choice in
        0)
            run_localization
            ;;
        1)
            run_control
            ;;
        2)
            echo "👋 Goodbye!"
            exit 0
            ;;
        *)
            echo "❌ Invalid choice: $choice. Please select 0-2."
            return 1
            ;;
    esac
}

# 主循环
main() {
    while true; do
        show_menu
        read -p "请选择模块 (输入数字 0-2): " choice
        
        if parse_choice "$choice"; then
            break
        fi
        echo ""
    done
}

# 检查命令行参数
if [ $# -eq 1 ]; then
    case $1 in
        "0"|"localization"|"loc")
            run_localization
            ;;
        "1"|"control"|"ctrl")
            run_control
            ;;
        "help"|"-h"|"--help")
            echo "用法: $0 [选项]"
            echo ""
            echo "选项:"
            echo "  0, localization, loc  - 运行定位模块"
            echo "  1, control, ctrl      - 运行控制模块"
            echo "  help, -h, --help      - 显示帮助信息"
            echo ""
            echo "不带参数运行将显示交互式菜单"
            ;;
        *)
            echo "❌ Unknown option: $1"
            echo "Run '$0 help' for usage information"
            exit 1
            ;;
    esac
else
    main
fi
