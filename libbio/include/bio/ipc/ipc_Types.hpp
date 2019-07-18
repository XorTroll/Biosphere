
#pragma once
#include <bio/svc/svc_Base.hpp>
#include <memory>

namespace bio::ipc
{
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
        bool requester_session_is_domain;
    };

    struct RequestArgument
    {
        virtual void Process(RequestData &data, u8 part) = 0;
    };

    BufferInfo MakeNormal(u32 type);
    BufferInfo MakeStatic(u32 index);
    BufferInfo MakeSmart(size_t buf_size, u32 index);

    static constexpr u32 SFCI = 0x49434653;
    static constexpr u32 SFCO = 0x4f434653;
}