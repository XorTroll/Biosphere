#include <bio/ipc/ipc_Session.hpp>
#include <vector>
#include <bio/sm/sm_Port.hpp>
#include <cstring>

namespace bio::ipc
{
    Session::Session() : handle(0), type(SessionType::Invalid), object_id(UINT32_MAX)
    {
    }

    Session::Session(KObject &handle) : handle(handle.Claim()), type(SessionType::Normal), object_id(UINT32_MAX)
    {
    }

    Session::Session(Session &parent, u32 object_id) : handle(parent.GetHandle().handle), type(SessionType::DomainSubService), object_id(object_id)
    {
    }

    Session::Session(Session &&other) : handle(0)
    {
        other.Claim(handle, object_id, type);
    }

    Session &Session::operator=(Session &&other)
    {
        other.Claim(handle, object_id, type);
        return *this;
    }

    void Session::Close()
    {
        if(IsValid())
        {
            if(IsDomainSubService())
            {
                RequestData rq;
                rq.in_raw_size = sizeof(DomainHeader);
                u32 *tls = (u32*)os::GetTLS();
                *tls++ = 4;
                u32 *fillsz = tls;
                *tls = 0;
                tls++;
                u32 pad = (((16 - (((uintptr_t)tls) & 15)) & 15) / 4);
                u32 *raw = (u32*)(tls + pad);
                size_t rawsz = ((rq.in_raw_size / 4) + 4);
                tls += rawsz;
                u16 *tls16 = (u16*)tls;
                size_t u16s = ((2 * rq.out_bufs_static_size + 3) / 4);
                tls += u16s;
                rawsz += u16s;
                *fillsz |= rawsz;
                DomainHeader *iraw = (DomainHeader*)raw;
                iraw->type = 2;
                iraw->object_id_count = 0;
                iraw->size = 0;
                iraw->object_id = object_id;
                iraw->pad[0] = iraw->pad[1] = 0;
                svc::SendSyncRequest(handle.handle);
            }
            else
            {
                u32 *tls = (u32*)os::GetTLS();
                tls[0] = 2;
                tls[1] = 0;
                svc::SendSyncRequest(handle.handle);
                svc::CloseHandle(handle.handle);
            }
        }
        type = SessionType::Invalid;
        object_id = UINT32_MAX;
    }

    Session::~Session()
    {
        Close();
    }

    KObject &Session::GetHandle()
    {
        return handle;
    }

    void Session::Claim(KObject &got_handle, Out<u32> got_object_id, Out<SessionType> got_type)
    {
        got_handle.handle = handle.Claim();
        got_object_id.Set(object_id);
        got_type.Set(type);
        type = SessionType::Invalid;
    }

    u32 Session::GetObjectId()
    {
        return object_id;
    }

    SessionType Session::GetType()
    {
        return type;
    }

    bool Session::IsNormalService()
    {
        return (GetType() != SessionType::Normal);
    }

    bool Session::IsValid()
    {
        return (GetType() != SessionType::Invalid);
    }

    bool Session::IsDomain()
    {
        return (GetType() == SessionType::Domain);
    }

    bool Session::IsDomainSubService()
    {
        return (GetType() == SessionType::DomainSubService);
    }

