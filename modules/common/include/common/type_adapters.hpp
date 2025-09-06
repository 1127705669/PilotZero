#ifndef COMMON__TYPE_ADAPTERS_HPP_
#define COMMON__TYPE_ADAPTERS_HPP_

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/nav_sat_fix.hpp>
#include <carla_msgs/msg/carla_ego_vehicle_control.hpp>
#include <pilot_msgs/localization/imu_data.pb.h>
#include <pilot_msgs/control/control_command.pb.h>
// #include <pilot_msgs/localization/gps_data.pb.h>  // GPS proto头文件（待启用）

/**
 * @file type_adapters.hpp
 * @brief Apollo宏 + ROS 2官方TypeAdapter + 分离转换实现
 * 
 * 设计理念：
 * - 宏定义TypeAdapter结构（只有声明）
 * - 具体胶水代码实现在common/src/{algorithm}/目录
 * - 按算法模块分组织转换代码
 * - 官方TypeAdapter机制
 */

// ========== 统一注册系统 ==========

/**
 * @brief 类型映射Traits - 通用基类
 */
template<typename ProtoType>
struct TypeMapping {
  static_assert(sizeof(ProtoType) == 0, "TypeMapping must be specialized for ProtoType");
};

/**
 * @brief 统一注册TypeAdapter的宏 - 一次性生成所有必要代码
 * 
 * @param KEY 数据key (如 imu, gps)
 * @param PROTO_TYPE Proto消息类型
 * @param ROS_TYPE ROS消息类型
 */
#define REGISTER_TYPE_ADAPTER(KEY, PROTO_TYPE, ROS_TYPE)                  \
/* 1. TypeMapping特化 */                                                 \
template<>                                                                \
struct TypeMapping<PROTO_TYPE> {                                         \
  using RosType = ROS_TYPE;                                              \
  using TypeAdapterType = rclcpp::TypeAdapter<PROTO_TYPE, ROS_TYPE>;    \
  static constexpr const char* converter_key = #KEY;                    \
};                                                                       \
                                                                         \
/* 2. 转换函数声明 */                                                     \
namespace common { namespace converters {                               \
  void convert_##KEY##_proto_to_ros(const PROTO_TYPE& src, ROS_TYPE& dst); \
  void convert_##KEY##_ros_to_proto(const ROS_TYPE& src, PROTO_TYPE& dst); \
}}                                                                       \
                                                                         \
/* 3. TypeAdapter特化 */                                                 \
template<>                                                               \
struct rclcpp::TypeAdapter<PROTO_TYPE, ROS_TYPE>                        \
{                                                                        \
  using is_specialized = std::true_type;                                 \
  using custom_type = PROTO_TYPE;                                        \
  using ros_message_type = ROS_TYPE;                                     \
                                                                         \
  static void convert_to_ros_message(const custom_type & source,         \
                                     ros_message_type & destination)     \
  {                                                                      \
    common::converters::convert_##KEY##_proto_to_ros(source, destination); \
  }                                                                      \
                                                                         \
  static void convert_to_custom(const ros_message_type & source,         \
                                custom_type & destination)               \
  {                                                                      \
    common::converters::convert_##KEY##_ros_to_proto(source, destination); \
  }                                                                      \
};

/**
 * @brief 编译期类型安全检查
 */
#define ENSURE_ADAPTER(PROTO_TYPE, ROS_TYPE)                              \
  static_assert(rclcpp::TypeAdapter<PROTO_TYPE, ROS_TYPE>::is_specialized::value, \
                "TypeAdapter<" #PROTO_TYPE ", " #ROS_TYPE "> must be specialized")

// ========== 自动注册所有TypeAdapter ==========

// 定义展开宏，将.def文件中的ADAPTER展开为REGISTER_TYPE_ADAPTER
#define ADAPTER(KEY, PROTO_TYPE, ROS_TYPE) \
  REGISTER_TYPE_ADAPTER(KEY, PROTO_TYPE, ROS_TYPE)

// 包含注册列表，自动展开所有TypeAdapter
#include "adapter_registry.def"

// 清理宏
#undef ADAPTER

#endif  // COMMON__TYPE_ADAPTERS_HPP_