#include <bio/sm/sm_Port.hpp>
#include <bio/svc/svc_Base.hpp>
#include <bio/ipc/ipc_Request.hpp>
#include <bio/os/os_Finalize.hpp>

namespace bio::sm
{
    static std::shared_ptr<ipc::Session> _inner_SmPort;
    static bool _inner_Initialized = false;

    Result Initialize()
    {
        if(_inner_Initialized) return 0;
        u32 port = 0;
        auto res = svc::ConnectToNamedPort(port, "sm:");
        if(res.IsSuccess())
        {
            _inner_SmPort = std::make_shared<ipc::Session>(port);

            // Check if smhax is present
            u32 dummyfsphandle;
            if(GetService("fsp-srv", dummyfsphandle).IsSuccess())
            {
                ipc::CloseSessionHandle(dummyfsphandle);
                _inner_Initialized = true;
                os::AddFinalizeFunction(sm::Finalize);
                return 0;
            }

            res = _inner_SmPort->ProcessRequest<0>(ipc::InProcessId(), ipc::InRaw<u64>(0), ipc::InRaw<u64>(0), ipc::InRaw<u64>(0));
            if(res.IsSuccess())
            {
                _inner_Initialized = true;
                os::AddFinalizeFunction(sm::Finalize);
            }
        }
        return res;
    }

    bool IsInitialized()
    {
        return _inner_Initialized;
    }

    void Finalize()
    {
        if(_inner_Initialized)
        {
            _inner_SmPort.reset();
            _inner_Initialized = false;
        }
    }

    std::shared_ptr<ipc::Session> &GetSession()
    {
        return _inner_SmPort;
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

    Result GetService(const char *name, Out<u32> handle)
    {
        return _inner_SmPort->ProcessRequest<1>(ipc::InRaw<u64>(_inner_ConvertServiceName(name)), ipc::InRaw<u64>(0), ipc::InRaw<u64>(0), ipc::OutHandle<0>(static_cast<u32&>(handle)));
    }

    Result RegisterService(const char *name, bool is_light, u32 max_sessions, Out<u32> handle)
    {
        if(!IsServiceRegistered(name)) return ResultServiceNotPresent;
        return _inner_SmPort->ProcessRequest<2>(ipc::InRaw<u64>(_inner_ConvertServiceName(name)), ipc::InRaw<u32>((u32)is_light), ipc::InRaw<u32>(max_sessions), ipc::OutHandle<0>(static_cast<u32&>(handle)));
    }

    Result UnregisterService(const char *name)
    {
        return _inner_SmPort->ProcessRequest<3>(ipc::InRaw<u64>(_inner_ConvertServiceName(name)), ipc::InRaw<u64>(0));
    }

    bool IsServiceRegistered(const char *name)
    {
        u32 dummyhandle;
        auto res = RegisterService(name, false, 1, dummyhandle);
        if(res.IsFailure()) return true;
        UnregisterService(name);
        ipc::CloseSessionHandle(dummyhandle);
        return false;
    }
}