    Result Session::ConvertToDomain()
    {
        Result rc = 0;
        if((!IsDomain()) && (!IsDomainSubService()))
        {
            u32 *tls = (u32*)os::GetTLS();
            tls[0] = 5;
            tls[1] = 8;
            tls[4] = SFCI;
            tls[5] = 0;
            tls[6] = 0;
            tls[7] = 0;
            rc = svc::SendSyncRequest(handle.handle);
            if(rc.IsFailure()) return rc;
            u32 ctrl0 = *tls++;
            u32 ctrl1 = *tls++;
            bool gotpid = false;
            u64 outpid = 0;
            std::vector<u32> outhdls;
            if(ctrl1 & 0x80000000)
            {
                u32 ctrl2 = *tls++;
                if(ctrl2 & 1)
                {
                    gotpid = true;
                    u64 opid = *tls++;
                    opid |= (((u64)(*tls++)) << 32);
                    outpid = opid;
                }
                size_t shcopy = ((ctrl2 >> 1) & 15);
                size_t shmove = ((ctrl2 >> 5) & 15);
                size_t sh = shcopy + shmove;
                u32 *ashbuf = tls + sh;
                if(sh > 8) sh = 8;
                outhdls.reserve(sh);
                if(sh > 0) for(u32 i = 0; i < sh; i++) outhdls.push_back((u32)(*(tls + i)));
                tls = ashbuf;
            }
            size_t s_st = ((ctrl0 >> 16) & 15);
            u32 *abst = (tls + s_st * 2);
            if(s_st > 8) s_st = 8;
            for(u32 i = 0; i < s_st; i++, tls += 2)
            {
                BufferSendData *bsd = (BufferSendData*)tls;
                BIO_IGNORE(bsd);
            }
            tls = abst;
            size_t sends = ((ctrl0 >> 20) & 15);
            size_t recvs = ((ctrl0 >> 24) & 15);
            size_t exchs = ((ctrl0 >> 28) & 15);
            size_t bufs = (sends + recvs + exchs);
            void *outraw = (void*)(((uintptr_t)(tls + bufs * 3) + 15) &~ 15);
            void *wpadraw = (void*)((uintptr_t)(tls + bufs * 3));
            BIO_IGNORE(wpadraw);
            if(bufs > 8) bufs = 8;
            for(u32 i = 0; i < bufs; i++, tls += 3)
            {
                BufferCommandData *bcd = (BufferCommandData*)tls;
                BIO_IGNORE(bcd);
            }
            struct ConvertRaw
            {
                u64 magic;
                u64 res;
                u32 oid;
            } *oraw = (ConvertRaw*)outraw;
            rc = oraw->res;
            if(rc.IsSuccess())
            {
                object_id = oraw->oid;
                type = SessionType::Domain;
            }
        }
        return rc;
    }

    Result Session::QueryPointerBufferSize(Out<size_t> size)
    {
        Result rc = 0;
        u32 *tls = (u32*)os::GetTLS();
        tls[0] = 5;
        tls[1] = 8;
        tls[2] = 0;
        tls[3] = 0;
        tls[4] = SFCI;
        tls[5] = 0;
        tls[6] = 3;
        tls[7] = 0;
        rc = svc::SendSyncRequest(handle.handle);
        if(rc.IsFailure()) return rc;
        u32 ctrl0 = *tls++;
        u32 ctrl1 = *tls++;
        bool gotpid = false;
        u64 outpid = 0;
        std::vector<u32> outhdls;
        if(ctrl1 & 0x80000000)
        {
            u32 ctrl2 = *tls++;
            if(ctrl2 & 1)
            {
                gotpid = true;
                u64 opid = *tls++;
                opid |= (((u64)(*tls++)) << 32);
                outpid = opid;
            }
            size_t shcopy = ((ctrl2 >> 1) & 15);
            size_t shmove = ((ctrl2 >> 5) & 15);
            size_t sh = shcopy + shmove;
            u32 *ashbuf = tls + sh;
            if(sh > 8) sh = 8;
            outhdls.reserve(sh);
            if(sh > 0) for(u32 i = 0; i < sh; i++) outhdls.push_back((u32)(*(tls + i)));
            tls = ashbuf;
        }
        size_t s_st = ((ctrl0 >> 16) & 15);
        u32 *abst = (tls + s_st * 2);
        if(s_st > 8) s_st = 8;
        for(u32 i = 0; i < s_st; i++, tls += 2)
        {
            BufferSendData *bsd = (BufferSendData*)tls;
            BIO_IGNORE(bsd);
        }
        tls = abst;
        size_t sends = ((ctrl0 >> 20) & 15);
        size_t recvs = ((ctrl0 >> 24) & 15);
        size_t exchs = ((ctrl0 >> 28) & 15);
        size_t bufs = (sends + recvs + exchs);
        void *outraw = (void*)(((uintptr_t)(tls + bufs * 3) + 15) &~ 15);
        void *wpadraw = (void*)((uintptr_t)(tls + bufs * 3));
        BIO_IGNORE(wpadraw);
        if(bufs > 8) bufs = 8;
        for(u32 i = 0; i < bufs; i++, tls += 3)
        {
            BufferCommandData *bcd = (BufferCommandData*)tls;
            BIO_IGNORE(bcd);
        }
        struct QueryRaw
        {
            u64 magic;
            u64 res;
            u32 size;
        } *oraw = (QueryRaw*)outraw;
        rc = oraw->res;
        if(rc.IsSuccess()) size.Set((size_t)(oraw->size & 0xffff));
        return rc;
    }

    ServiceSession::ServiceSession(const char *name)
    {
        strcpy(srv_name, name);
        sm::Initialize().Assert();
        sm::GetService(name, handle).Assert();
    }

    const char *ServiceSession::GetServiceName()
    {
        return srv_name;
    }
}