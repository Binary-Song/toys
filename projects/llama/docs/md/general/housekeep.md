# housekeep 参考 {#housekeep}

housekeep 有两种配置文件： housekeep.json 和 housekeep-modules.json。
 housekeep.json 负责配置每个模块要包含哪些源文件。housekeep-modules.json 负责配置要包含哪些模块。

## 通用规则

名为 include 和 exclude 的字段，若无特殊说明，都适用如下规则：

include 和 exclude 都是字符串数组，每个字符串是一个匹配规则。
当一个文件的相对路径（相对于 housekeep.json 或 housekeep-modules.json 所在目录）
匹配 include 中的任意规则但不匹配 exclude 的任何规则时，才能出现最终的结果集里。

默认 include 所有文件。

匹配规则用 python 的 glob 函数实现，支持通配符 `*`（匹配≥0个字符）和 `**` （匹配≥0个目录名）。
例如 **/*.txt 将递归匹配目录内所有.txt文件。

## housekeep.json 说明

下面是一个 housekeep.json 文件的实例。

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
- `include` ：包含在sources.cmake内的文件。
- `exclude` ：排除在sources.cmake外的文件。这些文件不会生成。
- `umbrella_header.generate` ：布尔值，表示是否生成方便头文件。
- `umbrella_header.include` ：包含在方便头文件中的文件。
- `umbrella_header.exclude` ：排除在方便头文件之外的文件。

## housekeep-modules.json 说明

- `include` ：包含在 modules.cmake 内的模块。
- `exclude` ：排除在 modules.cmake 外的模块。这些模块不会生成，也不会参与 housekeep。
