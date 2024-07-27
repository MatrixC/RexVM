# RexVM

[RexVM](https://github.com/MatrixC/RexVM) 一个基于解释器的JVM虚拟机，使用C++开发。

## 已验证系统及编译器

- MacOS(Apple silicon) + clang(18.1.6)
- Ubuntu(amd64)
- Ubuntu(aarch64) + clang(18.1.8) + gcc(13.2.0)

## 构建

### 1. 克隆仓库

```bash
git clone https://github.com/MatrixC/RexVM.git
cd RexVM
```

### 2. 安装编译器及构建工具xmake

可以通过以下命令安装xmake：

```bash
# MacOS
# 安装clang
brew install llvm
# 安装xmake
brew install xmake

# Ubuntu
# 安装clang
wget https://apt.llvm.org/llvm.sh
chmod u+x llvm.sh
sudo ./llvm.sh 18

# 安装xmake
bash <(wget https://raw.githubusercontent.com/tboox/xmake/master/scripts/get.sh -O -)
```

### 3. 构建项目

```bash
xmake

# 或者生成CMake项目文件后通过CMake进行构建或用Clion等IDE打开CMake工程文件
xmake project -k cmake
```

### 4. 运行RexVM

```bash
# 1. 安装JRE8或者JDK8环境，RexVM依赖rt.jar等openjdk lib
# 2. 配置环境变量export JRE_HOME=$JAVA_HOME 确保JRE_HOME/lib/rt.jar存在
# 3. 将RexVM所在的编译目录配进环境变量PATH
# 4. 运行class文件
RexVM Main
```




