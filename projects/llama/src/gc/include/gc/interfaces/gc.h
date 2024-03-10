#pragma once
#include "base/interfaces/has_canonical_address.h"

namespace llama
{
namespace gc
{

class IVistor
{
	
};

class IObject
{
public:
    virtual ~IObject() = default;
    void Trace() const
    {
        return Trace_gc_IObject();
    }

private:
    virtual void Trace_gc_IObject() const = 0;
};

} // namespace gc
} // namespace llama