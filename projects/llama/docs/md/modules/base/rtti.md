# RTTI {#rtti}


RTTI 是 C++ 的语言特性，全称运行时类型信息。RTTI 的主要作用是为动态类型转换（ `dynamic_cast` ）提供必要信息。没有 RTTI ，将无法判断对象的实际类型（又称，“动态类型”），也就无法进行安全的类型转换。

RTTI 一直以来就是 C++ 中争议比较大的一个特性，很多观点认为它违反了 C++ “零开销抽象”的原则。
因为它并不是用多少功能就付出多少成本。一般开启 RTTI 后，所有带有虚函数的类及其子类
都会产生RTTI信息，不管你有没有对它使用 dynamic_cast 。对于较大的继承树，RTTI 可以占用不少空间。
因此，不少开源项目不使用 RTTI ，或者采用自己的机制来实现RTTI的功能，比较著名的有 [LLVM-Style RTTI](https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html) 。

llama 没有直接采用 LLVM 风格的原因主要包括2点：

1. LLVM 风格的 RTTI 配置起来很复杂，需要定义多个枚举值和函数，容易出错。此外，共享的类型枚举导致基类和子类互相依赖：新增一个派生类时，基类必须重新编译。
2. LLVM 风格的 RTTI 不支持虚继承、菱形继承。而 llama 大量使用接口类，
虚继承和菱形继承不可避免。

简单来说，LLVM 偏静态的 RTTI 机制适合编译器对高性能的要求，但并不适合
 GUI 应用程序对运行时灵活性的要求。所以 llama 选择开发一套更动态、更灵活的
RTTI 机制。

llama 的 RTTI 有如下特点：

1. RTTI 信息可选（you only pay for what you use）。
2. 支持多继承、虚继承。多继承和虚继承在 llama 里主要用于继承接口。
3. 使用简单。只需要一个宏调用即可为类添加 RTTI 。

## 为类型添加 RTTI 

下面将介绍如何为类型添加 RTTI 信息。

首先，一个具有 RTTI 的类型必须直接或间接派生自 [IRtti](@ref llama::IRtti) 接口。
下方示例定义了一个直接派生自 [IRtti](@ref llama::IRtti) 的类 Foo。

```cpp
class Foo : public virtual llama::IRtti
{};
```

为了实现 IRtti 接口，我们需要调用 LLAMA_RTTI 宏。

```cpp
class Foo : public virtual llama::IRtti
{
	LLAMA_RTTI(::Foo)
};
```

LLAMA_RTTI 宏的第一个参数是类型的 **完整名称** ，即，必须以 `::` 开头指定完整的名字。
在 LLAMA_RTTI 内部，它会转为字符串，作为 llama 内部区分这个类型的唯一标识。

接下来，我们为 Foo 的子类 Bar 添加 RTTI 。

```cpp
class Bar : public Foo
{
	LLAMA_RTTI(::Bar, ::Foo)
};
```

`LLAMA_RTTI(::Bar, ::Foo)` 表示 Bar 派生自 Foo 。
如果有多个基类，还可以这样写：`LLAMA_RTTI(::Bar, ::Foo1, ::Foo2)`，表示
Bar 派生自 Foo1 和 Foo2 。

用 LLAMA_RTTI 注册的继承关系必须是 public 继承。

用 [Cast](@ref llama::IRtti::Cast) 方法来进行动态类型转换。

```cpp
Foo* foo = new Bar();
p<Bar> bar = foo->Cast<Bar>();
```

Cast 方法会从对象的实际类型（“动态类型”）沿着继承树向上搜索，直至找到 Bar 类型。
如果找不到，则抛出异常。所以 `LLAMA_RTTI` 本质上就是登记 Cast 可以搜索到的路径。

最后，补充一个注意点，如上面所示，如果是 IRtti 的直接派生类， **不要在LLAMA_RTTI里指定IRtti这个基类** 。这是条特殊规则，没啥理由。除此之外，几乎没有别的规则。你可以在 LLAMA_RTTI 里指定间接基类、甚至可以指定同一个基类多次，因为你知道这玩意本质就是在图里加了一条边而已。虽然但是，不建议这么干。另外，不要试图欺骗 LLAMA_RTTI ，否则你会得到一大坨编译错误。

@sa llama::IRtti llama::RttiContext [RTTI](#rtti)


