# PilotZero 项目架构

## 概述

PilotZero 是基于 ROS2 Humble 的现代化自动驾驶系统，采用配置驱动的模块化架构设计。

## 核心特性

### 1. 配置驱动架构
- **单一配置源**：`pipeline.yaml` 定义所有模块配置
- **无硬编码**：模块参数、输入输出、频率完全配置化
- **动态重载**：支持运行时配置更新

### 2. 生命周期管理
```cpp
// 统一的模块生命周期接口
virtual bool init();    // 初始化
virtual bool reset();   // 重置状态  
virtual bool stop();    // 停止清理
virtual void process(); // 主处理循环
```

### 3. 类型安全通信
- **Protocol Buffers**：高性能序列化
- **数据缓冲**：线程安全的模块间通信
- **话题映射**：自动 ROS2 话题管理

## 架构组件

### ModuleBase 框架
```cpp
class ModuleBase : public rclcpp::Node {
protected:
  double get_current_time() const;
  
  template<typename T>
  T get_parameter_value(const std::string& name, const T& default_val);
  
  template<typename ProtoType>
  void publish(const ProtoType& msg, const std::string& topic);
  
  template<typename ProtoType>
  bool get_input(const std::string& topic, ProtoType& data);
};
```

### 配置系统
所有模块通过 `pipeline.yaml` 配置：
```yaml
modules:
  controller:
    package: control
    plugin: control::ControllerComponent  
    frequency: 20.0
    parameters:
      control_gain_p: 0.8
      max_steering_angle: 0.5
    inputs:
      - topic: "/planning/trajectory"
        type: "Trajectory"
    outputs:
      - topic: "/control/command" 
        type: "ControlCommand"
```

### 组件注册
统一注册系统：
```cpp
// 使用配置键注册组件
PILOT_REGISTER_COMPONENT_WITH_CONFIG(control::ControlComponent, controller)
```

## 数据流程

1. **配置加载**：模块从 `pipeline.yaml` 读取参数
2. **I/O 设置**：根据配置自动创建发布器/订阅器
3. **定时器创建**：基于频率设置的 `process()` 调用定时器
4. **数据交换**：通过 ROS2 话题传递 Protocol Buffers 消息

## 模块开发

### 创建新模块
1. 继承 `ModuleBase`
2. 实现生命周期方法（`init`、`process` 等）
3. 在 `pipeline.yaml` 中添加配置
4. 使用 `PILOT_REGISTER_COMPONENT_WITH_CONFIG` 注册

### 示例模块
```cpp
class MyComponent : public common::component::ModuleBase {
public:
  explicit MyComponent(const rclcpp::NodeOptions& options);
  
  bool init() override;
  void process() override;
  
private:
  double my_param_;
};

bool MyComponent::init() {
  my_param_ = get_parameter_value("my_param", 1.0);
  return true;
}

void MyComponent::process() {
  // 主要算法逻辑
}

PILOT_REGISTER_COMPONENT_WITH_CONFIG(MyComponent, my_module)
```

## 构建系统

- **Colcon**：标准 ROS2 构建系统
- **CMake**：现代 CMake 与合适的目标管理
- **依赖管理**：构建和运行时依赖的清晰分离

## 质量标准

- **英文代码库**：专业的代码库，使用英文注释和日志
- **清晰架构**：合理的关注点分离
- **类型安全**：Protocol Buffers 和强类型系统
- **性能优化**：针对实时自动驾驶需求的优化