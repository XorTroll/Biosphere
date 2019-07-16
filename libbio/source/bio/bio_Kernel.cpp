#include <bio_Kernel.hpp>
#include <bio/svc/svc_Base.hpp>
#include <utility>

namespace bio
{
    KObject::KObject() : handle(0)
    {
    }

    KObject::KObject(u32 handle) : handle(handle)
    {
    }

    KObject::KObject(KObject &&other)
    {
        this->handle = other.handle;
        other.handle = 0;
    }

    KObject &KObject::operator=(KObject &&other)
    {
        this->handle = other.handle;
        other.handle = 0;
        return *this;
    }

    KObject::~KObject()
    {
        if(handle > 0)
        {
            svc::CloseHandle(handle).Assert();
        }
    }

    u32 KObject::Claim()
    {
        u32 handle = this->handle;
        this->handle = 0;
        return handle;
    }
}