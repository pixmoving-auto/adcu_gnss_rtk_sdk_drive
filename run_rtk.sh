#!/bin/bash

# RTK差分数据测试脚本
echo "中移差分数据Linux SDK 测试脚本"
echo "======================================================================"
echo ""

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 检查是否有main_gpsd可执行文件
if [ ! -f "$SCRIPT_DIR/main_gpsd" ]; then
    echo "错误：未找到main_gpsd可执行文件，请先编译。"
    echo "使用colcon build编译整个工作空间：colcon build --packages-select adcu_gnss_rtk_sdk_drive"
    exit 1
fi

# 检查SDK库文件
if [ ! -f "$SCRIPT_DIR/libcmcc_sdk.so" ]; then
    echo "警告：未找到libcmcc_sdk.so库文件，程序可能无法正常运行。"
    echo "请确保库文件在脚本目录下。"
    echo ""
fi

# 设置LD_LIBRARY_PATH以找到本地库
export LD_LIBRARY_PATH="$SCRIPT_DIR:$LD_LIBRARY_PATH"

# 检查并重启GPSD服务
echo "检查GPSD服务状态..."
if sudo systemctl is-active --quiet gpsd; then
    echo "✓ GPSD服务已经在运行，跳过重启"
else
    echo "✗ GPSD服务未运行，正在重启..."
    sudo systemctl restart gpsd
    sleep 2
    if sudo systemctl is-active --quiet gpsd; then
        echo "✓ GPSD服务重启成功！"
    else
        echo "✗ GPSD服务重启失败！"
        echo "请手动检查GPSD服务状态：systemctl status gpsd"
    fi
fi
echo ""

# 显示使用说明
echo "使用说明："
echo "1. 基本用法： ./run_rtk.sh [串口设备] [波特率]"
echo "2. 示例： ./run_rtk.sh /dev/ttyTHS0 460800"
echo "3. 如果不指定串口，程序仍会运行但不会发送差分数据到GPS设备"
echo ""

# 检查参数
if [ $# -eq 0 ]; then
    echo "警告：未指定串口设备，差分数据将无法发送到GPS设备。"
    echo "正在启动程序..."
    echo ""
    $SCRIPT_DIR/main_gpsd
elif [ $# -eq 1 ]; then
    echo "正在启动程序，使用串口：$1，默认波特率：460800"
    echo ""
    $SCRIPT_DIR/main_gpsd $1
elif [ $# -eq 2 ]; then
    echo "正在启动程序，使用串口：$1，波特率：$2"
    echo ""
    $SCRIPT_DIR/main_gpsd $1 $2
else
    echo "错误：参数过多。"
    echo "使用方法： ./run_rtk.sh [串口设备] [波特率]"
    exit 1
fi