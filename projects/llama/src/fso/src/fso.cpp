#include "fso/interface/fso.h"
#include "fso/hasher.h"
namespace llama
{
namespace fso
{


LLAMA_API(fso) Hash IFso::GetHash()
{
    Hasher hash;
    return hash.Calculate(*this);
}

} // namespace fso
} // namespace llama