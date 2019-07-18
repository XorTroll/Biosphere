
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
                bool domainmode = (IsDomain() || IsDomainSubService());
                u64 orawsz = 0;
                RequestData rq = {};
                u32 *tls = (u32*)os::GetTLS();
                rq.in_raw_size += (alignof(u64) - 1);
                rq.in_raw_size -= (rq.in_raw_size % alignof(u64));
                u64 magicoff = rq.in_raw_size;
                rq.in_raw_size += sizeof(u64);
                rq.in_raw_size += (alignof(u64) - 1);
                rq.in_raw_size -= (rq.in_raw_size % alignof(u64));
                u64 cmdidoff = rq.in_raw_size;
                rq.in_raw_size += sizeof(u64);
                rq.requester_session_handle = handle;
                rq.requester_session_is_domain = domainmode;
                ProcessArgument(rq, 0, Args...);
                if(domainmode)
                {
                    orawsz = rq.in_raw_size;
                    rq.in_raw_size += sizeof(DomainHeader) + (rq.in_object_ids_size * sizeof(u32));
                }
                *tls++ = (4 | (rq.in_bufs_static_size << 16) | (rq.in_bufs_size << 20) | (rq.out_bufs_size << 24) | (rq.out_bufs_exch_size << 28));
                u32 *fillsz = tls;
                if(rq.out_bufs_static_size > 0) *tls = ((rq.out_bufs_static_size + 2) << 10);
                else *tls = 0;
                if(rq.in_pid || (rq.in_copy_hs_size > 0) || (rq.in_move_hs_size > 0))
                {
                    *tls++ |= 0x80000000;
                    *tls++ = ((!!rq.in_pid) | (rq.in_copy_hs_size << 1) | (rq.in_move_hs_size << 5));
                    if(rq.in_pid) tls += 2;
                    if(rq.in_copy_hs_size > 0) for(u32 i = 0; i < rq.in_copy_hs_size; i++) *tls++ = rq.in_copy_hs[i];
                    if(rq.in_move_hs_size > 0) for(u32 i = 0; i < rq.in_move_hs_size; i++) *tls++ = rq.in_move_hs[i];
                }
                else tls++;
                if(rq.in_bufs_static_size > 0) for(u32 i = 0; i < rq.in_bufs_static_size; i++, tls += 2)
                {
                    Buffer ins = rq.in_bufs_static[i];
                    BufferSendData *bsd = (BufferSendData*)tls;
                    uintptr_t uptr = (uintptr_t)ins.data;
                    bsd->address = uptr;
                    bsd->packed = (ins.info.index | (ins.size << 16) | (((uptr >> 32) & 15) << 12) | (((uptr >> 36) & 15) << 6));
                }
                if(rq.in_bufs_size > 0) for(u32 i = 0; i < rq.in_bufs_size; i++, tls += 3)
                {
                    Buffer in = rq.in_bufs[i];
                    BufferCommandData *bcd = (BufferCommandData*)tls;
                    bcd->size = in.size;
                    uintptr_t uptr = (uintptr_t)in.data;
                    bcd->address = uptr;
                    bcd->packed = (in.info.type | (((uptr >> 32) & 15) << 28) | ((uptr >> 36) << 2));
                }
                if(rq.out_bufs_size > 0) for(u32 i = 0; i < rq.out_bufs_size; i++, tls += 3)
                {
                    Buffer out = rq.out_bufs[i];
                    BufferCommandData *bcd = (BufferCommandData*)tls;
                    bcd->size = out.size;
                    uintptr_t uptr = (uintptr_t)out.data;
                    bcd->address = uptr;
                    bcd->packed = (out.info.type | (((uptr >> 32) & 15) << 28) | ((uptr >> 36) << 2));
                }
                if(rq.out_bufs_exch_size > 0) for(u32 i = 0; i < rq.out_bufs_exch_size; i++, tls += 3)
                {
                    Buffer ex = rq.out_bufs_exch[i];
                    BufferCommandData *bcd = (BufferCommandData*)tls;
                    bcd->size = ex.size;
                    uintptr_t uptr = (uintptr_t)ex.data;
                    bcd->address = uptr;
                    bcd->packed = (ex.info.type | (((uptr >> 32) & 15) << 28) | ((uptr >> 36) << 2));
                }
                u32 pad = (((16 - (((uintptr_t)tls) & 15)) & 15) / 4);
                u32 *raw = (u32*)(tls + pad);
                size_t rawsz = ((rq.in_raw_size / 4) + 4);
                tls += rawsz;
                u16 *tls16 = (u16*)tls;
                if(rq.out_bufs_static_size > 0) for(u32 i = 0; i < rq.out_bufs_static_size; i++)
                {
                    Buffer outs = rq.out_bufs_static[i];
                    size_t outssz = (uintptr_t)outs.size;
                    tls16[i] = ((outssz > 0xffff) ? 0 : outssz);
                }
                size_t u16s = (((2 * rq.out_bufs_static_size) + 3) / 4);
                tls += u16s;
                rawsz += u16s;
                *fillsz |= rawsz;
                if(rq.out_bufs_static_size > 0) for(u32 i = 0; i < rq.out_bufs_static_size; i++, tls += 2)
                {
                    Buffer outs = rq.out_bufs_static[i];
                    BufferReceiveData *brd = (BufferReceiveData*)tls;
                    uintptr_t uptr = (uintptr_t)outs.data;
                    brd->address = uptr;
                    brd->packed = ((uptr >> 32) | (outs.size << 16));
                }
                void *vraw = (void*)raw;
                if(domainmode)
                {
                    DomainHeader *dh = (DomainHeader*)vraw;
                    u32 *ooids = (u32*)(((uintptr_t)vraw) + sizeof(DomainHeader) + orawsz);
                    dh->type = 1;
                    dh->object_id_count = (u8)rq.in_object_ids_size;
                    dh->size = orawsz;
                    dh->object_id = GetObjectId();
                    dh->pad[0] = dh->pad[1] = 0;
                    if(rq.in_object_ids_size > 0) for(u32 i = 0; i < rq.in_object_ids_size; i++) ooids[i] = rq.in_object_ids[i];
                    vraw = (void*)(((uintptr_t)vraw) + sizeof(DomainHeader));
                }
                rq.in_raw = vraw;
                *((u64*)(((u8*)rq.in_raw) + magicoff)) = SFCI;
                *((u64*)(((u8*)rq.in_raw) + cmdidoff)) = CommandId;
                ProcessArgument(rq, 2, Args...);
                Result rc = svc::SendSyncRequest(handle);
                if(rc.IsFailure()) return rc;
                u32 *otls = (u32*)os::GetTLS();
                rq.out_raw_size += (alignof(u64) - 1);
                rq.out_raw_size -= (rq.out_raw_size % alignof(u64));
                rq.out_raw_size += sizeof(u64);
                rq.out_raw_size += (alignof(u64) - 1);
                rq.out_raw_size -= (rq.out_raw_size % alignof(u64));
                u64 resoff = rq.out_raw_size;
                rq.out_raw_size += sizeof(u64);
                ProcessArgument(rq, 3, Args...);
                u32 ctrl0 = *otls++;
                u32 ctrl1 = *otls++;
                if(ctrl1 & 0x80000000)
                {
                    u32 ctrl2 = *otls++;
                    if(ctrl2 & 1)
                    {
                        u64 pid = *otls++;
                        pid |= (((u64)(*otls++)) << 32);
                        rq.out_pid = pid;
                    }
                    size_t ohcopy = ((ctrl2 >> 1) & 15);
                    size_t ohmove = ((ctrl2 >> 5) & 15);
                    size_t oh = (ohcopy + ohmove);
                    u32 *aftoh = (otls + oh);
                    if(oh > 8) oh = 8;
                    if(oh > 0)
                    {
                        for(size_t i = 0; i < oh; i++)
                        {
                            u32 hdl = *(otls + i);
                            rq.out_hs[rq.out_hs_size] = hdl;
                            rq.out_hs_size++;
                        }
                    }
                    otls = aftoh;
                }
                ProcessArgument(rq, 4, Args...);
                size_t nst = ((ctrl0 >> 16) & 15);
                u32 *aftst = otls + (nst * 2);
                if(nst > 8) nst = 8;
                if(nst > 0) for(u32 i = 0; i < nst; i++, otls += 2)
                {
                    BufferSendData *bsd = (BufferSendData*)otls;
                    BIO_IGNORE(bsd);
                }
                otls = aftst;
                size_t bsend = ((ctrl0 >> 20) & 15);
                size_t brecv = ((ctrl0 >> 24) & 15);
                size_t bexch = ((ctrl0 >> 28) & 15);
                size_t bnum = bsend + brecv + bexch;
                void *ovraw = (void*)(((uintptr_t)(otls + (bnum * 3)) + 15) &~ 15);
                if(bnum > 8) bnum = 8;
                if(bnum > 0) for(u32 i = 0; i < bnum; i++, otls += 3)
                {
                    BufferCommandData *bcd = (BufferCommandData*)otls;
                    BIO_IGNORE(bcd);
                }       
                if(domainmode)
                {
                    DomainResponse *dr = (DomainResponse*)ovraw;
                    u32 *ooids = (u32*)(((uintptr_t)ovraw) + sizeof(DomainResponse) + rq.out_raw_size);
                    u32 ooidcount = dr->object_id_count;
                    if(ooidcount > 8) ooidcount = 8;
                    if(ooidcount > 0)
                    {
                        for(u32 i = 0; i < ooidcount; i++)
                        {
                            rq.out_object_ids[rq.out_object_ids_size] = ooids[i];
                            rq.out_object_ids_size++;
                        }
                    }
                    ovraw = (void*)(((uintptr_t)ovraw) + sizeof(DomainResponse));
                }
                rq.out_raw = (u8*)ovraw;
                ProcessArgument(rq, 5, Args...);
                rc = (u32)(*((u64*)(((u8*)rq.out_raw) + resoff)));
                return rc;
            }
        protected:
            u32 handle;
            SessionType type;
            u32 object_id;
        private:
            void ProcessArgument(RequestData &data, u8 part)
            {
                // Support empty parameter packs, aka no arguments for request!
            }
            
            template<typename Argument>
            void ProcessArgument(RequestData &data, u8 part, Argument &&Arg)
            {
                Arg.Process(data, part);
            }

            template<typename Argument, typename ...Arguments>
            void ProcessArgument(RequestData &data, u8 part, Argument &&Arg, Arguments &&...Args)
            {
                ProcessArgument(data, part, Arg);
                ProcessArgument(data, part, Args...);
            }
    };

    class ServiceSession : public Session
    {
        public:
            ServiceSession(const char *name);
            const char *GetServiceName();
        private:
            char srv_name[0x10];
    };
}