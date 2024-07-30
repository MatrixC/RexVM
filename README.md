# RexVM

[RexVM](https://github.com/MatrixC/RexVM) 一个基于解释器的初级Java虚拟机(JVM)，使用C++开发。

## 支持特性
* 异常(Exception)，包括异常的throw，catch，通过fillInStackTrace爬栈等
* 反射(Reflect)，包括类、字段、方法的Class、Field、Method对象获取以及反射执行函数等
* 流处理(Java Stream)，包括Lambda表达式，MethodHandle特性以及invokeDynamic字节码指令
* 动态创建Class，包括Proxy动态代理，ClassLoader#defineClass，Unsafe#defineClass等函数的支持
* 注解(Annotation)，包括类，字段，方法上的各类Annotation获取
* 多线程(Thread)，包括Thread#start，Thread#yield，Thread#sleep等线程函数以及Object#wait，Object#notify等同步函数
* 部分IO功能(IO)，FileInputStream，FileOutputStream
* 绝大部分Unsafe函数，包括Direct Memory、CompareAndSwap相关函数，支持Atomic、ConcurrentHashMap等concurrent组件

## 计划支持的特性
* Trace GC
* 加强文件、网络等IO能力

## 已测试的操作系统及编译环境
* MacOS(Apple silicon) + clang(18.1.6)
* Ubuntu(amd64) + clang(17.0.6) or gcc(11.4.0)
* Ubuntu(aarch64) + clang(18.1.8) or gcc(13.2.0)

## 构建
需要C++编译器支持C++20

### 1. 克隆仓库
```bash
git clone https://github.com/MatrixC/RexVM.git
cd RexVM
```

### 2. 安装编译器及构建工具xmake
#### MacOS
```bash
# 安装clang
brew install llvm
# 安装xmake
brew install xmake
```

#### Ubuntu
```bash
# 安装clang
wget https://apt.llvm.org/llvm.sh
chmod u+x llvm.sh
sudo ./llvm.sh 18
# 安装xmake
bash <(wget https://raw.githubusercontent.com/tboox/xmake/master/scripts/get.sh -O -)
```

### 3. 构建配置
#### 切换debug/release模式
```bash
# debug模式
xmake f -m debug

# release模式
xmake f -m release
```

#### 切换优化级别
修改xmake.lua中的set_optimize
<br>
参考xmake手册 [xmake manual](https://xmake.io/mirror/manual/project_target.html)
| Value | Description | gcc/clang | msvc |
| ----- | ----------- | --------- | ---- |
| none  | disable optimization | -O0 | -Od |
| fast	| quick optimization | -O1 | default |
| faster	| faster optimization | -O2 | -O2 |
| fastest	| Optimization of the fastest running speed	| -O3 | -Ox -fp:fast |
| smallest	| Minimize code optimization | -Os | -O1 -GL |
| aggressive | over-optimization | -Ofast | -Ox  -fp:fast |

```lua
-- example
set_optimize("fastest")
```

### 4. 使用xmake构建
```bash
xmake
```

### 5. 生成CMake工程文件，通过make或IDE构建
```bash
xmake project -k cmake
```

## 运行RexVM

RexVM运行依赖JRE中的rt.jar文件，所以运行RexVM需要安装JRE8或者JDK8环境，如openjdk8。
<br>
安装完成后请配置环境变量JAVA_HOME，也可以配置环境变量CLASSPATH来指定class文件搜索路径。
<br>
```bash
# 执行CLASSPATH环境变量下的Main.class
xmake run rex Main
```

可以将RexVM的编译结果目录配置进PATH环境变量，配置完成后直接用rex执行
```bash
# 执行当前目录或者CLASSPATH环境变量下的Main.class
rex Main
```

## 第三方依赖
* [miniz](https://github.com/richgel999/miniz): zlib库，用于zip、jar包解压
* [fmtlib](https://github.com/fmtlib/fmt): 格式化输出库，用于格式化打印，代替std::cout，printf等

## 感谢
* [wind_jvm](https://github.com/wind2412/wind_jvm): 感谢wind_jvm项目，帮我了解了invokedynamic的原理以及很多native方法的实现
* [xmake](https://github.com/xmake-io/xmake): 感谢xmake提供了强大且简单易用的构建工具