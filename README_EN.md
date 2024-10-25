# Translated by GPT-4o.

# RexVM
A Java Virtual Machine (JVM) implementation developed using C++.

## Features Supported
* Exception handling, including throw, catch, and fillInStackTrace.
* Reflection, supporting Class, Field, and Method objects, along with reflective method execution.
* Java Stream processing, including Lambda expressions and MethodHandle features with the invokeDynamic bytecode instruction.
* Dynamic class creation, including Proxy dynamic proxy, ClassLoader#defineClass, Unsafe#defineClass, etc.
* Annotation retrieval on classes, fields, and methods.
* Multithreading support, including Thread#start, Thread#yield, Thread#sleep, and synchronization functions like Object#wait, Object#notify.
* Partial IO functionalities, including FileInputStream, FileOutputStream.
* Most Unsafe functions, including Direct Memory and CompareAndSwap related functions, supporting concurrent classes like Atomic, ConcurrentHashMap.
* java.util.ServiceLoader.
* Tracing garbage collection and finalize mechanism (Since v1.1).
* Just In Time (JIT) compilation, based on LLVM backend, with JVM function-level compilation, supporting all interpreter instructions and exceptions, and mixed execution with the interpreter (Since v1.3).

## Verified Operating Systems and Build Environments
### Operating Systems and CPU Architectures
* Mac OS (aarch64)
* Ubuntu (amd64, aarch64)
* Windows (amd64)
### Build Environments
* clang (17.0)
* gcc (11.4)
* msvc (v19.40)

## Build
Requires a C++ compiler supporting C++20 features.

### 1. Clone the Repository
```bash
git clone https://github.com/MatrixC/RexVM.git
cd RexVM
```

### 2. Install Compiler and Build Tool xmake
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

### 3. Build Configuration
#### Switch Between Debug/Release Modes
```bash
# Debug mode
xmake f -m debug

# Release Mode
xmake f -m release
```

#### Enable JIT Feature
To enable JIT, install LLVM and configure the LLVM_PATH and LLVM_VERSION environment variables, and enable the JIT compilation option. 
JIT is not supported on Windows.
```bash
# Configure environment variables
export LLVM_PATH="/opt/homebrew/Cellar/llvm/18.1.6"
export LLVM_VERSION="LLVM-18"

# Enable llvm-jit compilation option
xmake f --llvm-jit=y -m release
xmake
```

#### Change Optimization Level
Modify set_optimize in xmake.lua.
<br>
Refer to the [xmake manual](https://xmake.io/mirror/manual/project_target.html)
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

### 4. Build with xmake
```bash
xmake
```

### 5. Or Generate CMake Project File and Build with Make or an IDE
```bash
# Generate cmake project (clion)
xmake project -k cmake

# Generate Visual Studio project
xmake project -k vsxmake

# Generate Xcode project
xmake project -k xcode
```

## Running RexVM

RexVM requires the rt.jar file from the JRE, so you need to have JRE8 or JDK8, like openjdk8, installed.
<br>
After installation, configure the JAVA_HOME environment variable. You can also set the CLASSPATH environment variable to specify the class file search path.
<br>
```bash
# Run Main.class from the CLASSPATH environment variable
cd RexVM
xmake run rex Main
```

You can add the directory of RexVM's build results to the PATH environment variable. After configuration, use rex directly to execute.
```bash
# Run Main.class from the current directory or CLASSPATH
rex Main
```

## Demos
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

## Third-party Dependencies
* [miniz](https://github.com/richgel999/miniz): zlib library for zip and jar extraction.
* [fmtlib](https://github.com/fmtlib/fmt): Formatting library for formatted printing, replacing std::cout, printf, etc.
* [emhash](https://github.com/ktprime/emhash): High-performance hashmap implementation.
* [LLVM](https://github.com/llvm/llvm-project): Compiler infrastructure project.

## Acknowledgments
* [wind_jvm](https://github.com/wind2412/wind_jvm): Thanks to wind_jvm for helping me understand the principles of invokedynamic and many native method implementations.
* [xmake](https://github.com/xmake-io/xmake): Thanks to xmake for providing a powerful and easy-to-use build tool.