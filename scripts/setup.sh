#!/bin/bash

# PilotZero dependency setup script
# Installs ROS2 Humble and all required dependencies

set -e

echo "🔧 PilotZero Development Tools"
echo "=============================="

# Check Ubuntu version
check_ubuntu() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        if [[ "$VERSION_ID" != "22.04" ]]; then
            echo "Warning: Optimized for Ubuntu 22.04, current: $VERSION_ID"
        fi
    fi
}

# Install ROS2 Humble
install_ros2() {
    echo "Installing ROS2 Humble..."
    
    if [ -d "/opt/ros/humble" ]; then
        echo "ROS2 Humble already installed"
        return 0
    fi
    
    # Setup locale
    sudo apt update && sudo apt install locales -y
    sudo locale-gen en_US en_US.UTF-8
    sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
    
    # Setup sources
    sudo apt install software-properties-common curl -y
    sudo add-apt-repository universe -y
    
    sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
    
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null
    
    # Install ROS2
    sudo apt update
    sudo apt install ros-humble-desktop python3-colcon-common-extensions -y
    
    echo "ROS2 Humble installed successfully"
}

# Install project dependencies
install_dependencies() {
    echo "Installing project dependencies..."
    
    sudo apt update
    
    # Core dependencies
    sudo apt install -y \
        build-essential \
        cmake \
        git \
        python3-rosdep \
        python3-vcstool \
        python3-colcon-common-extensions \
        libyaml-cpp-dev \
        libprotobuf-dev \
        protobuf-compiler \
        ros-humble-rclcpp \
        ros-humble-rclcpp-components \
        ros-humble-std-msgs
    
    echo "All dependencies installed"
}

# Setup rosdep
setup_rosdep() {
    echo "Setting up rosdep..."
    
    if [ ! -f "/etc/ros/rosdep/sources.list.d/20-default.list" ]; then
        sudo rosdep init
    fi
    
    rosdep update
    echo "rosdep configured"
}

# Setup shell environment
setup_environment() {
    echo "Configuring shell environment..."
    
    BASHRC_LINE="source /opt/ros/humble/setup.bash"
    
    if ! grep -q "$BASHRC_LINE" ~/.bashrc; then
        echo "" >> ~/.bashrc
        echo "# ROS2 environment" >> ~/.bashrc
        echo "$BASHRC_LINE" >> ~/.bashrc
        echo "Added ROS2 to ~/.bashrc"
    else
        echo "ROS2 already in ~/.bashrc"
    fi
}

# ========== TypeAdapter 开发工具 ==========

# 生成TypeAdapter注册表
generate_typeadapter_registry() {
    echo "📝 Generating TypeAdapter registry from pipeline.yaml..."
    python3 scripts/generate_adapter_registry.py
    if [ $? -eq 0 ]; then
        echo "✅ TypeAdapter registry updated"
    else
        echo "❌ Failed to generate TypeAdapter registry"
        exit 1
    fi
    echo ""
}

# 添加新的TypeAdapter类型
add_typeadapter() {
    local key=$1
    local proto_type=$2
    local ros_type=$3
    local topic=$4
    
    if [ -z "$key" ] || [ -z "$proto_type" ] || [ -z "$ros_type" ]; then
        echo "❌ Usage: $0 --add-adapter <key> <proto_type> <ros_type> [topic]"
        echo "   Example: $0 --add-adapter gps pilot_msgs::localization::GpsData sensor_msgs::msg::NavSatFix /sensors/gps"
        exit 1
    fi
    
    if [ -z "$topic" ]; then
        topic="/${key}"
    fi
    
    echo "➕ Adding new TypeAdapter: $key"
    echo "   Proto: $proto_type"
    echo "   ROS: $ros_type" 
    echo "   Topic: $topic"
    
    # TODO: 实现向pipeline.yaml添加配置的逻辑
    echo "⚠️  Please manually add to modules/common/config/pipeline.yaml:"
    echo ""
    echo "  ${key}_data:"
    echo "    proto_type: \"$proto_type\""
    echo "    ros_topic: \"$topic\""
    echo "    ros_type: \"$ros_type\""
    echo ""
}

