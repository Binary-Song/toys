#pragma once
#include "base/exceptions.h"
#include "base/interfaces/has_canonical_address.h"
#include "base/misc.h"
#include "base/pointers.h"
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <optional>
#include <type_traits>
#include <unordered_map>

namespace llama
{
namespace gc
{

template <typename T> class gp : public p<T>
{
    static_assert(std::is_base_of<IHasCanonicalAddress, T>::value, "GC-able types must subclass IHasCanonicalAddress.");

public:
    using p<T>::p;

private:
    const void *m_canon_ptr = this->template static_as<IHasCanonicalAddress>()->GetCanonicalAddress();
};

template <typename T> class gnp : public np<T>
{
    static_assert(std::is_base_of<IHasCanonicalAddress, T>::value, "GC-able types must subclass IHasCanonicalAddress.");

public:
    using np<T>::np;

private:
    const void *m_canon_ptr = this->template static_as<IHasCanonicalAddress>()->GetCanonicalAddress();
};

template <typename T, typename... Args> gp<T> MakeGP(Args &&...args)
{
    p<T> ptr = new T(std::forward<Args>(args)...);
    return gp<T>(ptr);
}

} // namespace gc
} // namespace llama