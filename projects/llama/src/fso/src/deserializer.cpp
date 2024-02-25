#include "fso/deserializer.h"
#include "base/rtti.h"
#include "fso/interface/fso.h"
#include "fso/properties.h"

namespace llama
{
namespace fso
{

LLAMA_API(fso) up<IFso> Deserializer::Execute(RttiContext &context)
{
    std::string type;
	// 读取类型名称
    StringProperty prop{type};
	Visit(prop);
	// 创建对象
    up<IRtti> obj = context.Instantiate(type);
    up<IFso> subobj(obj.release()->Cast<IFso>(context));
	// 写入对象
	subobj->Accept(*this);
    return subobj;
}

} // namespace fso
} // namespace llama