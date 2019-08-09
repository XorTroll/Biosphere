#include <bio/ipc/ipc_Session.hpp>
#include <vector>
#include <bio/sm/sm_Port.hpp>
#include <cstring>

namespace bio::ipc
{
    Session::Session() : handle(0), type(SessionType::Invalid), object_id(UINT32_MAX)
    {
    }

    Session::Session(u32 handle) : handle(handle), type(SessionType::Normal), object_id(UINT32_MAX)
    {
    }

    Session::Session(Session &parent, u32 object_id) : handle(parent.GetHandle()), type(SessionType::DomainSubService), object_id(object_id)
    {
    }

    Session::Session(u32 parent_handle, u32 object_id) : handle(parent_handle), type(SessionType::DomainSubService), object_id(object_id)
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
                svc::SendSyncRequest(handle);
            }
            else CloseSessionHandle(handle);
        }
        type = SessionType::Invalid;
        object_id = UINT32_MAX;
    }

    Session::~Session()
    {
        Close();
    }

    u32 Session::GetHandle()
    {
        return handle;
    }

    void Session::Claim(u32 got_handle, Out<u32> got_object_id, Out<SessionType> got_type)
    {
        got_handle = handle;
        (u32&)got_object_id = object_id;
        (SessionType&)got_type = type;
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
            rc = svc::SendSyncRequest(handle);
            if(rc.IsFailure()) return rc;
        
            RequestData rq = {};
            rq.requester_session_is_domain = false;
            BIO_IPC_PROCESS_TYPE_RAW(u64, rq.out_raw_size)
            BIO_IPC_PROCESS_TYPE_RAW_OUT(u64, rq.out_raw_size, u64 offcmdres)
            BIO_IPC_PROCESS_TYPE_RAW_OUT(u32, rq.out_raw_size, u64 offobjid)
            ProcessResponse(rq);

            rc = (u32)*((u64*)(((u8*)rq.out_raw) + offcmdres));
            if(rc.IsSuccess())
            {
                object_id = *((u32*)(((u8*)rq.out_raw) + offobjid));
                type = SessionType::Domain;
            }
        }
        return rc;
    }

    Result Session::QueryPointerBufferSize(Out<size_t> size)
    {
        u32 *tls = (u32*)os::GetTLS();
        tls[0] = 5;
        tls[1] = 8;
        tls[2] = 0;
        tls[3] = 0;
        tls[4] = SFCI;
        tls[5] = 0;
        tls[6] = 3;
        tls[7] = 0;
        auto rc = svc::SendSyncRequest(handle);
        if(rc.IsFailure()) return rc;

        RequestData rq = {};
        rq.requester_session_is_domain = false;
        BIO_IPC_PROCESS_TYPE_RAW(u64, rq.out_raw_size)
        BIO_IPC_PROCESS_TYPE_RAW_OUT(u64, rq.out_raw_size, u64 offcmdres)
        BIO_IPC_PROCESS_TYPE_RAW_OUT(u32, rq.out_raw_size, u64 offsize)
        ProcessResponse(rq);

        rc = (u32)*((u64*)(((u8*)rq.out_raw) + offcmdres));
        if(rc.IsSuccess()) (size_t&)size = (size_t)(*((u32*)(((u8*)rq.out_raw) + offsize)) & 0xffff);
        return rc;
    }

    ServiceSession::ServiceSession(const char *name)
    {
        strcpy(srv_name, name);
        initial_res = sm::Initialize();
        if(initial_res.IsSuccess()) initial_res = sm::GetService(name, handle);
    }

    const char *ServiceSession::GetServiceName()
    {
        return srv_name;
    }

    Result ServiceSession::GetInitialResult()
    {
        return initial_res;
    }

    ResultWith<std::shared_ptr<ServiceSession>> ServiceSession::Create(const char *name)
    {
        auto srv = std::make_shared<ServiceSession>(name);
        return MakeResultWith(srv->GetInitialResult(), std::move(srv));
    }

    ResultWith<std::shared_ptr<ServiceSession>> ServiceSession::CreateDomain(const char *name)
    {
        auto srv = std::make_shared<ServiceSession>(name);
        auto res = srv->GetInitialResult();
        if(res.IsSuccess()) res = srv->ConvertToDomain();
        return MakeResultWith(res, std::move(srv));
    }
}