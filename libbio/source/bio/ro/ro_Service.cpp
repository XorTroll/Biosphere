#include <bio/ro/ro_Service.hpp>

namespace bio::ro
{
    Service::Service() : ServiceSession("ldr:ro") // Use ldr:ro since it has been there since 1.0.0
    {
    }

    ResultWith<std::shared_ptr<Service>> Service::Initialize()
    {
        auto srv = std::make_shared<Service>();
        auto res = srv->GetInitialResult();
        if(res.IsSuccess()) res = srv->ProcessRequest<4>(ipc::InProcessId(), ipc::InHandle<ipc::HandleMode::Copy>(svc::CurrentProcessPseudoHandle));
        return MakeResultWith(res, std::move(srv));
    }

    Result Service::LoadNro(void *nro_address, u64 nro_size, void *bss_address, size_t bss_size, Out<u64> nro_addr)
    {
        return ProcessRequest<0>(ipc::InProcessId(), ipc::InRaw<u64>(0), ipc::InRaw<u64>((u64)nro_address), ipc::InRaw<u64>(nro_size), ipc::InRaw<u64>((u64)bss_address), ipc::InRaw<u64>(bss_size), ipc::OutRaw<u64>(static_cast<u64&>(nro_addr)));
    }

    Result Service::UnloadNro(void *nro_address)
    {
        return ProcessRequest<1>(ipc::InProcessId(), ipc::InRaw<u64>(0), ipc::InRaw<u64>((u64)nro_address));
    }

    Result Service::LoadNrr(void *nrr_address, u64 nrr_size)
    {
        return ProcessRequest<2>(ipc::InProcessId(), ipc::InRaw<u64>(0), ipc::InRaw<u64>((u64)nrr_address), ipc::InRaw<u64>(nrr_size));
    }

    Result Service::UnloadNrr(void *nrr_address)
    {
        return ProcessRequest<3>(ipc::InProcessId(), ipc::InRaw<u64>(0), ipc::InRaw<u64>((u64)nrr_address));
    }
}