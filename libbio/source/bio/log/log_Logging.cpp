#include <bio/log/log_Logging.hpp>
#include <bio/svc/svc_Base.hpp>
#include <bio/flora/flora_Service.hpp>
#include <cstring>
#include <algorithm>
#include <string>

bio::log::LogWriteFunction global_StdoutLog = bio::log::FloraStdoutLoggingFunction;
bio::log::LogWriteFunction global_StderrLog = bio::log::FloraStderrLoggingFunction;

namespace bio::log
{
    static std::shared_ptr<flora::Service> _inner_FloraSession;
    static bool _inner_FloraInitialized = false;

    void SetStdoutLoggingFunction(LogWriteFunction func)
    {
        global_StdoutLog = func;
    }

    void SetStderrLoggingFunction(LogWriteFunction func)
    {
        global_StderrLog = func;
    }

    #define SVC_LOG_STD(type, ptr, sz) do { \
        svc::OutputDebugString((char*)ptr, sz); \
        } while(0)

    void SvcStdoutLoggingFunction(const void *ptr, size_t sz)
    {
        SVC_LOG_STD(stdout, ptr, sz);
    }

    void SvcStderrLoggingFunction(const void *ptr, size_t sz)
    {
        SVC_LOG_STD(stderr, ptr, sz);
    }

    static bool _inner_EnsureFloraIsInitialized()
    {
        if(!_inner_FloraInitialized)
        {
            auto [res, flora] = flora::Service::Initialize();
            if(res.IsSuccess())
            {
                _inner_FloraSession = std::move(flora);
                _inner_FloraInitialized = true;
            }
        }
        return _inner_FloraInitialized;
    }

    void FloraStdoutLoggingFunction(const void *ptr, size_t sz)
    {
        if(_inner_EnsureFloraIsInitialized()) _inner_FloraSession->WriteOut((char*)ptr, sz);
    }

    void FloraStderrLoggingFunction(const void *ptr, size_t sz)
    {
        if(_inner_EnsureFloraIsInitialized()) _inner_FloraSession->WriteErr((char*)ptr, sz);
    }
}