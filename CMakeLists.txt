cmake_minimum_required(VERSION 3.8)
project(adcu_gnss_rtk_sdk_drive)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 找到必要的系统库
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPS REQUIRED libgps)

# 添加可执行文件
add_executable(main_gpsd main_gpsd.c)

# 包含头文件目录
target_include_directories(main_gpsd PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
  ${GPS_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(main_gpsd
  ${GPS_LIBRARIES}
  pthread
  m
  ${CMAKE_CURRENT_SOURCE_DIR}/libcmcc_sdk.so
)

# 设置可执行文件的输出目录
install(TARGETS main_gpsd
  DESTINATION lib/${PROJECT_NAME}
)

# 安装库文件
install(FILES libcmcc_sdk.so
  DESTINATION lib/${PROJECT_NAME}
)

# 安装头文件
install(FILES libapi.h
  DESTINATION include/${PROJECT_NAME}
)

# 安装运行脚本
install(PROGRAMS run_rtk.sh
  DESTINATION lib/${PROJECT_NAME}
)

find_package(ament_cmake REQUIRED)
ament_package()