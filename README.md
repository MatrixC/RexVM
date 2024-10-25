# RexVM

一个Java虚拟机(JVM)实现，使用C++开发。

## 支持特性
* 异常(Exception)，包括异常的throw，catch，fillInStackTrace等
* 反射(Reflect)，包括类、字段、方法的Class、Field、Method对象获取以及反射执行函数等
* 流处理(Java Stream)，包括Lambda表达式，MethodHandle特性以及invokeDynamic字节码指令
* 动态创建Class，包括Proxy动态代理，ClassLoader#defineClass，Unsafe#defineClass等函数的支持
* 注解(Annotation)，包括类，字段，方法上的各类Annotation获取
* 多线程(Thread)，包括Thread#start，Thread#yield，Thread#sleep等线程函数以及Object#wait，Object#notify等同步函数
* 部分IO功能(IO)，FileInputStream，FileOutputStream
* 绝大部分Unsafe函数，包括Direct Memory、CompareAndSwap相关函数，支持Atomic、ConcurrentHashMap等concurrent类
* java.util.ServiceLoader
* Tracing垃圾回收(Tracing garbage collection)以及finalize机制(Since v1.1)
* JIT(Just In Time)，基于LLVM后端，JVM函数粒度编译，支持所有解释器指令和异常，可以与解释器混合执行(Since v1.3)

## 已验证的操作系统及编译环境
### 操作系统及CPU架构
* Mac OS(aarch64)
* Ubuntu(amd64, aarch64)
* Windows(amd64)
### 编译环境
* clang(17.0)
* gcc(11.4)
* msvc(v19.40)

## 构建
需要C++编译器支持C++20特性

### 1. 克隆仓库
```bash
git clone https://github.com/MatrixC/RexVM.git
cd RexVM
```

### 2. 安装编译器及构建工具xmake
#### MacOS
```bash
brew install xmake
```

#### Ubuntu
```bash
bash <(wget https://raw.githubusercontent.com/tboox/xmake/master/scripts/get.sh -O -)
```

#### Windows
```bash
Invoke-Expression (Invoke-Webrequest 'https://xmake.io/psget.text' -UseBasicParsing).Content
```

### 3. 构建配置
#### 切换debug/release模式
```bash
# debug模式
xmake f -m debug

# release模式
xmake f -m release

```
#### 开启JIT功能
开启JIT功能需要安装LLVM并配置环境变量 LLVM_PATH 和 LLVM_VERSION 并打开JIT编译选项
Windows环境暂不支持
```bash
# 配置环境变量
export LLVM_PATH="/opt/homebrew/Cellar/llvm/18.1.6"
export LLVM_VERSION="LLVM-18"

# 开启llvm-jit编译选项
xmake f --llvm-jit=y -m release
xmake
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

### 5. 或生成CMake工程文件，通过make或IDE构建
```bash
# 生成cmake工程 (clion)
xmake project -k cmake

# 生成Visual Studio工程
xmake project -k vsxmake

# 生成Xcode工程
xmake project -k xcode
```

## 运行RexVM

RexVM运行依赖JRE中的rt.jar文件，所以需要安装JRE8或者JDK8环境，如openjdk8。
<br>
安装完成后请配置环境变量JAVA_HOME，也可以配置环境变量CLASSPATH来指定class文件搜索路径。
<br>
```bash
# 执行CLASSPATH环境变量下的Main.class
cd RexVM
xmake run rex Main
```

可以将RexVM的编译结果目录配置进PATH环境变量，配置完成后直接用rex执行
```bash
# 执行当前目录或者CLASSPATH环境变量下的Main.class
rex Main
```

## 演示
### LambdaExample
```java
package rex.example;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

public class LambdaExample {

    public static void main(String[] args) {
        final List<String> wordList =
                Arrays.asList("apple", "banana", "avocado", "cherry", "blueberry", "apricot", "strawberry");

        final String result =
                wordList.stream()
                        .filter(item -> item.startsWith("a"))
                        .map(String::toUpperCase)
                        .map(item -> item + "(" + item.length() + ")")
                        .collect(Collectors.groupingBy(String::length, Collectors.joining(",")))
                        .entrySet()
                        .stream()
                        .map(item -> "Length" + item.getKey() + ": " + item.getValue())
                        .collect(Collectors.joining("\n"));

        System.out.println(result);
    }
}
```
![image](https://github.com/MatrixC/RexVM/blob/improve_mh/example/LambdaExample.gif)

### Fastjson2Example
```java
package rex.example;

//import ...

public class Fastjson2Example {

    @Getter
    @Setter
    @ToString
    static class Pojo {
        private String str;
        private boolean bool;
        private int integer;
        private double dob;
        private Long nullValue;
        private Map<String, Object> map;
    }

