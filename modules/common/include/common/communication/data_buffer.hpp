#ifndef COMMON__COMMUNICATION__DATA_BUFFER_HPP_
#define COMMON__COMMUNICATION__DATA_BUFFER_HPP_

#include <memory>
#include <mutex>
#include <chrono>

namespace common
{
namespace communication
{

/**
 * @brief 通用数据缓冲区模板
 * 
 * 功能：
 * - 线程安全存储任意类型数据（通常是Proto消息）
 * - TypeAdapter架构下：存储已转换的Proto数据
 * - 提供最新数据缓存，避免频繁数据查询
 * - 支持线程安全的读写操作
 */
template<typename T>
class DataBuffer {
 public:
  using DataType = T;
  
  DataBuffer() = default;
  
  /**
   * @brief 存储最新数据（通常是TypeAdapter转换后的Proto数据）
   */
  void update(const T& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    data_ = data;
    timestamp_ = std::chrono::steady_clock::now();
    has_data_ = true;
  }
  
  /**
   * @brief 获取最新数据（直接返回，无需转换）
   */
  bool get_latest(T& data) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!has_data_) return false;
    data = data_;
    return true;
  }
  
  /**
   * @brief 检查是否有数据
   */
  bool has_data() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return has_data_;
  }
  
  /**
   * @brief 清空缓冲区
   */
  void clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    has_data_ = false;
  }
  
  
 private:
  mutable std::mutex mutex_;
  T data_;  // 存储最新数据（TypeAdapter架构下通常是Proto消息）
  std::chrono::steady_clock::time_point timestamp_;
  bool has_data_{false};
};

}  // namespace communication
}  // namespace common

#endif  // COMMON__COMMUNICATION__DATA_BUFFER_HPP_
