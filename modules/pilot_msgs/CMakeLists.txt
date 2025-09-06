cmake_minimum_required(VERSION 3.8)
project(pilot_msgs)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(rosidl_default_runtime REQUIRED)
find_package(Protobuf REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# ========== Protocol Buffers 编译 ==========

# 收集所有proto文件
file(GLOB_RECURSE PROTO_FILES "proto/*.proto")

if(PROTO_FILES)
  # 创建中间目录结构，将proto文件复制到pilot_msgs子目录
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/proto_staging/${PROJECT_NAME})
  
  foreach(PROTO_FILE ${PROTO_FILES})
    # 获取相对路径
    file(RELATIVE_PATH REL_PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/proto ${PROTO_FILE})
    get_filename_component(PROTO_DIR ${REL_PROTO_PATH} DIRECTORY)
    get_filename_component(PROTO_NAME ${REL_PROTO_PATH} NAME)
    
    # 创建staging目录结构
    if(PROTO_DIR)
      file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/proto_staging/${PROJECT_NAME}/${PROTO_DIR})
    endif()
    
    # 复制proto文件到staging区域
    configure_file(
      ${PROTO_FILE}
      ${CMAKE_CURRENT_BINARY_DIR}/proto_staging/${PROJECT_NAME}/${REL_PROTO_PATH}
      COPYONLY
    )
  endforeach()
  
  # 一次性编译所有proto文件
  execute_process(
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
      --proto_path=${CMAKE_CURRENT_BINARY_DIR}/proto_staging
      --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}/proto_staging/${PROJECT_NAME}/common/geometry.proto
      ${CMAKE_CURRENT_BINARY_DIR}/proto_staging/${PROJECT_NAME}/common/header.proto
      ${CMAKE_CURRENT_BINARY_DIR}/proto_staging/${PROJECT_NAME}/localization/imu_data.proto
      ${CMAKE_CURRENT_BINARY_DIR}/proto_staging/${PROJECT_NAME}/localization/gps_data.proto
      ${CMAKE_CURRENT_BINARY_DIR}/proto_staging/${PROJECT_NAME}/control/control_command.proto
    RESULT_VARIABLE _protobuf_result
  )
  
  if(_protobuf_result)
    message(FATAL_ERROR "Error generating protobuf files")
  endif()
  
  # 收集生成的文件
  file(GLOB_RECURSE PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/*.pb.cc")
  file(GLOB_RECURSE PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/*.pb.h")
  
  # 创建protobuf库
  add_library(${PROJECT_NAME}_proto SHARED ${PROTO_SRCS})
  target_link_libraries(${PROJECT_NAME}_proto ${Protobuf_LIBRARIES})
  target_include_directories(${PROJECT_NAME}_proto PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
  )
  
  # 安装库和头文件
  install(TARGETS ${PROJECT_NAME}_proto
    EXPORT export_${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
  )
  
  # 安装头文件，保持目录结构  
  foreach(PROTO_HDR ${PROTO_HDRS})
    # 获取相对于build/pilot_msgs目录的路径
    file(RELATIVE_PATH REL_HDR_PATH ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} ${PROTO_HDR})
    get_filename_component(HDR_DIR ${REL_HDR_PATH} DIRECTORY)
    
    if(HDR_DIR)
      # 有子目录，按原目录结构安装
      install(FILES ${PROTO_HDR}
        DESTINATION include/${PROJECT_NAME}/${HDR_DIR}
      )
    else()
      # 根目录文件，直接安装到package目录
      install(FILES ${PROTO_HDR}
        DESTINATION include/${PROJECT_NAME}
      )
    endif()
  endforeach()
endif()

# Export
ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(Protobuf)
ament_export_include_directories(include)

ament_package()
