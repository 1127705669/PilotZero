# PilotZero Project Architecture

## Overview

PilotZero is a modern autonomous driving system based on ROS2 Humble, featuring configuration-driven modular architecture.

## Core Features

### 1. Configuration-Driven Architecture
- **Single Configuration Source**: `pipeline.yaml` defines all module configurations
- **No Hard-coding**: Module parameters, I/O, and frequencies are fully configurable
- **Dynamic Reload**: Supports runtime configuration updates

### 2. Lifecycle Management
```cpp
// Unified module lifecycle interface
virtual bool init();    // Initialization
virtual bool reset();   // Reset state
virtual bool stop();    // Stop and cleanup
virtual void process(); // Main processing loop
```

### 3. Type-Safe Communication
- **Protocol Buffers**: High-performance serialization
- **Data Buffers**: Thread-safe inter-module communication
- **Topic Mapping**: Automatic ROS2 topic management

## Architecture Components

### ModuleBase Framework
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

### Configuration System
All modules are configured via `pipeline.yaml`:
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

### Component Registration
Unified registration system:
```cpp
// Register component with configuration key
PILOT_REGISTER_COMPONENT_WITH_CONFIG(control::ControlComponent, controller)
```

## Data Flow

1. **Configuration Loading**: Modules read parameters from `pipeline.yaml`
2. **I/O Setup**: Automatic publisher/subscriber creation based on configuration
3. **Timer Creation**: Frequency-based timer setup for `process()` calls
4. **Data Exchange**: Protocol Buffers messages via ROS2 topics

## Module Development

### Creating New Modules
1. Inherit from `ModuleBase`
2. Implement lifecycle methods (`init`, `process`, etc.)
3. Add configuration to `pipeline.yaml`
4. Register with `PILOT_REGISTER_COMPONENT_WITH_CONFIG`

### Example Module
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
  // Main algorithm logic
}

PILOT_REGISTER_COMPONENT_WITH_CONFIG(MyComponent, my_module)
```

## Build System

- **Colcon**: Standard ROS2 build system
- **CMake**: Modern CMake with proper target management
- **Dependencies**: Clean separation of build and runtime dependencies

## Quality Standards

- **English-only**: Professional codebase with English comments and logs
- **Clean Architecture**: Proper separation of concerns
- **Type Safety**: Protocol Buffers and strong typing
- **Performance**: Optimized for real-time autonomous driving requirements