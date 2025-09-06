#include "control/control_component.hpp"
#include <rclcpp/rclcpp.hpp>
#include "common/component/component_register.hpp"

namespace control
{

ControlComponent::ControlComponent(const rclcpp::NodeOptions & options)
  : common::component::ModuleBase(options)
{
  init();
  RCLCPP_INFO(this->get_logger(), "ControlComponent constructor completed");
}

bool ControlComponent::init()
{
  RCLCPP_INFO(this->get_logger(), "Initializing ControlComponent...");

  // 获取参数
  control_gain_d_ = get_parameter_value<double>("control_gain_d", 0.05);

  RCLCPP_INFO(this->get_logger(), "ControlComponent initialized - TypeAdapter pure Proto interface");
  RCLCPP_INFO(this->get_logger(), "  Control Gain D: %.3f", control_gain_d_);

  is_initialized_ = true;
  return true;
}

bool ControlComponent::reset()
{
  RCLCPP_INFO(this->get_logger(), "Resetting ControlComponent...");

  // 重置数据标志
  process_count_ = 0;

  // 重新初始化
  bool success = init();
  RCLCPP_INFO(this->get_logger(), "ControlComponent reset completed");
  return success;
}

bool ControlComponent::stop()
{
  RCLCPP_INFO(this->get_logger(), "Stopping ControlComponent...");

  is_initialized_ = false;

  RCLCPP_INFO(this->get_logger(), "ControlComponent stopped, processed %lu cycles", process_count_);
  return true;
}

void ControlComponent::process()
{
  if (!is_initialized_) {
    return;
  }

  double current_time = get_current_time();
  process_count_++;

  // ========== TypeAdapter风格的纯Proto接口 ==========

  // 生成控制指令Proto数据
  pilot_msgs::control::ControlCommand control_cmd;
  generate_control_command(control_cmd);

  // 发布控制指令（TypeAdapter自动转换为ROS消息）
  publish("control_command", control_cmd);

  // 定期输出状态
  if (process_count_ % 10 == 0) {  // 每秒输出一次 (10Hz / 10)
    RCLCPP_INFO(this->get_logger(),
      "Control running - time: %.3f, throttle: %.2f, steer: %.2f, processed: %lu",
      current_time,
      control_cmd.throttle(),
      control_cmd.steer(),
      process_count_);
  }
}

void ControlComponent::generate_control_command(pilot_msgs::control::ControlCommand& control_cmd)
{
  // 设置消息头
  auto* header = control_cmd.mutable_header();
  header->set_timestamp(get_current_time());
  header->set_frame_id("ego_vehicle");

  // 简单的控制逻辑示例
  // TODO: 实现具体的控制算法
  
  // 示例：简单的定速控制
  control_cmd.set_throttle(0.3);      // 30%油门
  control_cmd.set_brake(0.0);         // 不刹车
  control_cmd.set_steer(0.0);         // 直行
  control_cmd.set_hand_brake(false);  // 不拉手刹
  control_cmd.set_reverse(false);     // 不倒车
  control_cmd.set_gear(1);            // 前进档
  control_cmd.set_manual_gear_shift(false); // 自动换档

  RCLCPP_DEBUG(this->get_logger(), "Generated control command at timestamp: %.3f", 
               control_cmd.header().timestamp());
}

}  // namespace control

// 注册组件
PILOT_REGISTER_COMPONENT_WITH_CONFIG(control::ControlComponent, control)