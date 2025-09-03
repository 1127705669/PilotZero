#include <sensor_msgs/msg/imu.hpp>
#include <rclcpp/rclcpp.hpp>
#include <pilot_msgs/localization/imu_data.pb.h>

/**
 * @file imu_converter.cpp
 * @brief IMU数据转换胶水代码具体实现
 * 
 * 专门负责sensor_msgs::Imu ↔ pilot_msgs::proto::ImuData的转换
 * 被TypeAdapter通过宏自动调用
 */

namespace common
{
namespace converters
{

// ========== IMU Proto → ROS 转换实现 ==========

void convert_imu_proto_to_ros(const pilot_msgs::localization::ImuData& proto_msg, 
                              sensor_msgs::msg::Imu& ros_msg)
{
  // ========== 具体的Proto → ROS 胶水代码 ==========
  
  // 时间戳和frame转换（使用Header）
  ros_msg.header.stamp = rclcpp::Time(static_cast<int64_t>(proto_msg.header().timestamp() * 1e9));
  ros_msg.header.frame_id = proto_msg.header().frame_id().empty() ? "imu_link" : proto_msg.header().frame_id();
  
  // 姿态四元数转换
  ros_msg.orientation.x = proto_msg.orientation().x();
  ros_msg.orientation.y = proto_msg.orientation().y();
  ros_msg.orientation.z = proto_msg.orientation().z();
  ros_msg.orientation.w = proto_msg.orientation().w();
  
  // 角速度转换
  ros_msg.angular_velocity.x = proto_msg.angular_velocity().x();
  ros_msg.angular_velocity.y = proto_msg.angular_velocity().y();
  ros_msg.angular_velocity.z = proto_msg.angular_velocity().z();
  
  // 线性加速度转换
  ros_msg.linear_acceleration.x = proto_msg.linear_acceleration().x();
  ros_msg.linear_acceleration.y = proto_msg.linear_acceleration().y();
  ros_msg.linear_acceleration.z = proto_msg.linear_acceleration().z();
  
  // 协方差矩阵转换
  for (size_t i = 0; i < 9; ++i) {
    // 姿态协方差
    if (i < static_cast<size_t>(proto_msg.orientation_covariance_size())) {
      ros_msg.orientation_covariance[i] = proto_msg.orientation_covariance(static_cast<int>(i));
    } else {
      ros_msg.orientation_covariance[i] = 0.0;  // 默认值
    }
    
    // 角速度协方差  
    if (i < static_cast<size_t>(proto_msg.angular_velocity_covariance_size())) {
      ros_msg.angular_velocity_covariance[i] = proto_msg.angular_velocity_covariance(static_cast<int>(i));
    } else {
      ros_msg.angular_velocity_covariance[i] = 0.0;  // 默认值
    }
    
    // 线性加速度协方差
    if (i < static_cast<size_t>(proto_msg.linear_acceleration_covariance_size())) {
      ros_msg.linear_acceleration_covariance[i] = proto_msg.linear_acceleration_covariance(static_cast<int>(i));
    } else {
      ros_msg.linear_acceleration_covariance[i] = 0.0;  // 默认值
    }
  }
}

// ========== IMU ROS → Proto 转换实现 ==========

void convert_imu_ros_to_proto(const sensor_msgs::msg::Imu& ros_msg,
                              pilot_msgs::localization::ImuData& proto_msg)
{
  // ========== 具体的ROS → Proto 胶水代码 ==========
  
  // 时间戳和Header转换
  auto* header = proto_msg.mutable_header();
  header->set_timestamp(ros_msg.header.stamp.sec + ros_msg.header.stamp.nanosec * 1e-9);
  header->set_frame_id(ros_msg.header.frame_id);
  
  // 姿态四元数转换
  auto* orientation = proto_msg.mutable_orientation();
  orientation->set_x(ros_msg.orientation.x);
  orientation->set_y(ros_msg.orientation.y);
  orientation->set_z(ros_msg.orientation.z);
  orientation->set_w(ros_msg.orientation.w);
  
  // 角速度转换
  auto* angular_vel = proto_msg.mutable_angular_velocity();
  angular_vel->set_x(ros_msg.angular_velocity.x);
  angular_vel->set_y(ros_msg.angular_velocity.y);
  angular_vel->set_z(ros_msg.angular_velocity.z);
  
  // 线性加速度转换
  auto* linear_acc = proto_msg.mutable_linear_acceleration();
  linear_acc->set_x(ros_msg.linear_acceleration.x);
  linear_acc->set_y(ros_msg.linear_acceleration.y);
  linear_acc->set_z(ros_msg.linear_acceleration.z);
  
  // 协方差矩阵转换
  for (size_t i = 0; i < 9; ++i) {
    proto_msg.add_orientation_covariance(ros_msg.orientation_covariance[i]);
    proto_msg.add_angular_velocity_covariance(ros_msg.angular_velocity_covariance[i]);
    proto_msg.add_linear_acceleration_covariance(ros_msg.linear_acceleration_covariance[i]);
  }
}

}  // namespace converters
}  // namespace common
