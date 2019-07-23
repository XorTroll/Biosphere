#include <bio/fatal/fatal_Service.hpp>

namespace bio::fatal
{
    Service::Service() : ServiceSession("fatal:u")
    {
    }

    std::shared_ptr<Service> Service::Initialize()
    {
        return std::make_shared<Service>();
    }

    Result Service::ThrowWithPolicy(Result res, ThrowMode mode)
    {
        return ProcessRequest<1>(ipc::InProcessId(), ipc::InRaw<u32>(static_cast<u32>(res)), ipc::InRaw<u32>(static_cast<u32>(mode)), ipc::InRaw<u64>(0));
    }
}