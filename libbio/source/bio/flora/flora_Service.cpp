#include <bio/flora/flora_Service.hpp>

namespace bio::flora
{
    Service::Service() : ServiceSession("flora")
    {
    }

    ResultWith<std::shared_ptr<Service>> Service::Initialize()
    {
        auto srv = std::make_shared<Service>();
        auto res = srv->GetInitialResult();
        if(res.IsSuccess()) res = srv->ProcessRequest<0>(ipc::InProcessId());
        return MakeResultWith(res, std::move(srv));
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