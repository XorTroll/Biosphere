#include <bio/hid/hid_Service.hpp>

namespace bio::hid
{
    Result AppletResource::GetSharedMemoryHandle(Out<KObject> handle)
    {
        return ProcessRequest<0>(ipc::OutHandle<0>(static_cast<KObject&>(handle)));
    }

    std::shared_ptr<Service> Service::Initialize()
    {
        return std::make_shared<Service>();
    }

    Service::Service() : ServiceSession("hid")
    {
    }

    Result Service::CreateAppletResource(u64 aruid, Out<std::shared_ptr<AppletResource>> out_res)
    {
        KObject handle;
        auto res = ProcessRequest<0>(ipc::InProcessId(), ipc::InRaw<u64>(aruid), ipc::OutHandle<0>(handle));
        if(res.IsSuccess()) (std::shared_ptr<AppletResource>&)out_res = std::make_shared<AppletResource>(handle);
        return res;
    }

    Result Service::SetSupportedNpadStyleSet(u64 aruid, u32 npad_style_tag)
    {
        return ProcessRequest<100>(ipc::InProcessId(), ipc::InRaw<u32>(npad_style_tag), ipc::InRaw<u64>(aruid));
    }

    Result Service::SetSupportedNpadIdType(u64 aruid, u32 *controllers, size_t controllers_count)
    {
        return ProcessRequest<102>(ipc::InProcessId(), ipc::InStaticBuffer(controllers, (controllers_count * sizeof(u32)), 0), ipc::InRaw<u64>(aruid));
    }

    Result Service::ActivateNpad(u64 aruid)
    {
        return ProcessRequest<103>(ipc::InProcessId(), ipc::InRaw<u64>(aruid));
    }

    Result Service::SetNpadJoyAssignmentModeSingle(u64 aruid, u32 controller, u64 joy_type)
    {
        return ProcessRequest<123>(ipc::InProcessId(), ipc::InRaw<u32>(controller), ipc::InRaw<u64>(aruid), ipc::InRaw<u64>(joy_type));
    }

    Result Service::SetNpadJoyAssignmentModeDual(u64 aruid, u32 controller)
    {
        return ProcessRequest<124>(ipc::InProcessId(), ipc::InRaw<u32>(controller), ipc::InRaw<u64>(aruid));
    }
}