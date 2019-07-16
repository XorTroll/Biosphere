
#pragma once
#include <bio/ipc/ipc_Session.hpp>

namespace bio::ipc
{
    struct Simple : RequestArgument
    {
        void Process(RequestData &data, u8 part) override;
    };

    template<typename T>
    struct InRaw : RequestArgument
    {
        T value;
        u64 offset;

        InRaw(T type) : value(type), offset(0)
        {
        }

        void Process(RequestData &data, u8 part) override
        {
            switch(part)
            {
                case 0:
                    data.in_raw_size += (alignof(T) - 1);
                    data.in_raw_size -= (data.in_raw_size % alignof(T));
                    offset = data.in_raw_size;
                    data.in_raw_size += sizeof(T);
                    break;
                case 2:
                    *((T*)(((u8*)data.in_raw) + offset)) = value;
                    break;
            }
        }
    };

    struct InProcessId : RequestArgument
    {
        void Process(RequestData &data, u8 part) override;
    };

    template<HandleMode HMode>
    struct InHandle : RequestArgument
    {
        u32 handle;
        HandleMode mode;

        InHandle(KObject in_handle) : handle(in_handle.handle), mode(HMode)
        {
        }

        void Process(RequestData &data, u8 part) override
        {
            switch(part)
            {
                case 0:
                    switch(mode)
                    {
                        case HandleMode::Copy:
                            data.in_copy_hs[data.in_copy_hs_size] = handle;
                            data.in_copy_hs_size++;
                            break;
                        case HandleMode::Move:
                            data.in_move_hs[data.in_move_hs_size] = handle;
                            data.in_move_hs_size++;
                            break;
                    }
                    break;
            }
        }
    };

    struct InObjectId : RequestArgument
    {
        u32 obj_id;

        InObjectId(u32 object_id);
        void Process(RequestData &data, u8 part) override;
    };

    struct InBuffer : RequestArgument
    {
        Buffer buf;

        InBuffer(const void *data, size_t size, u32 type);
        void Process(RequestData &data, u8 part) override;
    };

    struct InStaticBuffer : RequestArgument
    {
        Buffer buf;

        InStaticBuffer(const void *data, size_t Size, u32 Index);
        void Process(RequestData &data, u8 part) override;
    };

    struct InSmartBuffer : RequestArgument
    {
        Buffer buf_normal;
        Buffer buf_static;

        InSmartBuffer(const void *data, size_t size, u32 index, u64 expected_size);
        void Process(RequestData &data, u8 part) override;
    };

    template<typename T>
    struct OutRaw : RequestArgument
    {
        T &value;
        u64 offset;

        OutRaw(T &ref) : value(ref), offset(0)
        {
        }

        void Process(RequestData &data, u8 part) override
        {
            switch(part)
            {
                case 3:
                    data.out_raw_size += (alignof(T) - 1);
                    data.out_raw_size -= (data.out_raw_size % alignof(T));
                    offset = data.out_raw_size;
                    data.out_raw_size += sizeof(T);
                    break;
                case 5:
                    value = *((T*)(((u8*)data.out_raw) + offset));
                    break;
            }
        }
    };

    struct OutProcessId : RequestArgument
    {
        u64 &pid;

        OutProcessId(u64 &out_pid);
        void Process(RequestData &data, u8 part) override;
    };

    template<u32 OIndex>
    struct OutHandle : RequestArgument
    {
        u32 idx;
        KObject &handle;

        OutHandle(KObject &out_h) : handle(out_h), idx(OIndex)
        {
        }

        void Process(RequestData &data, u8 part) override
        {
            switch(part)
            {
                case 4:
                    if(idx < data.out_hs_size) handle = KObject(data.out_hs[idx]);
                    break;
            }
        }
    };

    template<u32 OIndex>
    struct OutObjectId : RequestArgument
    {
        u32 idx;
        u32 &obj_id;

        OutObjectId(u32 &out_obj_id) : obj_id(out_obj_id), idx(0)
        {
        }

        void Process(RequestData &data, u8 part) override
        {
            switch(part)
            {
                case 5:
                    if(idx < data.out_object_ids_size) obj_id = data.out_object_ids[idx];
                    break;
            }
        }
    };

    template<u32 OIndex>
    struct OutSession : RequestArgument
    {
        u32 idx;
        Session &s;

        OutSession(Session &session) : s(session), idx(OIndex)
        {
        }

        void Process(RequestData &data, u8 part) override
        {
            switch(part)
            {
                case 4:
                    if(idx < data.out_hs_size) s = Session(data.out_hs[idx]);
                    break;
            }
        }
    };

    struct OutBuffer : RequestArgument
    {
        Buffer buf;

        OutBuffer(const void *data, size_t size, u32 type);
        void Process(RequestData &data, u8 part) override;
    };

    struct OutStaticBuffer : RequestArgument
    {
        Buffer buf;

        OutStaticBuffer(const void *data, size_t size, u32 index);
        void Process(RequestData &data, u8 part) override;
    };

    struct OutSmartBuffer : RequestArgument
    {
        Buffer buf_normal;
        Buffer buf_static;

        OutSmartBuffer(const void *data, size_t size, u32 index, u64 expected_size);
        void Process(RequestData &data, u8 part) override;
    };
}