# 初始化

这个仓库有子模块，需要“递归克隆”。

```
git clone xxx --recursive
```

# 生成

## 必选项

下载安装LLVM、Ninja和CMake

LLVM: https://releases.llvm.org/

Ninja: https://github.com/ninja-build/ninja/releases

CMake: https://cmake.org/download/

确保 ninja.exe 和 cmake.exe 在 PATH 里。

在cmd里执行下面指令生成toys。

Windows:

Windows上可能要开启“开发者模式”来允许创建符号链接。开启方法：

设置 -> 更新和安全 -> 开发者选项 -> 开发人员模式

```cmd
set "CC=C:\Program Files\LLVM\bin\clang.exe"
set "CXX=C:\Program Files\LLVM\bin\clang++.exe"
cmake -GNinja -B build -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON 
cmake --build build
```

Linux:

Linux 也只需要 cmake、ninja和llvm。用你最爱的包管理器装，或者从官网下。cmake指令一样。

## 可选项

### 生成文档

下载 Doxygen : https://www.doxygen.nl/download.html

cmake -GNinja -B build -S . -DLLAMA_BUILD_DOCS=ON 

