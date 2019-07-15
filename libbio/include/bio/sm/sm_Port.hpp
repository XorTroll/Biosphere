
#pragma once
#include <bio/bio_Kernel.hpp>
#include <ipc/ipc_Request.hpp>

namespace bio::sm
{
    Result Initialize();
    bool IsInitialized();
    ipc::Session &GetSession();
    Result GetService(const char *name, KObject &handle);
}