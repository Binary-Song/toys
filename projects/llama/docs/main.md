# 项目概览

llama 项目是由“模块”构成的。

模块的目录结构是固定的，目录结构有自己的含义，会对 CMake 生成的 target 产生影响。 

## 模块的目录结构

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
│  │  │  ├─ housekeep.json
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
|  mymodule/housekeep.json 文件 | 模块级 housekeep 配置文件，见 housekeep 的说明 | 可选 | 否 |
|  mymodule/include/mymodule/mymodule.h 文件 |  模块的"方便头文件"，包含 mymodule/include 目录里其他所有头文件。下游模块可以直接 include 这个文件来引入模块的所有功能。这个文件可以自动生成，见 housekeep 的说明。 | 建议有 | 默认是，可以通过配置 housekeep.json 来调整 |
|  modules.cmake 文件  | 自动生成的模块列表。 | - | 是 |
|  housekeep-modules.json 文件  | 全局 housekeep 配置文件，见 housekeep 的说明 | 可选 | 否 |

除了上述的必要文件外，src 和 test 目录里还得有至少一个 .cpp 文件。

## 自动维护工具 housekeep 

housekeep 是一个 python 脚本 （projects/llama/scripts/housekeep.py），负责生成一些简单的文件列表。

housekeep 生成两种 CMake 文件：

- sources.cmake: 决定哪些文件会参加编译。
- modules.cmake: 决定哪些模块会参加编译。

可以分别通过 housekeep.json 和 housekeep-modules.json 的 include 和 exclude 字段控制哪些文件或模块要包含、哪些要排除。

默认情况下，CMake 运行时会自动运行一次 housekeep。可以在 CMake 时增加 -DLLAMA_AUTO_HOUSEKEEP=OFF 来关闭这个行为。

housekeep 还会生成“方便头文件”，这个文件和模块同名，包含 include 目录下所有的文件。可以通过 housekeep.json 中 umbrella_header 下的选项来控制生成它的行为。

### housekeep.json 说明

```
{
	"umbrella_header": {
		"generate": false,
		"include" : ["**/*foo*.cpp"],
		"exclude" : ["foobar.cpp"]
	},
	"include": ["**/*.h", "**/*.cpp"]
	"exclude": ["foobar.cpp"]
}
```

### 字段说明


#### include 和 exclude 

名为 include 和 exclude 的字段，若无特殊说明，都适用如下规则：

include 和 exclude 都是字符串数组，每个字符串是一个匹配规则。
当一个文件的相对路径（相对于 housekeep.json 或 housekeep-modules.json 所在目录）
匹配 include 中的任意规则但不匹配 exclude 的任何规则时，才能出现最终的结果集里。

匹配规则用 python 的 glob 函数实现，支持通配符 `*`（匹配≥0个字符）和 `**` （匹配≥0个目录名）。
例如 **/*.txt 将递归匹配目录内所有.txt文件。

#### include

包含在sources.cmake内的文件。见

#### exclude

排除在sources.cmake外的文件。

#### umbrella_header.generate 

布尔值，表示是否生成方便头文件。

#### umbrella_header.include

包含在方便头文件中的文件。同上。

#### umbrella_header.exclude

排除在方便头文件之外的文件。同上。

### housekeep-modules.json 说明



