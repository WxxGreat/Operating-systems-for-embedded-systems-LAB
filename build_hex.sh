#!/bin/bash

# =========================================
# Auto Build Script (Standalone)
# Usage:
#   ./build_hex.sh lab1_ex1         # Arduino HEX
#   ./build_hex.sh lab1_ex1 --viper # VIPER target
# =========================================

if [ $# -lt 1 ]; then
    echo "Usage: $0 <project_name> [--viper]"
    exit 1
fi

PROJECT=$1
USE_VIPER=false
if [ "$2" == "--viper" ]; then
    USE_VIPER=true
fi

# 脚本所在目录
SCRIPT_DIR="$(dirname "$(realpath "$0")")"

# Trampoline 根目录
TRAMPOLINE="$SCRIPT_DIR/trampoline"
GOIL="$TRAMPOLINE/goil"
GOIL_TEMPLATES="$GOIL/templates"

VIPER_PATH="$TRAMPOLINE/viper"

# VIPER 模板目录（这里指向实际模板，假设里面有 config/viper 子目录）
VIPER_TEMPLATES="$TRAMPOLINE/viper/templates"
export VIPER_PATH

# Course_LAB 根目录
BASE_DIR="$SCRIPT_DIR/Course_LAB"

# 自动解析 LAB 和 EX 编号
LAB_NUM=$(echo "$PROJECT" | sed -E 's/lab([0-9]+)_ex.*/\1/')
EX_NUM=$(echo "$PROJECT" | sed -E 's/lab[0-9]+_ex([0-9]+)/\1/')

PROJECT_DIR="$BASE_DIR/LAB${LAB_NUM}/ex${EX_NUM}"
OIL_FILE="$PROJECT_DIR/${PROJECT}.oil"

if [ ! -f "$OIL_FILE" ]; then
    echo "Error: OIL file not found: $OIL_FILE"
    exit 1
fi

# 确保 goil 可执行文件在 PATH
export PATH="$GOIL/makefile-unix:$PATH"

# 根据 target 选择模板路径和 goil target
if [ "$USE_VIPER" = true ]; then
    TARGET="posix/linux"
    TEMPLATES="$GOIL_TEMPLATES/"
    # 编译 VIPER core
    echo "Building VIPER core..."
    cd "$VIPER_PATH" || { echo "Cannot cd to $VIPER_PATH"; exit 1; }
    make
    if [ $? -ne 0 ]; then
        echo "VIPER make failed!"
        exit 1
    fi
    # 返回项目目录
    cd "$PROJECT_DIR" || exit 1
else
    TARGET="avr/arduino/uno"
    TEMPLATES="$GOIL_TEMPLATES"
fi

# 进入项目目录
cd "$PROJECT_DIR" || exit 1

echo "=============================="
echo "Project: $PROJECT"
echo "Target: $TARGET"
echo "Using templates: $TEMPLATES"
echo "Project dir: $(pwd)"
echo "=============================="

# Step 1: 运行 goil
goil --target="$TARGET" --templates="$TEMPLATES" "$OIL_FILE"
if [ $? -ne 0 ]; then
    echo "goil failed!"
    exit 1
fi

# Step 2: 运行 make.py
./make.py
if [ $? -ne 0 ]; then
    echo "make.py failed!"
    exit 1
fi

# Step 3: 查找 HEX 文件
HEX_FILE=$(find . -maxdepth 2 -name "*.hex" | head -n 1)

echo "========================================"
echo "Build completed successfully!"
if [ "$USE_VIPER" = true ]; then
    EX_SUFFIX=$(echo "$PROJECT" | sed -E 's/lab[0-9]+_(ex[0-9]+)/\1/')
    EXE_DIR="$PROJECT_DIR"
    EXE_FILE="${EX_SUFFIX}_exe"
    echo "Executable (VIPER) generated in $EXE_DIR"
    echo "You can run it with:"
    echo "========================================"
    echo "cd $EXE_DIR && ./$EXE_FILE"
else
    echo "HEX file: $PROJECT_DIR/$HEX_FILE"
fi
echo "========================================"
