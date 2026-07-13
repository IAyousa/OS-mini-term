# ====================================================================
#  操作系统模拟器 Makefile (适用于 Windows MinGW 环境)
# ====================================================================

# 编译器与编译选项
CXX      := D:/app/Dev-Cpp/MinGW64/bin/g++.exe
CXXFLAGS := -std=c++11 -g -fpermissive -Iinclude

# 链接库 (EasyX 图形库)
LIBS     := -leasyx

# 目标输出可执行文件
TARGET   := src/main.exe

# 自动收集子目录下的所有 .cpp 源文件
SRCS     := src/main.cpp \
            $(wildcard src/device/*.cpp) \
            $(wildcard src/file/*.cpp) \
            $(wildcard src/memory/*.cpp) \
            $(wildcard src/process/*.cpp)

# 默认构建目标
all: $(TARGET)

$(TARGET): $(SRCS)
	@echo [Makefile] 正在编译所有源文件
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)
	@echo [Makefile] 构建成功！输出目标: $(TARGET)

# 清理生成文件
clean:
	del /Q /F src\\main.exe 2>nul || rm -f src/main.exe
