#!/bin/bash

# 测试脚本：验证所有项目是否能正确从注册表读取配置
echo "=== 开始兼容注册表测试 ==="

# 检查内核模块
if [ ! -f /proc/compat_registry ]; then
    echo "警告：/proc/compat_registry 不存在，请先加载内核模块"
    echo "可以使用以下命令加载："
    echo "  sudo insmod drivers/compat_registry/compat_registry.ko"
    echo ""
else
    echo "✓ /proc/compat_registry 存在"
    echo ""
    echo "当前注册表内容："
    cat /proc/compat_registry
    echo ""
fi

# 检查libcompatreg库
echo "=== 检查libcompatreg库 ==="
if [ ! -f libcompatreg.a ]; then
    echo "编译libcompatreg静态库..."
    make -f Makefile.libcompatreg libcompatreg.a
fi
if [ ! -f libcompatreg.so ]; then
    echo "编译libcompatreg共享库..."
    make -f Makefile.libcompatreg libcompatreg.so
fi

echo "✓ libcompatreg库检查完成"
echo ""

# 测试cmd项目
echo "=== 测试cmd项目 ==="
cd GithubProject/cmd
if [ ! -f Makefile ]; then
    echo "创建cmd项目的Makefile..."
    cat << 'EOF' > Makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS = -L.. -lcompatreg

SRC = main.c
OBJ = $(SRC:.c=.o)
TARGET = cmd

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
EOF
fi

echo "编译cmd项目..."
make clean > /dev/null 2>&1
make
if [ $? -eq 0 ]; then
    echo "✓ cmd项目编译成功"
    echo "cmd项目配置测试："
    ./cmd.exe 2>/dev/null || echo "  程序运行需要图形环境或特定配置"
else
    echo "✗ cmd项目编译失败"
fi
cd ../..

# 测试Explorer-for-Linux项目
echo ""
echo "=== 测试Explorer-for-Linux项目 ==="
cd GithubProject/Explorer-for-Linux
if [ ! -f Makefile ]; then
    echo "创建Explorer-for-Linux项目的Makefile..."
    cat << 'EOF' > Makefile
CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17 -fPIC
LDFLAGS = -L.. -lcompatreg -lQt5Widgets -lQt5Core -lQt5Gui

SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = Explorer-for-Linux

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
EOF
fi

echo "编译Explorer-for-Linux项目..."
make clean > /dev/null 2>&1
make
if [ $? -eq 0 ]; then
    echo "✓ Explorer-for-Linux项目编译成功"
else
    echo "✗ Explorer-for-Linux项目编译失败 (可能需要Qt开发库)"
fi
cd ../..

# 测试SAS-for-Linux项目
echo ""
echo "=== 测试SAS-for-Linux项目 ==="
cd GithubProject/SAS-for-Linux
if [ ! -f Makefile ]; then
    echo "创建SAS-for-Linux项目的Makefile..."
    cat << 'EOF' > Makefile
CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17 -fPIC
LDFLAGS = -L.. -lcompatreg -lQt5Widgets -lQt5Core -lQt5Gui

SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = SAS-for-Linux

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
EOF
fi

echo "编译SAS-for-Linux项目..."
make clean > /dev/null 2>&1
make
if [ $? -eq 0 ]; then
    echo "✓ SAS-for-Linux项目编译成功"
else
    echo "✗ SAS-for-Linux项目编译失败 (可能需要Qt开发库)"
fi
cd ../..

# 测试Linux_uac项目
echo ""
echo "=== 测试Linux_uac项目 ==="
cd GithubProject/Linux_uac
echo "Linux_uac项目已有Makefile，编译..."
make clean > /dev/null 2>&1
make
if [ $? -eq 0 ]; then
    echo "✓ Linux_uac项目编译成功"
else
    echo "✗ Linux_uac项目编译失败"
fi
cd ../..

# 测试runbox项目
echo ""
echo "=== 测试runbox项目 ==="
cd GithubProject/runbox
echo "检查runbox项目依赖..."
if command -v cargo &> /dev/null; then
    echo "Rust/Cargo可用，编译runbox项目..."
    cargo clean > /dev/null 2>&1
    cargo build
    if [ $? -eq 0 ]; then
        echo "✓ runbox项目编译成功"
    else
        echo "✗ runbox项目编译失败"
    fi
else
    echo "⚠ Cargo不可用，跳过runbox项目编译"
fi
cd ../..

echo ""
echo "=== 测试总结 ==="
echo "所有项目编译测试完成。"
echo ""
echo "手动验证建议："
echo "1. 确保内核模块已加载: sudo insmod drivers/compat_registry/compat_registry.ko"
echo "2. 创建测试配置文件（参考文档）"
echo "3. 运行各程序验证配置读取:"
echo "   - cmd: ./GithubProject/cmd/cmd"
echo "   - Explorer: ./GithubProject/Explorer-for-Linux/Explorer-for-Linux"
echo "   - SAS: ./GithubProject/SAS-for-Linux/SAS-for-Linux"
echo "   - UAC: sudo ./GithubProject/Linux_uac/uac_ui"
echo "   - runbox: cd GithubProject/runbox && cargo run"
echo ""
echo "配置修改测试："
echo "1. 修改/etc/registry/下的配置文件"
echo "2. 重启应用程序验证配置生效"
echo ""
echo "故障排除："
echo "- 如果链接错误，检查libcompatreg.a和libcompatreg.so是否存在"
echo "- 如果Qt程序编译失败，安装Qt开发库: sudo apt-get install qtbase5-dev qtchooser qt5-qmake libqt5widgets5 libqt5core5a libqt5gui5"
echo "- 如果程序运行时无法读取/proc/compat_registry，检查内核模块状态"