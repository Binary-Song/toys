// #pragma once
// #include "codec.h"
// #include "foundation/enums.h"
// #include "foundation/exceptions.h"
// #include "foundation/path.h"
// #include <fstream>
// #include <string>
// #include <string_view>
// #include <vector>
// namespace llama
// {

// enum class Read

// /// 相对路径。
// /// 相对路径需要和绝对路径组合才能形成完整路径。只能通过绝对路径打开文件。
// class RelativePath
// {
//   public:
//     RelativePath(std::string_view path, ReadPathOption opt = ReadPathOption::None);

//     RelativePath(std::wstring_view path, ReadPathOption opt = ReadPathOption::None) : RelativePath{ToUtf8(path), opt}
//     {
//     }

//     RelativePath(std::u16string_view path, ReadPathOption opt = ReadPathOption::None) : RelativePath{ToUtf8(path), opt}
//     {
//     }

//     std::string ToString(WritePathOption opt = WritePathOption_NativeSep) const
//     {
//         std::vector<std::string const *> stack;
//         std::string result;
//         result.reserve(5 * m_components.size());
//         for (std::string const &component : m_components)
//         {
//             if (+(opt & WritePathOption::WindowsSep))
//             {
//                 result.append(component);
//             }
//         }
//     }

//   private:
//     std::vector<std::string> m_components;
// };

// /// 绝对路径
// class Path
// {
//   public:
//     Path(std::string_view path, ReadPathOption opt = ReadPathOption::None);
// };

// } // namespace llama