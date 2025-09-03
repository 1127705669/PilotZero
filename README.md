# PilotZero Autonomous Driving System

现代自动驾驶通信架构，基于ROS2 + TypeAdapter + Protocol Buffers，实现算法与通信的完全解耦。

## 🚀 TypeAdapter架构特色

- **🔄 自动转换**: ROS消息 ↔ Proto数据，算法层完全使用Proto接口
- **📝 配置驱动**: `pipeline.yaml`一处配置，自动生成所有TypeAdapter代码
- **🏗️ 零循环依赖**: 统一Proto包架构，模块间清晰依赖关系
- **⚡ 高效通信**: TypeAdapter编译期优化，零运行时开销
- **🎯 完全通用**: 框架支持任意算法模块，无硬编码业务逻辑

## ✅ 系统验证状态

- **✅ TypeAdapter系统**: 实时处理CARLA IMU数据，30Hz稳定运行
- **✅ 统一Proto包**: 4个模块成功编译，0循环依赖
- **✅ 配置化管理**: 从YAML自动生成注册表和映射关系
- **✅ 算法接口纯净**: `get_latest("imu_data", proto_data)` 简洁易用
- **✅ 工程化工具**: 自动化build/setup脚本，开发效率高

## 🏗️ 架构组成

- **统一Proto包** (`modules/pilot_msgs/`): 按算法模块组织的Proto定义
- **TypeAdapter框架** (`modules/common/`): 自动转换和通用模块基类  
- **算法模块** (`modules/localization/`): 纯Proto接口的业务逻辑
- **自动化工具** (`scripts/`): 代码生成和项目管理脚本

## ⚡ 快速开始

### 1) 环境安装
```bash
# 安装ROS2环境和所有依赖
./scripts/setup.sh install-env
source ~/.bashrc  # 重新加载环境
```

### 2) 编译项目  
```bash
# 标准编译（自动生成TypeAdapter）
./scripts/build.sh

# 或快速编译
./scripts/setup.sh --quick-build
```

### 3) 运行系统
```bash
# 启动定位模块（TypeAdapter演示）
source install/setup.bash
ros2 launch pilot_launch localization.launch.py

# 查看IMU数据转换：CARLA ROS消息 → Proto数据
```

## 🔧 开发工具

### TypeAdapter管理
```bash
# 查看当前注册状态
./scripts/setup.sh --adapter-status

# 重新生成注册表  
./scripts/setup.sh --generate-adapters

# 添加新TypeAdapter（会给出配置模板）
./scripts/setup.sh --add-adapter gps pilot_msgs::localization::GpsData sensor_msgs::msg::NavSatFix
```

### 项目管理
```bash
# 清理构建
./scripts/setup.sh --clean

# 查看活跃topics
./scripts/setup.sh --topic-list

# Debug编译
./scripts/build.sh --debug
```

## Project Structure

```
PilotZero/                                    # Standard ROS2 workspace
├── modules/                                  # Algorithm modules (3 ROS2 packages)
│   ├── common/                               # Core framework package
│   │   ├── include/common/
│   │   │   ├── component/module_base.hpp     # Module base class
│   │   │   └── communication/data_buffer.hpp # Data buffer system
│   │   ├── config/                           # System configuration
│   │   │   ├── pipeline.yaml                 # Module pipeline config
│   │   │   ├── qos_profiles.yaml            # ROS2 QoS settings
│   │   │   └── dds_profile.xml              # DDS configuration
│   │   └── proto/common.proto                # Common data types
│   ├── control/                              # Vehicle control module
│   │   ├── include/control/control_component.hpp
│   │   ├── src/control_component.cpp
│   │   └── proto/control_command.proto
│   └── planning/                             # Path planning module
│       ├── include/planning/planner_component.hpp
│       ├── src/planner_component.cpp
│       └── proto/trajectory.proto
├── pilot_launch/                             # Launch files
│   └── launch/
│       ├── control.launch.py
│       └── planning.launch.py
├── scripts/                                  # Build scripts
│   ├── setup.sh                             # Install dependencies
│   └── build.sh                             # Compile project
└── docs/                                     # Documentation
    ├── Chinese/                              # 中文文档
    │   ├── project_architecture.md
    │   └── lifecycle_and_time.md
    └── English/                              # English Documentation  
        ├── project_architecture.md
        └── lifecycle_and_time.md
```

## Quick Start

### 1. Install Dependencies

```bash
# Clone the repository
git clone <repository-url> PilotZero
cd PilotZero

# Install ROS2 Humble and all dependencies
./scripts/setup.sh
```

### 2. Build the Project

```bash
# Source ROS2 environment
source /opt/ros/humble/setup.bash

# Build all modules
./scripts/build.sh
```

### 3. Run Components

```bash
# Source the workspace
source install/setup.bash

# Launch control component
ros2 launch pilot_launch control.launch.py

# Or launch planning component
ros2 launch pilot_launch planning.launch.py
```

### 4. Verify Installation

```bash
# Check running nodes
ros2 node list

# View topics
ros2 topic list

# Check installed packages
ros2 pkg list | grep -E "(common|control|planning)"
```

## Architecture Overview

### Core Framework (`common` module)
- **ModuleBase**: Base class for all components with lifecycle management
- **DataBuffer**: Thread-safe data exchange system
- **Component Registration**: Unified component registration macros
- **Configuration System**: YAML-based parameter management

### Active Modules
- **Control**: Vehicle control command generation (20Hz)
- **Planning**: Trajectory planning and path generation (10Hz)

### Key Features
- Clean English-only codebase
- Professional logging without decorative icons
- Simplified initialization patterns
- Configuration-driven parameters
- Standard ROS2 component architecture

## Development

### Adding New Modules

1. Create module directory: `modules/your_module/`
2. Add `package.xml` and `CMakeLists.txt`
3. Implement component inheriting from `ModuleBase`
4. Register component with `PILOT_REGISTER_COMPONENT`
5. Add configuration to `pipeline.yaml`

### Module Template

```cpp
#include "common/component/module_base.hpp"

class YourComponent : public common::component::ModuleBase
{
public:
  explicit YourComponent(const rclcpp::NodeOptions & options);
  
  bool init() override;
  void process() override;
};
```

## Dependencies

- **ROS2 Humble**: Core robotics framework
- **Protocol Buffers**: High-performance serialization
- **YAML-CPP**: Configuration file parsing
- **Colcon**: Build system

## License

MIT License - see LICENSE file for details

## Contributing

1. Follow the clean code standards (English-only, professional logging)
2. Inherit from `ModuleBase` for new components
3. Use configuration-driven parameters
4. Maintain clean separation of concerns
5. Test with provided scripts

---
*Clean, professional, ready for autonomous driving development*