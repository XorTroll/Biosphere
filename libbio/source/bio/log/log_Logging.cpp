#include <bio/log/log_Logging.hpp>
#include <bio/svc/svc_Base.hpp>
#include <bio/flora/flora_Service.hpp>
#include <cstring>
#include <algorithm>

bio::log::LogWriteFunction global_StdoutLog = bio::log::SvcStdoutLoggingFunction;
bio::log::LogWriteFunction global_StderrLog = bio::log::SvcStderrLoggingFunction;

namespace bio::log
{
    static std::shared_ptr<flora::Service> _inner_FloraSession;
    static bool _inner_Initialized = false;

    void SetStdoutLoggingFunction(LogWriteFunction func)
    {
        global_StdoutLog = func;
    }

    void SetStderrLoggingFunction(LogWriteFunction func)
    {
        global_StderrLog = func;
    }

    void SvcStdoutLoggingFunction(const void *ptr, size_t sz)
    {
        char *log = new char[sz + 0x20]();
        sprintf(log, "(stdout) %s", (char*)ptr);
        svc::OutputDebugString(log, sz + 0x20);
        delete[] log;
    }

    void SvcStderrLoggingFunction(const void *ptr, size_t sz)
    {
        char *log = new char[sz + 0x20]();
        sprintf(log, "(stderr) %s", (char*)ptr);
        svc::OutputDebugString(log, sz + 0x20);
        delete[] log;
    }

    static bool _inner_EnsureFloraIsInitialized()
    {
        if(!_inner_Initialized)
        {
            auto &[res, flora] = flora::Service::Initialize().Assert();
            if(res.IsSuccess())
            {
                _inner_FloraSession = std::move(flora);
                _inner_Initialized = true;
            }
        }
        return _inner_Initialized;
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