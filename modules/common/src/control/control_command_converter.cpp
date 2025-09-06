#include <carla_msgs/msg/carla_ego_vehicle_control.hpp>
#include <rclcpp/rclcpp.hpp>
#include <pilot_msgs/control/control_command.pb.h>

/**
 * @file control_command_converter.cpp
 * @brief 控制指令数据转换胶水代码具体实现
 *
 * 实现pilot_msgs::control::ControlCommand ↔ carla_msgs::msg::CarlaEgoVehicleControl的具体转换逻辑
 * 胶水代码集中在common层管理，按算法模块分子文件夹组织
 */

namespace common
{
namespace converters
{

// ========== ControlCommand Proto → ROS (carla_msgs::msg::CarlaEgoVehicleControl) 转换实现 ==========

void convert_control_command_proto_to_ros(const pilot_msgs::control::ControlCommand& proto_msg,
                                          carla_msgs::msg::CarlaEgoVehicleControl& ros_msg)
{
  // ========== 具体的Proto → ROS 胶水代码 ==========

  // 时间戳转换
  ros_msg.header.stamp = rclcpp::Time(static_cast<int64_t>(proto_msg.header().timestamp() * 1e9));
  ros_msg.header.frame_id = proto_msg.header().frame_id().empty() ? "ego_vehicle" : proto_msg.header().frame_id();

  // 控制参数转换
  ros_msg.throttle = proto_msg.throttle();
  ros_msg.brake = proto_msg.brake();
  ros_msg.steer = proto_msg.steer();
  ros_msg.hand_brake = proto_msg.hand_brake();
  ros_msg.reverse = proto_msg.reverse();
  ros_msg.gear = proto_msg.gear();
  ros_msg.manual_gear_shift = proto_msg.manual_gear_shift();
}

// ========== ControlCommand ROS (carla_msgs::msg::CarlaEgoVehicleControl) → Proto 转换实现 ==========

void convert_control_command_ros_to_proto(const carla_msgs::msg::CarlaEgoVehicleControl& ros_msg,
                                          pilot_msgs::control::ControlCommand& proto_msg)
{
  // ========== 具体的ROS → Proto 胶水代码 ==========

  // Header转换
  auto* header = proto_msg.mutable_header();
  header->set_timestamp(ros_msg.header.stamp.sec + ros_msg.header.stamp.nanosec * 1e-9);
  header->set_frame_id(ros_msg.header.frame_id);

  // 控制参数转换
  proto_msg.set_throttle(ros_msg.throttle);
  proto_msg.set_brake(ros_msg.brake);
  proto_msg.set_steer(ros_msg.steer);
  proto_msg.set_hand_brake(ros_msg.hand_brake);
  proto_msg.set_reverse(ros_msg.reverse);
  proto_msg.set_gear(ros_msg.gear);
  proto_msg.set_manual_gear_shift(ros_msg.manual_gear_shift);
}

}  // namespace converters
}  // namespace common
