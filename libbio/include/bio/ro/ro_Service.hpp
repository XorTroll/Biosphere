
#pragma once
#include <bio/ipc/ipc_Request.hpp>

namespace bio::ro
{
    class Service final : public ipc::ServiceSession
    {
        public:
            Service();
            static std::shared_ptr<Service> Initialize();
            Result LoadNro(void *nro_address, u64 nro_size, void *bss_address, size_t bss_size, Out<u64> nro_addr);
            Result UnloadNro(void *nro_address);
            Result LoadNrr(void *nrr_address, u64 nrr_size);
            Result UnloadNrr(void *nrr_address);
    };
}