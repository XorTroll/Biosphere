#include <bio/fatal/fatal_Service.hpp>

namespace bio::fatal
{
    Service::Service() : ServiceSession("fatal:u")
    {
    }

    ResultWith<std::shared_ptr<Service>> Service::Initialize()
    {
        auto srv = std::make_shared<Service>();
        return ResultWith<std::shared_ptr<Service>>(srv->GetInitialResult(), std::move(srv));
    }

    Result Service::ThrowWithPolicy(Result res, ThrowMode mode)
    {
        return ProcessRequest<1>(ipc::InProcessId(), ipc::InRaw<u32>(static_cast<u32>(res)), ipc::InRaw<u32>(static_cast<u32>(mode)), ipc::InRaw<u64>(0));
    }
}