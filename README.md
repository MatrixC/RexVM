# RexVM

[RexVM](https://github.com/MatrixC/RexVM) 一个简易的JVM虚拟机，使用C++编写，并使用xmake作为构建管理工具。RexVM能够在MacOS及Ubuntu操作系统上运行，支持x86_64和arm64。

## 系统要求

- 支持的操作系统：MacOS, Ubuntu
- 支持的架构：x86_64, arm64
- llvm clang
- xmake

## 安装与使用

### 1. 克隆仓库
```bash
git clone https://github.com/MatrixC/RexVM.git
cd RexVM
```

### 2. 安装xmake
可以通过以下命令安装xmake：
```bash
# MacOS
brew install xmake

# Ubuntu
bash <(wget https://raw.githubusercontent.com/tboox/xmake/master/scripts/get.sh -O -)
```

### 3. 构建项目
```bash
xmake
```

### 4. 运行RexVM
```bash
xmake run
```

---

# RexVM

[RexVM](https://github.com/MatrixC/RexVM) is a simple JVM virtual machine written in C++ and uses xmake as the build management tool. RexVM runs on MacOS and Ubuntu operating systems, supporting both x86_64 and arm64 architectures.

## Features

- **Platform**: Can run on MacOS and Ubuntu systems.
- **Architecture Support**: Supports x86_64 and arm64 architectures.

## System Requirements

- Supported OS: MacOS, Ubuntu
- Supported Architectures: x86_64, arm64
- llvm clang
- xmake

## Installation and Usage

### 1. Clone the Repository

```bash
git clone https://github.com/MatrixC/RexVM.git
cd RexVM
```

### 2. Install xmake

You can install xmake with the following commands:
```bash
# MacOS
brew install xmake

# Ubuntu
bash <(wget https://raw.githubusercontent.com/tboox/xmake/master/scripts/get.sh -O -)
```

### 3. Build the Project
```bash
xmake
```

### 4. Run RexVM
```bash
xmake run
```
