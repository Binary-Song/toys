#include "fso/hasher.h"
#include "base/misc.h"
#include "cryptopp/sha.h"
namespace llama
{
namespace fso
{

using HashAlgorithm = CryptoPP::SHA1;

class Hasher::Impl
{
public:
    Impl() : hash(), digest()
    {
    }

    HashAlgorithm hash;
    Hash digest;
};

Hasher::Hasher() : m_impl(new Hasher::Impl{})
{
    static_assert(Hash().size() == HashAlgorithm::DIGESTSIZE);
}

Hasher::~Hasher() = default;
Hasher::Hasher(Hasher &&) noexcept = default;
Hasher &Hasher::operator=(Hasher &&) noexcept = default;

void Hasher::Visit_fso_IVisitor(IProperty &buffer)
{
    std::string val = buffer.GetValue();
    m_impl->hash.Update((const byte *)val.data(), val.size());
}

Hash Hasher::Calculate(IFso &object)
{
    Hasher hasher;
    object.Accept(hasher);
    hasher.m_impl->hash.Final((byte *)hasher.m_impl->digest.data());
    return hasher.m_impl->digest;
}

} // namespace fso
} // namespace llama