#ifndef CONTROL__CONTROL_COMPONENT_HPP_
#define CONTROL__CONTROL_COMPONENT_HPP_

#include <rclcpp/rclcpp.hpp>
#include "common/component/module_base.hpp"
#include "common/component/component_register.hpp"

// 包含pilot_msgs的control_command proto头文件
#include <pilot_msgs/control/control_command.pb.h>

namespace control
{

/**
 * @brief 控制组件 - 最小化实现
 *
 * 当前版本：
 * - 使用纯Proto接口：publish("control_command", proto_data)
 * - TypeAdapter自动处理Proto ↔ ROS转换
 * - 配置驱动的Key到Topic映射
 * - 发布控制指令到CARLA
 */
class ControlComponent : public common::component::ModuleBase
{
public:
  explicit ControlComponent(const rclcpp::NodeOptions & options);
  ~ControlComponent() = default;

  // ModuleBase lifecycle methods
  bool init() override;
  bool reset() override;
  bool stop() override;
  void process() override;

private:
  // 核心控制算法方法
  void generate_control_command(pilot_msgs::control::ControlCommand& control_cmd);

  // 模块状态
  bool is_initialized_ = false;
  uint64_t process_count_ = 0;

  // 控制参数
  double control_gain_d_ = 0.05;
};

}  // namespace control

#endif  // CONTROL__CONTROL_COMPONENT_HPP_
