#!/bin/bash

# PilotZero 简单构建脚本
# 编译整个项目

set -e

echo "🚀 PilotZero Simple Build"
echo "========================="

# 检查环境
if [ -z "$ROS_DISTRO" ]; then
    echo "❌ Error: ROS2 environment not set"
    echo "Run: source /opt/ros/humble/setup.bash"
    exit 1
fi

if [ ! -f "modules/common/package.xml" ]; then
    echo "❌ Error: Run from PilotZero project root directory"
    exit 1
fi

echo "✅ Environment: ROS2 $ROS_DISTRO"

# 检查并source carla_msgs环境
if [ -d "/home/ethan/workspace/ros-carla-msgs/install" ]; then
    echo "📦 Sourcing carla_msgs environment..."
    source /home/ethan/workspace/ros-carla-msgs/install/setup.bash
    echo "✅ carla_msgs environment sourced"
else
    echo "⚠️  Warning: carla_msgs not found at /home/ethan/workspace/ros-carla-msgs/install"
    echo "   Some modules may fail to build without carla_msgs"
fi

# 清理构建产物
echo "🧹 Cleaning build artifacts..."
if [ -d "build" ]; then
    rm -rf build
    echo "   - Removed build/"
fi
if [ -d "install" ]; then
    rm -rf install
    echo "   - Removed install/"
fi
if [ -d "log" ]; then
    rm -rf log
    echo "   - Removed log/"
fi

# 构建项目
echo "🔨 Building PilotZero..."
echo ""

# 使用colcon构建所有包
colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release

if [ $? -eq 0 ]; then
    echo ""
    echo "🎉 Build successful!"
    echo "==================="
    echo ""
    echo "💡 Next steps:"
    echo "   source install/setup.bash"
    echo "   ./scripts/run.sh"
    echo ""
else
    echo "❌ Build failed!"
    exit 1
fi