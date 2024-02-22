#include "foundation/rtti.h"
#include <array>

namespace llama
{

RttiContext &RttiContext::GetDefaultInstance()
{
    static RttiContext context{false};
    return context;
}

} // namespace llama
