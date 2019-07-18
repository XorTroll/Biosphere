
#pragma once
#include <bio/hid/hid_Types.hpp>
#include <bio/ipc/ipc_Request.hpp>

namespace bio::hid
{
    class AppletResource : public ipc::Session
    {
        public:
            using Session::Session;
            Result GetSharedMemoryHandle(Out<u32> handle);
    };

    class Service : public ipc::ServiceSession
    {
        public:
            Service();
            static std::shared_ptr<Service> Initialize();

            Result CreateAppletResource(u64 aruid, Out<std::shared_ptr<AppletResource>> out_res);
            Result SetSupportedNpadStyleSet(u64 aruid, u32 npad_style_tag);
            Result SetSupportedNpadIdType(u64 aruid, u32 *controllers, size_t controllers_count);
            Result ActivateNpad(u64 aruid);
            Result SetNpadJoyAssignmentModeSingle(u64 aruid, u32 controller, u64 joy_type);
            Result SetNpadJoyAssignmentModeDual(u64 aruid, u32 controller);
    };
}