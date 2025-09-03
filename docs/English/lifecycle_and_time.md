# PilotZero Lifecycle and Time Management

## Overview

PilotZero implements comprehensive lifecycle management and time interfaces:

1. **Module Lifecycle Management**: `init()`, `reset()`, `stop()` virtual functions
2. **Unified Time Interface**: `get_current_time()` for timestamp access
3. **Configuration-Driven Parameters**: Module parameters from `pipeline.yaml`
4. **Data Timestamps**: All Protocol Buffer messages include timestamp fields

## Core Improvements

### 1. ModuleBase Lifecycle Methods

```cpp
class ModuleBase : public rclcpp::Node {
public:
  // Lifecycle methods for subclasses to override
  virtual bool init() { return true; }    // Initialization
  virtual bool reset() { return true; }   // Reset state
  virtual bool stop() { return true; }    // Stop and cleanup
  virtual void process() = 0;             // Main processing (pure virtual)

protected:
  double get_current_time() const;
};
```

### 2. Simplified Initialization Pattern

**Clean Constructor Pattern**:
```cpp
ControlComponent::ControlComponent(const rclcpp::NodeOptions& options)
  : ModuleBase(options) {
  init();  // Direct initialization in constructor
  RCLCPP_INFO(this->get_logger(), "ControlComponent constructor completed");
}

bool ControlComponent::init() {
  control_frequency_ = get_parameter_value("control_frequency", 20.0);
  enable_debug_output_ = get_parameter_value("enable_debug_output", true);
  
  RCLCPP_INFO(this->get_logger(), 
    "ControlComponent initialized - frequency: %.1f Hz, debug: %s",
    control_frequency_, enable_debug_output_ ? "enabled" : "disabled");
  
  is_initialized_ = true;
  return true;
}
```

### 3. Time Management

**Unified Time Interface**:
```cpp
// Get current time in seconds (steady clock)
double get_current_time() const {
  auto now = std::chrono::steady_clock::now();
  auto duration = now.time_since_epoch();
  return std::chrono::duration<double>(duration).count();
}
```

**Usage in Components**:
```cpp
void ControlComponent::process() {
  double current_time = get_current_time();
  
  // Create timestamped command
  control::proto::ControlCommand cmd;
  cmd.set_timestamp(current_time);
  cmd.set_command_id(process_count_);
  
  // Process and publish
}
```

### 4. Configuration System

**Parameter Loading**:
```cpp
template<typename T>
T get_parameter_value(const std::string& param_name, const T& default_value) const {
  if (module_params_[param_name]) {
    try {
      return module_params_[param_name].as<T>();
    } catch (const std::exception& e) {
      RCLCPP_WARN(this->get_logger(), 
                  "Failed to parse parameter %s: %s, using default value", 
                  param_name.c_str(), e.what());
    }
  }
  return default_value;
}
```

## Implementation Details

### 1. Lifecycle Flow

1. **Construction**: `ModuleBase` constructor loads configuration
2. **Initialization**: Subclass `init()` method called from constructor
3. **Runtime**: Timer-driven `process()` calls at configured frequency
4. **Shutdown**: `stop()` method for cleanup

### 2. Timer-Based Processing

```cpp
// Timer created based on configuration frequency
auto period = std::chrono::duration<double>(1.0 / module_frequency_);
timer_ = this->create_wall_timer(
  std::chrono::duration_cast<std::chrono::nanoseconds>(period),
  std::bind(&ModuleBase::timer_callback, this));

void ModuleBase::timer_callback() {
  process();  // Direct call, no state checking
}
```

### 3. Data Freshness

**Timestamp Checking Example**:
```cpp
void PlannerComponent::process() {
  localization::proto::EgoState ego_state;
  if (!get_input("/ego_state", ego_state)) {
    return;
  }
  
  // Check data freshness
  double current_time = get_current_time();
  if (ego_state.has_timestamp()) {
    double data_age = current_time - ego_state.timestamp();
    if (data_age > 0.5) {  // 500ms threshold
      RCLCPP_WARN(this->get_logger(), "Ego state data too old: %.3fs", data_age);
      return;
    }
  }
  
  // Process fresh data...
}
```

## Best Practices

### 1. Always Use Timestamps
- Set timestamps when creating messages
- Check data freshness before processing
- Use `get_current_time()` for consistency

### 2. Proper Lifecycle Implementation
- Initialize in constructor via `init()`
- Clean up resources in `stop()`
- Reset state properly in `reset()`

### 3. Configuration-Driven Development
- Read all parameters in `init()`
- Use meaningful default values
- Log parameter values for debugging

### 4. Professional Logging
- Use English-only log messages
- Include relevant context (timestamps, values)
- Use appropriate log levels (INFO, WARN, ERROR)

## Example: Complete Component

```cpp
class ExampleComponent : public common::component::ModuleBase {
public:
  explicit ExampleComponent(const rclcpp::NodeOptions& options)
    : ModuleBase(options) {
    init();
    RCLCPP_INFO(get_logger(), "ExampleComponent ready");
  }
  
  bool init() override {
    frequency_ = get_parameter_value("frequency", 10.0);
    timeout_ = get_parameter_value("timeout", 1.0);
    
    RCLCPP_INFO(get_logger(), "Initialized - freq: %.1fHz, timeout: %.1fs", 
                frequency_, timeout_);
    return true;
  }
  
  void process() override {
    double current_time = get_current_time();
    
    // Algorithm implementation
    example::proto::Result result;
    result.set_timestamp(current_time);
    
    publish(result, "/example/output");
  }
  
private:
  double frequency_;
  double timeout_;
};

PILOT_REGISTER_COMPONENT_WITH_CONFIG(ExampleComponent, example)
```

This architecture provides clean, maintainable, and professional autonomous driving system development.
