
#pragma once
#include <bio/svc/svc_Base.hpp>
#include <bio/os/os_TLS.hpp>
#include <memory>

namespace bio::ipc
{
    #define BIO_IPC_PROCESS_TYPE_RAW(type, var) var += (alignof(type) - 1); \
        var -= (var % alignof(type)); \
        var += sizeof(type);

    #define BIO_IPC_PROCESS_TYPE_RAW_OUT(type, var, out) var += (alignof(type) - 1); \
        var -= (var % alignof(type)); \
        out = var; \
        var += sizeof(type);


    enum class SessionType
    {
        Invalid,
        Normal,
        Domain,
        DomainSubService,
    };

    enum class HandleMode
    {
        Copy,
        Move
    };

    enum class BufferMode
    {
        Normal,
        Static,
        Smart,
    };

    struct BufferInfo
    {
        BufferMode mode;
        u32 type;
        u32 index;
        size_t buffer_size;
    };

    struct Buffer
    {
        BufferInfo info;
        const void *data;
        size_t size;
    };

    struct BufferCommandData
    {
        u32 size;
        u32 address;
        u32 packed;
    };

    struct BufferReceiveData
    {
        u32 address;
        u32 packed;
    };

    struct BufferSendData
    {
        u32 packed;
        u32 address;
    };

    struct DomainHeader
    {
        u8 type;
        u8 object_id_count;
        u16 size;
        u32 object_id;
        u32 pad[2];
    };

    struct DomainResponse
    {
        u32 object_id_count;
        u32 pad[3];
    };

    class Session;

    struct RequestData
    {
        void *in_raw;
        u64 in_raw_size;
        u32 in_copy_hs[8];
        u32 in_copy_hs_size;
        u32 in_move_hs[8];
        u32 in_move_hs_size;
        u32 in_object_ids[8];
        u32 in_object_ids_size;
        Buffer in_bufs[8];
        u32 in_bufs_size;
        Buffer in_bufs_static[8];
        u32 in_bufs_static_size;
        bool in_pid;
        u64 out_pid;
        u32 out_hs[8];
        u32 out_hs_size;
        u32 out_object_ids[8];
        u32 out_object_ids_size;
        void *out_raw;
        u64 out_raw_size;
        Buffer out_bufs[8];
        u32 out_bufs_size;
        Buffer out_bufs_static[8];
        u32 out_bufs_static_size;
        Buffer out_bufs_exch[8];
        u32 out_bufs_exch_size;
        
        u32 requester_session_handle;
        u32 requester_session_object_id;
        bool requester_session_is_domain;
    };

    class RequestArgument
    {
        public:
            virtual void Process(RequestData &data, u8 part) = 0;
    };

    BufferInfo MakeNormal(u32 type);
    BufferInfo MakeStatic(u32 index);
    BufferInfo MakeSmart(size_t buf_size, u32 index);

    static constexpr u32 SFCI = 0x49434653;
    static constexpr u32 SFCO = 0x4F434653;

