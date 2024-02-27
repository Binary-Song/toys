# 模块

本文介绍 llama 中的模块的概念。

"模块" 是 llama 中统一管理的一组构建目标。根据模块的类型和文件结构不同，生成的目标也不尽相同。

例如， SHARED 类型（动态库）的模块包含对象库、动态库和单元测试等目标。这些目标之间的关系由 llama.cmake 内部处理。

模块的目录结构是固定的。开发者需要按照规定的结构编写模块（模块的完整性由工具自动检查）。维护固定目录结构的收益是：llama 的模块几乎不需要编写 CMake 脚本。一切都可以由工具自动生成。

下面将介绍这种目录结构。

## 目录结构

projects/llama/src 目录中的每个子目录都是一个模块。

```
project/
├─ llama/
│  ├─ src/
│  │  ├─ mymodule/
│  │  │  ├─ include/
│  │  │  │  ├─ mymodule/
│  │  │  │  │  ├─ mymodule.h
│  │  │  ├─ src/
│  │  │  │  ├─ xxx.cpp
│  │  │  ├─ test/
│  │  │  │  ├─ xxx.cpp
│  │  │  ├─ CMakeLists.txt
│  │  ├─ housekeep-modules.json
│  │  ├─ modules.cmake
```

模块中每个文件和目录的含义如下（省略projects/llama/src）：

|文件/目录| 含义 | 是否必须 | 是否自动生成 |
| ----- | ---- | --- | --- | 
|  mymodule 目录  |  模块的根目录，该目录的名字就是模块的名字，模块的命名由小写字母、短横线（-）和下划线（_）组成。 | - | - |
|  mymodule/include 目录   |  模块的公共包含目录。这里存放模块公开的 .h 文件。下游模块和当前模块可以 include 该目录下的头文件。 | 必须存在 | 否 | 
|  mymodule/src 目录   |  模块的源代码目录和私有包含目录。这里存放 .cpp 和模块私有的 .h 文件。仅当前模块可以 include 该目录下的头文件。 | 必须存在 | 否 |
|  mymodule/test 目录   |  模块的单元测试目录，里面存放单元测试的源代码。 | 必须存在 | 否 |
|  mymodule/CMakeLists.txt 文件 | 模块的 CMakeLists.txt ，下面会介绍如何编写。 | 必须存在 | 否 |
|  mymodule/sources.cmake 文件 | 自动生成的源代码列表。 | 必须存在 | 是 |
|  mymodule/housekeep.json 文件 | housekeep 的配置文件，见 housekeep 的说明 | 可选 | 否 |
|  mymodule/include/mymodule/mymodule.h 文件 |  模块的"方便头文件"，包含 mymodule/include 目录里其他所有头文件。下游模块可以直接 include 这个文件来引入模块的所有功能。这个文件可以自动生成，见 housekeep 的说明。 | 建议有 | 默认是，可以通过配置 housekeep.json 来调整 |

全局的文件说明如下（省略projects/llama/src）

|文件/目录| 含义 | 是否必须 | 是否自动生成 |
| ----- | ---- | --- | --- | 
|  modules.cmake 文件  | 自动生成的模块列表。 | - | 是 |

除了上述的必要文件外，src 和 test 目录里还得有至少一个 .cpp 文件。

## 自动维护工具 housekeep 

housekeep 是一个 python 脚本，模块中需要随时和文件树结构同步的源码都是由它生成的。housekeep 的脚本可以在 projects/llama/scripts/housekeep.py 中找到。

默认情况下，CMake 时会自动运行一次 housekeep 来更新（通过 CMake 缓存 LLAMA_AUTO_HOUSEKEEP 来控制）。也可以手动执行该脚本来更新：

```
python housekeep.py
```

housekeep 的行为通过 llama 目录下的 housekeep-modules.json 和模块目录下的 housekeep.json 控制。

## 

