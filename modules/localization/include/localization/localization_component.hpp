#ifndef LOCALIZATION__LOCALIZATION_COMPONENT_HPP_
#define LOCALIZATION__LOCALIZATION_COMPONENT_HPP_

#include <rclcpp/rclcpp.hpp>
#include "common/component/module_base.hpp"
#include "common/component/component_register.hpp"

// 包含统一pilot_msgs包的proto头文件
#include <pilot_msgs/localization/imu_data.pb.h>

namespace localization
{

/**
 * @brief 定位组件 - 验证TypeAdapter简化架构
 * 
 * 当前版本：
 * - 使用纯Proto接口：get_latest("imu_data", proto_data)
 * - TypeAdapter自动处理ROS ↔ Proto转换
 * - 配置驱动的Key到Topic映射
 * - 无需DataManager，直接使用rclcpp::TypeAdapter
 */
class LocalizationComponent : public common::component::ModuleBase
{
public:
  explicit LocalizationComponent(const rclcpp::NodeOptions & options);
  ~LocalizationComponent() = default;

  // ModuleBase lifecycle methods
  bool init() override;
  bool reset() override;
  bool stop() override;
  void process() override;

private:  
  // 核心算法方法
  void process_imu_data(const pilot_msgs::localization::ImuData& imu_data);
  
  // 工具函数
  void quaternion_to_euler(const pilot_msgs::common::Quaternion& q, double& roll, double& pitch, double& yaw);

  // 传感器数据状态
  bool has_imu_data_ = false;
  bool is_initialized_ = false;
  
  // 运行统计
  uint64_t process_count_ = 0;
  uint64_t imu_process_count_ = 0;
};

}  // namespace localization

#endif  // LOCALIZATION__LOCALIZATION_COMPONENT_HPP_