    public static void main(String[] args) {
        final Pojo pojo = new Pojo();
        pojo.setStr("Hello RexVM");
        pojo.setBool(true);
        pojo.setInteger(123456);
        pojo.setDob(Math.PI);
        final Map<String, Object> map = new HashMap<>();
        map.put("key1", "value1");
        map.put("key2", Arrays.asList(100, 200, 300));
        pojo.setMap(map);

        final String jsonString = JSON.toJSONString(pojo, JSONWriter.Feature.PrettyFormat);
        System.out.println("Serialize");
        System.out.println(jsonString);

        final Pojo fromJSON = JSON.parseObject(jsonString, Pojo.class);
        System.out.println();
        System.out.println("Deserialize");
        System.out.println(fromJSON.toString());
    }
}
```
![image](https://github.com/MatrixC/RexVM/blob/improve_mh/example/Fastjson2Example.gif)

### Junit5Example
```java
package rex.example;

//import ...

public class Junit5Example {

    @Test
    public void test1() {
        Assertions.assertEquals(1 + 2 > 3, "Error");
    }

    @Test
    public void test2() {
        int sum = 0;
        for (int i = 0; i <= 100; i++) {
            sum += i;
        }
        Assertions.assertEquals(sum , 5050);
    }

    public static void main(String[] args) {
        final LauncherDiscoveryRequest request =
                LauncherDiscoveryRequestBuilder
                        .request()
                        .selectors(DiscoverySelectors.selectClass(Junit5Example.class))
                        .build();

        final Launcher launcher = LauncherFactory.create();
        final SummaryGeneratingListener listener = new SummaryGeneratingListener();
        launcher.registerTestExecutionListeners(listener);
        launcher.execute(request);

        final TestExecutionSummary summary = listener.getSummary();
        summary.printTo(new PrintWriter(System.out));
    }
}
```
![image](https://github.com/MatrixC/RexVM/blob/improve_mh/example/Junit5Example.gif)

### ThreadExample
```java
package rex.example;

import java.util.Random;

public class ThreadExample {

    static final Random random = new Random();

    static void sleepRandom() {
        try {
            Thread.sleep(random.nextInt(50));
        } catch (Exception ignored) {
        }
    }

    public static void main(String[] args) throws Exception {
        final Thread t1 = new Thread(() -> {
            for (int i = 0; i < 20; i++) {
                System.out.println("Thread1 " + i);
                sleepRandom();
            }
        });
        final Thread t2 = new Thread(() -> {
            for (int i = 0; i < 20; i++) {
                System.out.println("Thread2 " + i);
                sleepRandom();
            }
        });
        t1.start();
        t2.start();
        t1.join();
        t2.join();

        System.out.println("End");
    }
}
```
![image](https://github.com/MatrixC/RexVM/blob/improve_mh/example/ThreadExample.gif)

### AtomicExample
```java
package rex.example;

import java.util.concurrent.atomic.AtomicInteger;

public class AtomicExample {

    static int normalInteger = 0;
    static AtomicInteger atomicInteger = new AtomicInteger(0);

    public static void main(String[] args) throws Exception {
        final Thread t1 = new Thread(() -> {
            for (int i = 0; i < 100000; i++) {
                normalInteger++;
                atomicInteger.incrementAndGet();
            }
        });
        final Thread t2 = new Thread(() -> {
            for (int i = 0; i < 100000; i++) {
                normalInteger++;
                atomicInteger.incrementAndGet();
            }
        });
        final Thread t3 = new Thread(() -> {
            for (int i = 0; i < 100000; i++) {
                normalInteger++;
                atomicInteger.incrementAndGet();
            }
        });

        t1.start();
        t2.start();
        t3.start();
        t1.join();
        t2.join();
        t3.join();
        System.out.println("Normal Integer: " + normalInteger);
        System.out.println("Atomic Integer: " + atomicInteger);
    }
}
```
![image](https://github.com/MatrixC/RexVM/blob/improve_mh/example/AtomicExample.gif)

## 第三方依赖
* [miniz](https://github.com/richgel999/miniz): zlib库，用于zip、jar包解压
* [fmtlib](https://github.com/fmtlib/fmt): 格式化输出库，用于格式化打印，代替std::cout，printf等
* [emhash](https://github.com/ktprime/emhash): 高性能hashmap实现
* [LLVM](https://github.com/llvm/llvm-project): 编译器基础设施项目

## 致谢
* [wind_jvm](https://github.com/wind2412/wind_jvm): 感谢wind_jvm项目帮我了解了invokedynamic的原理以及很多native方法的实现
* [xmake](https://github.com/xmake-io/xmake): 感谢xmake提供了强大且简单易用的构建工具
