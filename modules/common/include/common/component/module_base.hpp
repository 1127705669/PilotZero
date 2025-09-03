#ifndef COMMON__COMPONENT__MODULE_BASE_HPP_
#define COMMON__COMPONENT__MODULE_BASE_HPP_

#include <rclcpp/rclcpp.hpp>
#include <yaml-cpp/yaml.h>
#include <string>
#include <memory>
#include <map>
#include <any>
#include <chrono>

#include "common/communication/data_buffer.hpp"
#include "common/type_adapters.hpp"  // 包含TypeAdapter定义

namespace common
{
namespace component
{

/**
 * @brief 纯净的算法模块基类 - 基于TypeAdapter的简化设计
 * 
 * 设计理念：
 * - 算法模块只使用Proto接口：get_latest() / publish() 
 * - TypeAdapter自动处理Proto ↔ ROS转换
 * - 配置驱动Key到Topic的映射
 * - 无需复杂的DataManager和AdapterFactory
 */
class ModuleBase : public rclcpp::Node
{
public:
  explicit ModuleBase(const rclcpp::NodeOptions & options);
  virtual ~ModuleBase() = default;

  // 算法模块生命周期接口
  virtual bool init() { return true; }
  virtual bool reset() { return true; }
  virtual bool stop() { return true; }
  virtual void process() = 0;

protected:
  // 时间获取
  double get_current_time() const
  {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration<double>(duration).count();
  }

  // ========== 通用TypeAdapter接口 - 基于Traits ==========
  
  /**
   * @brief 通用发布接口 - 使用TypeMapping自动推导ROS类型
   */
  template<typename ProtoType>
  void publish(const std::string & key, const ProtoType & proto_msg)
  {
    using Mapping = TypeMapping<ProtoType>;
    using TypeAdapterType = typename Mapping::TypeAdapterType;
    
    auto topic = get_topic_for_key(key);
    if (topic.empty()) {
      RCLCPP_WARN_ONCE(this->get_logger(), "No topic mapping for key: %s", key.c_str());
      return;
    }
    
    auto pub_key = key + "_pub_" + typeid(ProtoType).name();
    if (publishers_.find(pub_key) == publishers_.end()) {
      auto publisher = this->create_publisher<TypeAdapterType>(topic, 10);
      publishers_[pub_key] = publisher;
      RCLCPP_INFO(this->get_logger(), "Created TypeAdapter publisher: %s → %s", 
                  key.c_str(), topic.c_str());
    }
    
    auto it = publishers_.find(pub_key);
    if (it != publishers_.end()) {
      auto publisher = std::static_pointer_cast<rclcpp::Publisher<TypeAdapterType>>(it->second);
      publisher->publish(proto_msg);  // TypeAdapter自动转换
      RCLCPP_DEBUG(this->get_logger(), "Published proto: %s → %s", key.c_str(), topic.c_str());
    }
  }

  /**
   * @brief 通用订阅接口 - 使用TypeMapping自动推导ROS类型
   */
  template<typename ProtoType>
  bool get_latest(const std::string & key, ProtoType & data)
  {
    using Mapping = TypeMapping<ProtoType>;
    using TypeAdapterType = typename Mapping::TypeAdapterType;
    
    auto topic = get_topic_for_key(key);
    if (topic.empty()) {
      RCLCPP_WARN_ONCE(this->get_logger(), "No topic mapping for key: %s", key.c_str());
      return false;
    }
    
    auto sub_key = key + "_sub_" + typeid(ProtoType).name();
    auto buf_key = key + "_buf_" + typeid(ProtoType).name();
    
    if (subscriptions_.find(sub_key) == subscriptions_.end()) {
      auto buffer = std::make_shared<common::communication::DataBuffer<ProtoType>>();
      data_buffers_[buf_key] = buffer;
      
      auto subscription = this->create_subscription<TypeAdapterType>(
        topic, 10,
        [buffer](const ProtoType & proto_msg) {  // 直接接收Proto数据
          buffer->update(proto_msg);
        });
      
      subscriptions_[sub_key] = subscription;
      RCLCPP_INFO(this->get_logger(), "Created TypeAdapter subscription: %s ← %s", 
                  key.c_str(), topic.c_str());
    }
    
    auto buffer_it = data_buffers_.find(buf_key);
    if (buffer_it != data_buffers_.end()) {
      auto buffer = std::any_cast<std::shared_ptr<common::communication::DataBuffer<ProtoType>>>(buffer_it->second);
      return buffer->get_latest(data);
    }
    
    return false;
  }

  // 获取模块参数
  template<typename T>
  T get_parameter_value(const std::string & param_name, const T & default_value) const
  {
    if (module_params_[param_name]) {
      try {
        return module_params_[param_name].as<T>();
      } catch (const std::exception & e) {
        RCLCPP_WARN(this->get_logger(), 
                    "Failed to parse parameter %s: %s, using default", 
                    param_name.c_str(), e.what());
      }
    }
    return default_value;
  }

private:
  // Key到Topic映射结构
  struct KeyMapping {
    std::string topic;
    std::string proto_type;
    std::string ros_type;
  };

