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

如果是Windows，请将 ninja.exe 和 cmake.exe 的目录添加进 PATH 环境变量里。

## 生成 

用 CMakePresets.json 一键生成+测试。（需要CMake >= 3.25）

Windows:

```
cmake --workflow --preset win-clang-ci
```

Linux:

```
cmake --workflow --preset linux-clang-ci
```

## 可选项

### 生成文档

下载 Doxygen : https://www.doxygen.nl/download.html

```
cmake -GNinja -B build -S . -DLLAMA_BUILD_DOCS=ON
cmake --build --target docs
```

