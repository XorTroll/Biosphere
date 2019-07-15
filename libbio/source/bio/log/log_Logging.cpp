#include <bio/log/log_Logging.hpp>
#include <bio/svc/svc_Base.hpp>

bio::log::LogWriteFunction global_StdoutLog = bio::log::SvcOutputLoggingFunction;

namespace bio::log
{
    void SetLoggingFunction(LogWriteFunction func)
    {
        global_StdoutLog = func;
    }

    void SvcOutputLoggingFunction(const void *ptr, size_t sz)
    {
        svc::OutputDebugString((char*)ptr, sz);
    }
}