  // 初始化方法
  void load_module_params();
  void load_key_mappings();
  void init_module_timer();
  void timer_callback();
  
  // 根据Key获取Topic
  std::string get_topic_for_key(const std::string & key) const;
  
  // 生成类型相关的唯一键
  // 旧的模板函数已删除，使用宏生成的特化版本
  
  // 旧的通用实现已删除，使用宏生成的特化版本

  // 模块状态
  std::string module_name_;
  double module_frequency_ = 30.0;
  YAML::Node module_params_;
  rclcpp::TimerBase::SharedPtr timer_;
  
  // Key映射配置
  std::map<std::string, KeyMapping> key_mappings_;
  
  // 通用ROS通信组件存储
  std::map<std::string, std::any> data_buffers_;           // key_type → DataBuffer<ProtoType>
  std::map<std::string, rclcpp::SubscriptionBase::SharedPtr> subscriptions_; // key_type → Subscription<TypeAdapter>
  std::map<std::string, rclcpp::PublisherBase::SharedPtr> publishers_;        // key_type → Publisher<TypeAdapter>
};

// ========== 实现 ==========

inline ModuleBase::ModuleBase(const rclcpp::NodeOptions & options)
  : Node("pilot_module", options)
{
  // 方案1：从参数获取模块名（推荐）
  this->declare_parameter("module_name", "");
  module_name_ = this->get_parameter("module_name").as_string();
  
  // 方案2：如果参数为空，从节点名自动推导（去掉_node后缀）
  if (module_name_.empty()) {
    std::string node_name = this->get_name();
    if (node_name.size() > 5 && node_name.substr(node_name.size() - 5) == "_node") {
      module_name_ = node_name.substr(0, node_name.size() - 5);
    } else {
      module_name_ = node_name;
    }
  }
  
  RCLCPP_INFO(this->get_logger(), "Module: %s initializing with TypeAdapter...", module_name_.c_str());
  
  load_key_mappings();
  load_module_params();
  init_module_timer();
}

inline void ModuleBase::load_module_params()
{
  try {
    std::string config_path = "modules/common/config/pipeline.yaml";
    YAML::Node config = YAML::LoadFile(config_path);
    
    if (config["modules"] && config["modules"][module_name_]) {
      auto module_config = config["modules"][module_name_];
      
      if (module_config["frequency"]) {
        module_frequency_ = module_config["frequency"].as<double>();
      }
      
      if (module_config["parameters"]) {
        module_params_ = module_config["parameters"];
      }
      
      RCLCPP_INFO(this->get_logger(), "Loaded parameters for: %s", module_name_.c_str());
    }
  } catch (const std::exception & e) {
    RCLCPP_WARN(this->get_logger(), "Failed to load module params: %s", e.what());
  }
}

inline void ModuleBase::load_key_mappings()
{
  try {
    std::string config_path = "modules/common/config/pipeline.yaml";
    YAML::Node config = YAML::LoadFile(config_path);
    
    if (config["data_keys"]) {
      for (const auto& key_pair : config["data_keys"]) {
        std::string key = key_pair.first.as<std::string>();
        auto key_config = key_pair.second;
        
        KeyMapping mapping;
        mapping.topic = key_config["ros_topic"].as<std::string>();
        
        key_mappings_[key] = mapping;
        
        RCLCPP_INFO(this->get_logger(), "Loaded key mapping: %s → %s", 
                    key.c_str(), mapping.topic.c_str());
      }
    }
  } catch (const std::exception & e) {
    RCLCPP_WARN(this->get_logger(), "Failed to load key mappings: %s", e.what());
  }
}

inline void ModuleBase::init_module_timer()
{
  // 创建定时器
  auto period = std::chrono::duration<double>(1.0 / module_frequency_);
  timer_ = this->create_wall_timer(
    std::chrono::duration_cast<std::chrono::nanoseconds>(period),
    std::bind(&ModuleBase::timer_callback, this));
  
  RCLCPP_INFO(this->get_logger(), "Module %s initialized at %.1f Hz with TypeAdapter", 
              module_name_.c_str(), module_frequency_);
}

inline std::string ModuleBase::get_topic_for_key(const std::string & key) const
{
  auto it = key_mappings_.find(key);
  return (it != key_mappings_.end()) ? it->second.topic : "";
}

inline void ModuleBase::timer_callback()
{
  process();
}

// ========== 通用TypeAdapter实现完成 ==========
// 所有类型通过TypeMapping Traits自动推导，无需硬编码特化

}  // namespace component
}  // namespace common

#endif  // COMMON__COMPONENT__MODULE_BASE_HPP_