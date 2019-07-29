#include <bio/flora/flora_Service.hpp>

namespace bio::flora
{
    Service::Service() : ServiceSession("flora")
    {
    }

    ResultWith<std::shared_ptr<Service>> Service::Initialize()
    {
        auto srv = std::make_shared<Service>();
        srv->ProcessRequest<0>(ipc::InProcessId()).Assert();
        return ResultWith<std::shared_ptr<Service>>(srv->GetInitialResult(), std::move(srv));
    }

    Result Service::WriteOut(char *log, size_t log_size)
    {
        return ProcessRequest<1>(ipc::InBuffer(log, log_size, 0));
    }

    Result Service::WriteErr(char *log, size_t log_size)
    {
        return ProcessRequest<2>(ipc::InBuffer(log, log_size, 0));
    }
}