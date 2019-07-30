#include <flora/flora_ILogService.hpp>

namespace flora
{
    Result ILogService::Initialize(PidDescriptor pid)
    {
        auto rc = pminfoInitialize();
        if(rc == 0)
        {
            rc = pminfoGetTitleId(&app_id, pid.pid);
            initialized = R_SUCCEEDED(rc);
            pminfoExit();
        }
        return rc;
    }

    static bool SendLogHeader(u64 appid, u32 type)
    {
        u8 log_header[sizeof(u64) + sizeof(u32)] = {0};
        memcpy(&log_header[0], &appid, sizeof(u64));
        memcpy(&log_header[sizeof(u64)], &type, sizeof(u32));
        return SendRaw(log_header, sizeof(u64) + sizeof(u32));
    }

    static bool SendString(char *buf, u32 len)
    {
        bool ok = SendRaw(&len, sizeof(u32));
        if(ok) ok = SendRaw(buf, len);
        return ok;
    }

    Result ILogService::Write(u32 type, InBuffer<char> log)
    {
        if(!initialized) return 0xDEAD1;
        bool ok = SendLogHeader(app_id, type);
        if(ok) ok = SendString(log.buffer, log.num_elements);
        return ok ? 0 : 0xDEAD2;
    }

    Result ILogService::WriteOut(InBuffer<char> log)
    {
        return Write(1, log);
    }

    Result ILogService::WriteErr(InBuffer<char> log)
    {
        return Write(2, log);
    }
}