    inline void PrepareCommandHeader(RequestData &data)
    {
        u64 orawsz = 0;
        u32 *tls = (u32*)os::GetTLS();
        if(data.requester_session_is_domain)
        {
            orawsz = data.in_raw_size;
            data.in_raw_size += sizeof(DomainHeader) + (data.in_object_ids_size * sizeof(u32));
        }
        *tls++ = (4 | (data.in_bufs_static_size << 16) | (data.in_bufs_size << 20) | (data.out_bufs_size << 24) | (data.out_bufs_exch_size << 28));
        u32 *fillsz = tls;
        if(data.out_bufs_static_size > 0) *tls = ((data.out_bufs_static_size + 2) << 10);
        else *tls = 0;
        if(data.in_pid || (data.in_copy_hs_size > 0) || (data.in_move_hs_size > 0))
        {
            *tls++ |= 0x80000000;
            *tls++ = ((!!data.in_pid) | (data.in_copy_hs_size << 1) | (data.in_move_hs_size << 5));
            if(data.in_pid) tls += 2;
            if(data.in_copy_hs_size > 0) for(u32 i = 0; i < data.in_copy_hs_size; i++) *tls++ = data.in_copy_hs[i];
            if(data.in_move_hs_size > 0) for(u32 i = 0; i < data.in_move_hs_size; i++) *tls++ = data.in_move_hs[i];
        }
        else tls++;
        if(data.in_bufs_static_size > 0) for(u32 i = 0; i < data.in_bufs_static_size; i++, tls += 2)
        {
            Buffer ins = data.in_bufs_static[i];
            BufferSendData *bsd = (BufferSendData*)tls;
            uintptr_t uptr = (uintptr_t)ins.data;
            bsd->address = uptr;
            bsd->packed = (ins.info.index | (ins.size << 16) | (((uptr >> 32) & 15) << 12) | (((uptr >> 36) & 15) << 6));
        }
        if(data.in_bufs_size > 0) for(u32 i = 0; i < data.in_bufs_size; i++, tls += 3)
        {
            Buffer in = data.in_bufs[i];
            BufferCommandData *bcd = (BufferCommandData*)tls;
            bcd->size = in.size;
            uintptr_t uptr = (uintptr_t)in.data;
            bcd->address = uptr;
            bcd->packed = (in.info.type | (((uptr >> 32) & 15) << 28) | ((uptr >> 36) << 2));
        }
        if(data.out_bufs_size > 0) for(u32 i = 0; i < data.out_bufs_size; i++, tls += 3)
        {
            Buffer out = data.out_bufs[i];
            BufferCommandData *bcd = (BufferCommandData*)tls;
            bcd->size = out.size;
            uintptr_t uptr = (uintptr_t)out.data;
            bcd->address = uptr;
            bcd->packed = (out.info.type | (((uptr >> 32) & 15) << 28) | ((uptr >> 36) << 2));
        }
        if(data.out_bufs_exch_size > 0) for(u32 i = 0; i < data.out_bufs_exch_size; i++, tls += 3)
        {
            Buffer ex = data.out_bufs_exch[i];
            BufferCommandData *bcd = (BufferCommandData*)tls;
            bcd->size = ex.size;
            uintptr_t uptr = (uintptr_t)ex.data;
            bcd->address = uptr;
            bcd->packed = (ex.info.type | (((uptr >> 32) & 15) << 28) | ((uptr >> 36) << 2));
        }
        u32 pad = (((16 - (((uintptr_t)tls) & 15)) & 15) / 4);
        u32 *raw = (u32*)(tls + pad);
        size_t rawsz = ((data.in_raw_size / 4) + 4);
        tls += rawsz;
        u16 *tls16 = (u16*)tls;
        if(data.out_bufs_static_size > 0) for(u32 i = 0; i < data.out_bufs_static_size; i++)
        {
            Buffer outs = data.out_bufs_static[i];
            size_t outssz = (uintptr_t)outs.size;
            tls16[i] = ((outssz > 0xffff) ? 0 : outssz);
        }
        size_t u16s = (((2 * data.out_bufs_static_size) + 3) / 4);
        tls += u16s;
        rawsz += u16s;
        *fillsz |= rawsz;
        if(data.out_bufs_static_size > 0) for(u32 i = 0; i < data.out_bufs_static_size; i++, tls += 2)
        {
            Buffer outs = data.out_bufs_static[i];
            BufferReceiveData *brd = (BufferReceiveData*)tls;
            uintptr_t uptr = (uintptr_t)outs.data;
            brd->address = uptr;
            brd->packed = ((uptr >> 32) | (outs.size << 16));
        }
        void *vraw = (void*)raw;
        if(data.requester_session_is_domain)
        {
            DomainHeader *dh = (DomainHeader*)vraw;
            u32 *ooids = (u32*)(((uintptr_t)vraw) + sizeof(DomainHeader) + orawsz);
            dh->type = 1;
            dh->object_id_count = (u8)data.in_object_ids_size;
            dh->size = orawsz;
            dh->object_id = data.requester_session_object_id;
            dh->pad[0] = dh->pad[1] = 0;
            if(data.in_object_ids_size > 0) for(u32 i = 0; i < data.in_object_ids_size; i++) ooids[i] = data.in_object_ids[i];
            vraw = (void*)(((uintptr_t)vraw) + sizeof(DomainHeader));
        }
        data.in_raw = vraw;
    }

    inline void ProcessResponse(RequestData &data)
    {
        u32 *otls = (u32*)os::GetTLS();
        u32 ctrl0 = *otls++;
        u32 ctrl1 = *otls++;
        if(ctrl1 & 0x80000000)
        {
            u32 ctrl2 = *otls++;
            if(ctrl2 & 1)
            {
                u64 pid = *otls++;
                pid |= (((u64)(*otls++)) << 32);
                data.out_pid = pid;
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
                    data.out_hs[data.out_hs_size] = hdl;
                    data.out_hs_size++;
                }
            }
            otls = aftoh;
        }
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
        if(data.requester_session_is_domain)
        {
            DomainResponse *dr = (DomainResponse*)ovraw;
            u32 *ooids = (u32*)(((uintptr_t)ovraw) + sizeof(DomainResponse) + data.out_raw_size);
            u32 ooidcount = dr->object_id_count;
            if(ooidcount > 8) ooidcount = 8;
            if(ooidcount > 0)
            {
                for(u32 i = 0; i < ooidcount; i++)
                {
                    data.out_object_ids[data.out_object_ids_size] = ooids[i];
                    data.out_object_ids_size++;
                }
            }
            ovraw = (void*)(((uintptr_t)ovraw) + sizeof(DomainResponse));
        }
        data.out_raw = ovraw;
    }

    void CloseSessionHandle(u32 handle);
}