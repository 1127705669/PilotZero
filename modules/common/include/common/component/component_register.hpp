#ifndef COMMON__COMPONENT__COMPONENT_REGISTER_HPP_
#define COMMON__COMPONENT__COMPONENT_REGISTER_HPP_

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_components/register_node_macro.hpp>

namespace common
{
namespace component
{

/**
 * PilotZero component registration macro
 */
#define PILOT_REGISTER_COMPONENT(CLASS_NAME) \
  namespace { \
    struct ComponentInitializer_##CLASS_NAME { \
      ComponentInitializer_##CLASS_NAME() { \
        RCLCPP_INFO(rclcpp::get_logger("component_register"), \
                   "Registering PilotZero component: " #CLASS_NAME); \
      } \
    }; \
    static ComponentInitializer_##CLASS_NAME init_##CLASS_NAME; \
  } \
  RCLCPP_COMPONENTS_REGISTER_NODE(CLASS_NAME)

/**
 * Component registration macro with configuration key
 */
#define PILOT_REGISTER_COMPONENT_WITH_CONFIG(CLASS_NAME, CONFIG_KEY) \
  namespace { \
    struct ComponentInitializer { \
      ComponentInitializer() { \
        RCLCPP_INFO(rclcpp::get_logger("component_register"), \
                   "Registering PilotZero component: " #CLASS_NAME " (config: " #CONFIG_KEY ")"); \
      } \
    }; \
    static ComponentInitializer component_init; \
  } \
  RCLCPP_COMPONENTS_REGISTER_NODE(CLASS_NAME)

}  // namespace component
}  // namespace common

#endif  // COMMON__COMPONENT__COMPONENT_REGISTER_HPP_
