#include <bio/os/os_Event.hpp>
#include <bio/svc/svc_Base.hpp>

namespace bio::os
{
    Event::Event(bool auto_clear, u32 w_handle, u32 r_handle) : cauto(auto_clear), whandle(w_handle), rhandle(r_handle)
    {
    }

    Event::~Event()
    {
        this->Close();
    }

    bool Event::IsValid()
    {
        return (this->rhandle != 0);
    }

    bool Event::AutoClears()
    {
        return this->cauto;
    }

    void Event::SetAutoClear(bool AutoClear)
    {
        this->cauto = AutoClear;
    }

    Result Event::Wait(u64 Timeout)
    {
        if(this->rhandle == 0) return 0xDEAD;
        Result rc;
        u64 deadline = 0;
        if(Timeout != UINT64_MAX) deadline = svc::GetSystemTick() + ((Timeout * 12) / 625);
        do
        {
            do
            {
                i64 stime = -1;
                if(deadline)
                {
                    stime = deadline - svc::GetSystemTick();
                    stime = (((stime >= 0) ? stime : 0) * 625) / 12;
                }
                u32 svctmp = 0;
                rc = svc::WaitSynchronization(svctmp, &this->rhandle, 1, stime);
                if(deadline && ((rc & 0x3FFFFF) == 0xEA01)) return rc;
            } while(rc.IsFailure());
            if(this->cauto) svc::SignalEvent(this->rhandle);
        } while((rc & 0x3FFFFF) == 0xFA01);
        return rc;
    }

    Result Event::Fire()
    {
        if(this->rhandle == 0) return 0xDEAD;
        return svc::SignalEvent(this->rhandle);
    }

    Result Event::Clear()
    {
        if(this->whandle != 0) return svc::ClearEvent(this->whandle);
        if(this->rhandle != 0) return svc::ResetSignal(this->rhandle);
        return 0xDEAD;
    }

    void Event::Close()
    {
        if(this->whandle != 0) svc::CloseHandle(this->whandle);
        if(this->rhandle != 0) svc::CloseHandle(this->rhandle);
        this->whandle = 0;
        this->rhandle = 0;
    }

    ResultWith<std::shared_ptr<Event>> Event::Create(bool auto_clear)
    {
        u32 w = 0;
        u32 r = 0;
        auto res = svc::CreateEvent(w, r);
        auto ev = std::make_shared<Event>(auto_clear, w, r);
        return MakeResultWith(res, std::move(ev));
    }

    std::shared_ptr<Event> Event::Open(u32 handle, bool auto_clear)
    {
        return std::make_shared<Event>(auto_clear, 0, handle);
    }
}