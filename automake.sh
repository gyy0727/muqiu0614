#!/bin/bash

## 删除 lib、build 和 bin 目录下的所有文件
#echo "Deleting files in lib directory..."
#rm -rf lib/*
#
#echo "Deleting files in build directory..."
#rm -rf build/*
#
#echo "Deleting files in bin directory..."
##rm -rf bin/*
#
## 执行 cmake
#echo "Running cmake..."
#cmake -S . -B build
#
## 切换到 build 目录
#cd build
#
## 执行 make 构建项目
#echo "Building project..."
#make -j2
#
#echo "Build completed."
#echo "------------------------------------------------------------------"
#cd ..
#
#cd ./bin
#
## 获取当前目录下所有可执行文件的列表
#./Sylar
#
## 遍历可执行文件列表，并执行每个文件
#
#
## 退出 bin 目录
#cd ..
#!/bin/bash

# 设置默认的可执行文件名称
DEFAULT_EXECUTABLE="test"

# 检查是否传递了参数
if [ "$#" -eq 1 ]; then
    # 如果传递了一个参数，将其拼接到默认值后面
    EXECUTABLE="${DEFAULT_EXECUTABLE}_$1"
else
    # 如果没有传递参数，使用默认的可执行文件
    EXECUTABLE="Sylar_Default"
fi

# 删除 lib、build 和 bin 目录下的所有文件
echo "Deleting files in lib directory..."
rm -rf lib/*

echo "Deleting files in build directory..."
rm -rf build/*

echo "Deleting files in bin directory..."
#rm -rf bin/*

# 执行 cmake
echo "Running cmake..."
cmake -S . -B build

# 切换到 build 目录
cd build

# 执行 make 构建项目
echo "Building project..."
make -j2

echo "Build completed."
echo "------------------------------------------------------------------"
cd ..

# 切换到 bin 目录
cd bin

# 检查可执行文件是否存在
if [ ! -f "$EXECUTABLE" ]; then
     ./"$EXECUTABLE"

    #echo "Error: Executable file $EXECUTABLE does not exist."
    exit 1
fi

# 执行指定的可执行文件
echo "Running $EXECUTABLE..."
./"$EXECUTABLE"

# 退出 bin 目录
cd ..

