#include "fso/serializer.h"
#include "base/rtti.h"
#include "fso/interface/fso.h"
#include "fso/properties.h"

namespace llama
{
namespace fso
{

LLAMA_API(fso) void Serializer::Execute(RttiContext &context, IFso &object)
{
    // 写入类型名称
    std::string type(object.GetTypeId());
    StringProperty prop{type};
    Visit(prop);
    // 读取对象写入流
    object.Accept(*this);
}

} // namespace fso
} // namespace llama