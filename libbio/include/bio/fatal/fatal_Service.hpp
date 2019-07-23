
#pragma once
#include <bio/fatal/fatal_Types.hpp>
#include <bio/ipc/ipc_Request.hpp>

namespace bio::fatal
{
    class Service : public ipc::ServiceSession
    {
        public:
            Service();
            static std::shared_ptr<Service> Initialize();

            Result ThrowWithPolicy(Result res, ThrowMode mode);
    };
}