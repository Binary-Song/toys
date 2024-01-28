#pragma once
#include "codex.h"
#include "foundation/enums.h"
#include "foundation/exceptions.h"
#include "foundation/path.h"
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
namespace llama
{

/// 相对路径。
/// 需要和绝对路径组合才能形成完整路径。只能通过绝对路径打开文件。
class RelativePath
{
  public:
    RelativePath(std::string_view path, PathOptions opt = PathOptions::None);

    RelativePath(std::wstring_view path, PathOptions opt = PathOptions::None) : RelativePath{ToUtf8(path), opt}
    {
    }

    RelativePath(std::u16string_view path, PathOptions opt = PathOptions::None) : RelativePath{ToUtf8(path), opt}
    {
    }

  private:
    std::vector<std::string> m_components;
};

/// 绝对路径
class Path
{
  public:
    Path(std::string_view path, PathOptions opt = PathOptions::None);
};

} // namespace llama