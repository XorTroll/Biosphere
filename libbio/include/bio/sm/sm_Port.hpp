
#pragma once
#include <bio/ipc/ipc_Request.hpp>

namespace bio::sm
{
    Result Initialize();
    bool IsInitialized();
    void Finalize();
    std::shared_ptr<ipc::Session> &GetSession();

    Result GetService(const char *name, Out<u32> handle);
    Result RegisterService(const char *name, bool is_light, u32 max_sessions, Out<u32> handle);
    Result UnregisterService(const char *name);

    bool IsServiceRegistered(const char *name);
}