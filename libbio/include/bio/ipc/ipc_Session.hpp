
#pragma once
#include <bio/ipc/ipc_Types.hpp>
#include <bio/os/os_TLS.hpp>
#include <bio/log/log_Logging.hpp>
#include <memory>

namespace bio::ipc
{
    class Session
    {
        public:
            Session();
            Session(u32 handle);
            Session(Session &parent, u32 object_id);
            Session(u32 parent_handle, u32 object_id);
            Session(Session &&other);
            Session &operator=(Session &&other);
            void Close();
            virtual ~Session();
            u32 GetHandle();
            u32 GetObjectId();
            SessionType GetType();
            bool IsValid();
            bool IsNormalService();
            bool IsDomain();
            bool IsDomainSubService();
            Result ConvertToDomain();
            Result QueryPointerBufferSize(Out<size_t> size);
            void Claim(u32 got_handle, Out<u32> got_object_id, Out<SessionType> got_type);

            template<u32 CommandId, typename ...Arguments>
            Result ProcessRequest(Arguments &&...Args)
            {
                RequestData rq = {};
                rq.requester_session_handle = handle;
                rq.requester_session_object_id = object_id;
                rq.requester_session_is_domain = (IsDomain() || IsDomainSubService());

                rq.in_raw_size += (alignof(u64) - 1);
                rq.in_raw_size -= (rq.in_raw_size % alignof(u64));
                u64 magicoff = rq.in_raw_size;
                rq.in_raw_size += sizeof(u64);
                rq.in_raw_size += (alignof(u64) - 1);
                rq.in_raw_size -= (rq.in_raw_size % alignof(u64));
                u64 cmdidoff = rq.in_raw_size;
                rq.in_raw_size += sizeof(u64);

                (Args.Process(rq, 0), ...);
                PrepareCommandHeader(rq);

                *((u64*)(((u8*)rq.in_raw) + magicoff)) = SFCI;
                *((u64*)(((u8*)rq.in_raw) + cmdidoff)) = CommandId;
                (Args.Process(rq, 2), ...);

                Result rc = svc::SendSyncRequest(handle);
                if(rc.IsFailure()) return rc;

                rq.out_raw_size += (alignof(u64) - 1);
                rq.out_raw_size -= (rq.out_raw_size % alignof(u64));
                rq.out_raw_size += sizeof(u64);
                rq.out_raw_size += (alignof(u64) - 1);
                rq.out_raw_size -= (rq.out_raw_size % alignof(u64));
                u64 resoff = rq.out_raw_size;
                rq.out_raw_size += sizeof(u64);
                
                (Args.Process(rq, 3), ...);
                ProcessResponse(rq);

                (Args.Process(rq, 4), ...);
                (Args.Process(rq, 5), ...);
                rc = (u32)(*((u64*)(((u8*)rq.out_raw) + resoff)));
                return rc;
            }
        protected:
            u32 handle;
            SessionType type;
            u32 object_id;
    };

    class ServiceSession : public Session
    {
        public:
            ServiceSession(const char *name);
            const char *GetServiceName();
            Result GetInitialResult();

            static ResultWith<std::shared_ptr<ServiceSession>> Create(const char *name);
            static ResultWith<std::shared_ptr<ServiceSession>> CreateDomain(const char *name);
        private:
            Result initial_res;
            char srv_name[0x10];
    };
}