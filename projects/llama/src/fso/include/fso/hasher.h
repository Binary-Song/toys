#include "fso/interface/fso.h"
#include "hash.h"
#include <memory>

namespace llama
{

namespace fso
{

class Hasher : public virtual IVisitor
{
public:
    Hasher();
    ~Hasher();
    Hasher(Hasher &&) noexcept;
    Hasher &operator=(Hasher &&) noexcept;
    // no copy
    Hasher(const Hasher &) = delete;
    Hasher &operator=(const Hasher &) = delete;

    static Hash Calculate(IFso &object);

private:
    virtual void Visit_fso_IVisitor(IProperty &buffer) override;

	// pimpl，避免引入cryptopp
    class Impl;

private:
    std::unique_ptr<Impl> m_impl;
};

} // namespace fso

} // namespace llama