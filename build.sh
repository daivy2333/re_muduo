#!/bin/bash

# 编译脚本
# 用于自动化构建re_muduo项目

# 设置颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 打印信息函数
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# 检查cmake是否安装
if ! command -v cmake &> /dev/null; then
    print_error "cmake未安装，请先安装cmake"
    exit 1
fi

# 检查make是否安装
if ! command -v make &> /dev/null; then
    print_error "make未安装，请先安装make"
    exit 1
fi

# 获取脚本所在目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# 创建build目录
if [ -d "$SCRIPT_DIR/build" ]; then
    print_warning "build目录已存在，将清理并重新创建"
    rm -rf "$SCRIPT_DIR/build"
fi

print_info "创建build目录..."
mkdir -p "$SCRIPT_DIR/build"
cd "$SCRIPT_DIR/build"

# 运行cmake
print_info "运行cmake配置..."
if ! cmake ..; then
    print_error "cmake配置失败"
    exit 1
fi

# 运行make
print_info "开始编译..."
if ! make; then
    print_error "编译失败"
    exit 1
fi

print_info "编译成功完成！"