# 显示TypeAdapter状态
show_typeadapter_status() {
    echo "📊 TypeAdapter System Status"
    echo "============================"
    
    if [ -f "modules/common/include/common/adapter_registry.def" ]; then
        echo "✅ Registry file exists"
        local count=$(grep -c "^ADAPTER(" modules/common/include/common/adapter_registry.def 2>/dev/null || echo "0")
        echo "📝 Registered adapters: $count"
        
        echo ""
        echo "🔍 Current registrations:"
        grep "^ADAPTER(" modules/common/include/common/adapter_registry.def 2>/dev/null || echo "   None found"
    else
        echo "❌ Registry file not found"
        echo "   Run: $0 --generate-adapters"
    fi
    echo ""
}

# 显示帮助信息
show_help() {
    echo ""
    echo "用法: $0 [命令] [选项]"
    echo ""
    echo "🏗️  环境安装："
    echo "  install-env          安装ROS2环境和依赖"
    echo "  install-deps         仅安装项目依赖"
    echo ""
    echo "🔧 TypeAdapter 工具："
    echo "  --generate-adapters  从pipeline.yaml生成TypeAdapter注册表"
    echo "  --adapter-status     显示TypeAdapter系统状态"
    echo "  --add-adapter KEY PROTO ROS [TOPIC]  添加新TypeAdapter"
    echo ""
    echo "🚀 项目管理："
    echo "  --quick-build        快速编译"
    echo "  --clean              清理构建产物"
    echo "  --topic-list         查看活跃topics"
    echo ""
    echo "示例："
    echo "  $0 install-env                    # 安装完整环境"
    echo "  $0 --generate-adapters            # 重新生成TypeAdapter"
    echo "  $0 --add-adapter gps pilot_msgs::localization::GpsData sensor_msgs::msg::NavSatFix"
    echo "  $0 --quick-build                  # 快速编译"
    echo ""
}

# 快速编译
quick_build() {
    echo "⚡ Quick build with TypeAdapter generation..."
    generate_typeadapter_registry
    ./scripts/build.sh --quick
}

# 清理构建
clean_project() {
    echo "🧹 Cleaning project..."
    ./scripts/build.sh --clean
}

# 查看topics
show_topics() {
    echo "📡 Active ROS2 topics:"
    source install/setup.bash 2>/dev/null || echo "⚠️  Project not built"
    ros2 topic list 2>/dev/null | grep -E "(carla|pilot|imu|gps)" || echo "   No pilot/carla topics found"
    echo ""
}

# 安装完整环境
install_full_environment() {
    echo "🏗️  Installing full development environment..."
    check_ubuntu
    install_ros2
    install_dependencies
    setup_rosdep
    setup_environment
    
    echo ""
    echo "✅ Environment setup complete!"
    echo "============================="
    echo ""
    echo "Next steps:"
    echo "1. Reload shell: source ~/.bashrc"
    echo "2. Build project: ./scripts/build.sh"
    echo ""
}

# Main execution
main() {
    if [ $# -eq 0 ]; then
        show_help
        exit 0
    fi
    
    case $1 in
        install-env)
            install_full_environment
            ;;
        install-deps)
            install_dependencies
            ;;
        --generate-adapters)
            generate_typeadapter_registry
            ;;
        --adapter-status)
            show_typeadapter_status
            ;;
        --add-adapter)
            add_typeadapter "$2" "$3" "$4" "$5"
            ;;
        --quick-build)
            quick_build
            ;;
        --clean)
            clean_project
            ;;
        --topic-list)
            show_topics
            ;;
        -h|--help)
            show_help
            ;;
        *)
            echo "❌ Unknown command: $1"
            show_help
            exit 1
            ;;
    esac
}

main "$@"