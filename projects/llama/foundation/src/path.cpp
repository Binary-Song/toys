#include "foundation/path.h"
#include "foundation/enums.h"
#include <cctype>

namespace llama
{

static bool IsSep(char ch)
{
#ifdef LLAMA_WIN
    return ch == '/' || ch == '\\';
#endif
    return ch == '/';
}

static bool IsValidRelativePath(std::string_view path)
{
#ifdef LLAMA_WIN
    // C:\ 或者 C:/
    if (path.size() >= 3 && tolower(path[0]) >= 'a' && tolower(path[0]) <= 'z' && path[1] == ':' && IsSep(path[2]))
    {
        return false;
    }
    // 这个和linux的绝对路径比较像，也ban了
    if (path.starts_with('\\'))
    {
        return false;
    }
#else
    if (path.starts_with('/'))
    {
        return false;
    }
#endif
    return true;
}

static bool IsValidAbsolutePath(std::string_view path)
{
#ifdef LLAMA_WIN
    // C:\ 或者 C:/
    if (path.size() >= 3 && tolower(path[0]) >= 'a' && tolower(path[0]) <= 'z' && path[1] == ':' && IsSep(path[2]))
    {
        return true;
    }
#else
    if (path.starts_with('/'))
    {
        return true;
    }
#endif
    return false;
}

RelativePath::RelativePath(std::string_view path, ReadPathOptions opt)
{
    if (!IsValidRelativePath(path))
        throw Exception(ExceptionKind::InvalidRelativePath);

    std::string component;
    for (size_t index = 0; index < path.size(); index++)
    {
        char ch = path[index];
        if (!IsSep(ch))
        {
            component.push_back(ch);
        }
        else
        {
            if (!component.empty())
            {
                // 检查是否允许 ".." 路径
                if (component == ".." && !(opt & ReadPathOptions::AllowDotDot))
                {
                    throw Exception(ExceptionKind::InvalidRelativePath);
                }
                m_components.push_back(std::move(component));
            }
        }
    }
    if (!component.empty())
    {
        m_components.push_back(std::move(component));
    }
}

Path::Path(std::string_view path, ReadPathOptions opt)
{
    if (!IsValidAbsolutePath(path))
        throw Exception(ExceptionKind::InvalidRelativePath);
}

} // namespace llama