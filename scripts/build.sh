#!/bin/bash

# PilotZero build script
# Compiles all modules using colcon

set -e

echo "PilotZero Build"
echo "==============="

# Check environment
check_environment() {
    if [ -z "$ROS_DISTRO" ]; then
        echo "Error: ROS2 environment not set"
        echo "Run: source /opt/ros/humble/setup.bash"
        exit 1
    fi
    
    if [ ! -f "modules/common/package.xml" ]; then
        echo "Error: Run from PilotZero project root directory"
        exit 1
    fi
    
    echo "Environment: ROS2 $ROS_DISTRO"
}

# 确保TypeAdapter注册表是最新的
ensure_adapters() {
    if [ ! -f "modules/common/include/common/adapter_registry.def" ]; then
        echo "📝 TypeAdapter registry not found, generating..."
        ./scripts/setup.sh --generate-adapters
    fi
}

# Install workspace dependencies
install_workspace_deps() {
    echo "📦 Installing workspace dependencies..."
    rosdep install --from-paths modules pilot_launch --ignore-src -r -y
    echo ""
}

# Clean build artifacts
clean_build() {
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
    echo "✅ Clean completed"
    echo ""
}

# Build project with proper dependency order
build_project() {
    echo "🔨 Building PilotZero with TypeAdapter system..."
    echo ""
    
    # 步骤1：编译proto包
    echo "Step 1/4: Building pilot_msgs (Proto definitions)..."
    colcon build --packages-select pilot_msgs \
        --cmake-args -DCMAKE_BUILD_TYPE=Release
    
    if [ $? -ne 0 ]; then
        echo "❌ Failed to build pilot_msgs"
        exit 1
    fi
    
    # 步骤2：编译通用框架
    echo "Step 2/4: Building common (TypeAdapter framework)..."
    colcon build --packages-select common \
        --cmake-args -DCMAKE_BUILD_TYPE=Release \
        --allow-overriding common
    
    if [ $? -ne 0 ]; then
        echo "❌ Failed to build common framework"
        exit 1
    fi
    
    # 步骤3：编译所有算法模块
    echo "Step 3/4: Building algorithm modules..."
    colcon build --packages-select localization \
        --cmake-args -DCMAKE_BUILD_TYPE=Release \
        --allow-overriding localization
        
    if [ $? -ne 0 ]; then
        echo "❌ Failed to build algorithm modules"
        exit 1
    fi
    
    # 步骤4：编译启动包
    echo "Step 4/4: Building launch packages..."
    colcon build --packages-select pilot_launch \
        --cmake-args -DCMAKE_BUILD_TYPE=Release \
        --allow-overriding pilot_launch
    
    if [ $? -ne 0 ]; then
        echo "❌ Failed to build launch packages"
        exit 1
    fi
    
    echo ""
    echo "🎉 Build successful!"
    echo "==================="
    echo ""
    echo "💡 Quick Start:"
    echo "   source install/setup.bash"
    echo "   ros2 launch pilot_launch localization.launch.py"
    echo ""
    echo "📋 Available launches:"
    echo "   - localization.launch.py  # 定位模块（TypeAdapter演示）"
    echo "   - full_system.launch.py   # 完整系统"
    echo ""
}

# 显示帮助信息
show_help() {
    echo "🚀 PilotZero TypeAdapter Build System"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help           显示此帮助信息"
    echo "  -c, --clean          清理所有构建产物"
    echo "  -g, --generate-only  仅生成TypeAdapter注册表"
    echo "  -f, --full-rebuild   完整重构建（清理+生成+编译）"
    echo "  -q, --quick          快速增量编译"
    echo "  -d, --debug          使用Debug模式编译"
    echo "  --deps-only          仅安装依赖"
    echo ""
    echo "示例:"
    echo "  $0                   # 标准编译流程"
    echo "  $0 -f                # 完整重构建"
    echo "  $0 -g                # 仅重新生成TypeAdapter"
    echo "  $0 -c                # 清理构建产物"
    echo ""
}

# Quick incremental build
quick_build() {
    echo "⚡ Quick incremental build..."
    ensure_adapters
    colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release
    echo "✅ Quick build completed"
}

# Main execution with command line options
main() {
    local BUILD_TYPE="Release"
    local CLEAN_FIRST=false
    local GENERATE_ONLY=false
    local QUICK_BUILD=false
    local DEPS_ONLY=false
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--clean)
                clean_build
                exit 0
                ;;
            -g|--generate-only)
                GENERATE_ONLY=true
                shift
                ;;
            -f|--full-rebuild)
                CLEAN_FIRST=true
                shift
                ;;
            -q|--quick)
                QUICK_BUILD=true
                shift
                ;;
            -d|--debug)
                BUILD_TYPE="Debug"
                shift
                ;;
            --deps-only)
                DEPS_ONLY=true
                shift
                ;;
            *)
                echo "❌ 未知选项: $1"
                echo "运行 '$0 --help' 查看可用选项"
                exit 1
                ;;
        esac
    done
    
    # 执行相应操作
    check_environment
    
    if [ "$DEPS_ONLY" = true ]; then
        install_workspace_deps
        exit 0
    fi
    
    if [ "$GENERATE_ONLY" = true ]; then
        ./scripts/setup.sh --generate-adapters
        exit 0
    fi
    
    if [ "$CLEAN_FIRST" = true ]; then
        clean_build
    fi
    
    if [ "$QUICK_BUILD" = true ]; then
        quick_build
        exit 0
    fi
    
    # 标准编译流程
    ensure_adapters
    install_workspace_deps  
    build_project
}

main "$@"
