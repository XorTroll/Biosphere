
#pragma once
#include <bio/ipc/ipc_Request.hpp>

namespace bio::sm
{
    Result Initialize();
    bool IsInitialized();
    void Finalize();
    std::shared_ptr<ipc::Session> &GetSession();

    Result GetService(const char *name, Out<u32> handle);
}