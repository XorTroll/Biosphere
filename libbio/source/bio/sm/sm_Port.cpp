#include <bio/sm/sm_Port.hpp>
#include <bio/svc/svc_Base.hpp>
#include <bio/ipc/ipc_Request.hpp>

namespace bio::sm
{
    static ipc::Session _inner_Sm(InvalidObject);
    static bool _inner_Initialized = false;

    Result Initialize()
    {
        if(_inner_Initialized) return 0;
        u32 port = 0;
        auto res = svc::ConnectToNamedPort(port, "sm:");
        if(res.IsSuccess())
        {
            KObject smh(port);
            _inner_Sm = ipc::Session(smh);
            res = _inner_Sm.ProcessRequest<0>(ipc::InProcessId(), ipc::InRaw<u64>(0), ipc::InRaw<u64>(0), ipc::InRaw<u64>(0));
            if(res.IsSuccess()) _inner_Initialized = true;
        }
        return res;
    }

    bool IsInitialized()
    {
        return _inner_Initialized;
    }

    ipc::Session &GetSession()
    {
        return _inner_Sm;
    }

    static u64 _inner_ConvertServiceName(const char *name)
    {
        u64 converted = 0;
        for(u32 i = 0; i < 8; i++)
        {
            if(name[i] == '\0') break;
            converted |= ((u64)name[i]) << (8 * i);
        }
        return converted;
    }

    Result GetService(const char *name, KObject &handle)
    {
        return _inner_Sm.ProcessRequest<1>(bio::ipc::InRaw<bio::u64>(_inner_ConvertServiceName(name)), bio::ipc::InRaw<bio::u64>(0), bio::ipc::InRaw<bio::u64>(0), bio::ipc::OutHandle<0>(handle));
    }
}