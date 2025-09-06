#include "localization/localization_component.hpp"
#include <rclcpp/rclcpp.hpp>
#include <cmath>
#include "common/component/component_register.hpp"

namespace localization
{

LocalizationComponent::LocalizationComponent(const rclcpp::NodeOptions & options)
  : common::component::ModuleBase(options)
{
  init();
  RCLCPP_INFO(this->get_logger(), "LocalizationComponent constructor completed");
}

bool LocalizationComponent::init()
{
  RCLCPP_INFO(this->get_logger(), "Initializing LocalizationComponent...");
  
  // 注意：TypeAdapter架构完全自动化，算法只需使用get_latest(key, proto_data)
  
  RCLCPP_INFO(this->get_logger(), "LocalizationComponent initialized - TypeAdapter pure Proto interface");
  
  is_initialized_ = true;
  return true;
}



bool LocalizationComponent::reset()
{
  RCLCPP_INFO(this->get_logger(), "Resetting LocalizationComponent...");
  
  // 重置数据标志
  has_imu_data_ = false;
  process_count_ = 0;
  
  // 重新初始化
  bool success = init();
  RCLCPP_INFO(this->get_logger(), "LocalizationComponent reset completed");
  return success;
}

bool LocalizationComponent::stop()
{
  RCLCPP_INFO(this->get_logger(), "Stopping LocalizationComponent...");
  
  is_initialized_ = false;
  
  RCLCPP_INFO(this->get_logger(), "LocalizationComponent stopped, processed %lu cycles", process_count_);
  return true;
}

void LocalizationComponent::process()
{
  if (!is_initialized_) {
    return;
  }

  double current_time = get_current_time();
  process_count_++;
  
  // ========== TypeAdapter风格的纯Proto接口 ==========
  
  // 直接获取IMU Proto数据（TypeAdapter自动从ROS消息转换）
  pilot_msgs::localization::ImuData imu_data;
  bool has_imu = get_latest("imu_data", imu_data);
  
  // 直接获取GPS Proto数据（TypeAdapter自动从ROS消息转换）
  // pilot_msgs::localization::GpsData gps_data;  
  // bool has_gps = get_latest("gps_data", gps_data);
  
  // 更新数据状态
  has_imu_data_ = has_imu;
  
  // 处理IMU数据
  if (has_imu_data_) {
    process_imu_data(imu_data);
  }
  
  // 处理GPS数据
  // if (has_gps) {
  //   process_gps_data(gps_data);
  // }
  
  // 定期输出状态
  if (process_count_ % 30 == 0) {  // 每秒输出一次
    RCLCPP_INFO(this->get_logger(), 
      "Localization running - time: %.3f, IMU: %s, processed: %lu",
      current_time,
      has_imu_data_ ? "✓" : "✗",
      process_count_);
  }
}

void LocalizationComponent::process_imu_data(const pilot_msgs::localization::ImuData& imu_data)
{
  imu_process_count_++;
  
  // 提取IMU数据
  const auto& header = imu_data.header();
  const auto& orientation = imu_data.orientation();
  const auto& angular_vel = imu_data.angular_velocity();
  const auto& linear_acc = imu_data.linear_acceleration();
  
  // 每5秒打印一次详细的IMU信息（假设30Hz频率，即150次）
  if (imu_process_count_ % 150 == 1) {
    RCLCPP_INFO(this->get_logger(), 
      "📍 IMU Data - Time: %.3f, Frame: %s\n"
      "   🧭 Orientation (Quaternion): x=%.3f, y=%.3f, z=%.3f, w=%.3f\n"
      "   🔄 Angular Velocity: x=%.3f, y=%.3f, z=%.3f rad/s\n"
      "   ⚡ Linear Acceleration: x=%.3f, y=%.3f, z=%.3f m/s²",
      header.timestamp(),
      header.frame_id().c_str(),
      orientation.x(), orientation.y(), orientation.z(), orientation.w(),
      angular_vel.x(), angular_vel.y(), angular_vel.z(),
      linear_acc.x(), linear_acc.y(), linear_acc.z()
    );
    
    // 计算欧拉角（Roll, Pitch, Yaw）便于理解姿态
    double roll, pitch, yaw;
    quaternion_to_euler(orientation, roll, pitch, yaw);
    
    RCLCPP_INFO(this->get_logger(),
      "   📐 Euler Angles: Roll=%.1f°, Pitch=%.1f°, Yaw=%.1f°",
      roll * 180.0 / M_PI, pitch * 180.0 / M_PI, yaw * 180.0 / M_PI
    );
    
    RCLCPP_INFO(this->get_logger(),
      "   📊 IMU Statistics: Processed %lu messages", imu_process_count_
    );
  }
  
  // Debug级别：实时打印简要信息
  RCLCPP_DEBUG(this->get_logger(), "IMU: %.3f | Yaw: %.1f°", 
               header.timestamp(), 
               std::atan2(2*(orientation.w()*orientation.z() + orientation.x()*orientation.y()), 
                         1-2*(orientation.y()*orientation.y() + orientation.z()*orientation.z())) * 180.0 / M_PI);
}

void LocalizationComponent::quaternion_to_euler(const pilot_msgs::common::Quaternion& q, 
                                                double& roll, double& pitch, double& yaw)
{
  // 四元数到欧拉角转换 (ZYX旋转顺序)
  double qx = q.x();
  double qy = q.y();
  double qz = q.z();
  double qw = q.w();
  
  // Roll (x轴旋转)
  double sinr_cosp = 2 * (qw * qx + qy * qz);
  double cosr_cosp = 1 - 2 * (qx * qx + qy * qy);
  roll = std::atan2(sinr_cosp, cosr_cosp);
  
  // Pitch (y轴旋转)
  double sinp = 2 * (qw * qy - qz * qx);
  if (std::abs(sinp) >= 1) {
    pitch = std::copysign(M_PI / 2, sinp); // 使用90度如果超出范围
  } else {
    pitch = std::asin(sinp);
  }
  
  // Yaw (z轴旋转)
  double siny_cosp = 2 * (qw * qz + qx * qy);
  double cosy_cosp = 1 - 2 * (qy * qy + qz * qz);
  yaw = std::atan2(siny_cosp, cosy_cosp);
}

}  // namespace localization

// 注册组件
PILOT_REGISTER_COMPONENT_WITH_CONFIG(localization::LocalizationComponent, localization)
