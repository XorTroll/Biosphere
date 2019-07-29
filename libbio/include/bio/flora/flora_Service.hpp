
#pragma once
#include <bio/flora/flora_Types.hpp>
#include <bio/ipc/ipc_Request.hpp>

namespace bio::flora
{
    class Service : public ipc::ServiceSession
    {
        public:
            Service();
            static ResultWith<std::shared_ptr<Service>> Initialize();

            Result WriteOut(char *log, size_t log_size);
            Result WriteErr(char *log, size_t log_size);
    };
}