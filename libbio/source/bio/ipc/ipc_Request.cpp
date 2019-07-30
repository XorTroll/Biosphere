#include <bio/ipc/ipc_Request.hpp>

namespace bio::ipc
{
    void InProcessId::Process(RequestData &data, u8 part)
    {
        switch(part)
        {
            case 0:
                data.in_pid = true;
                break;
        }
    }

    InObjectId::InObjectId(u32 object_id)
    {
        obj_id = object_id;
    }

    void InObjectId::Process(RequestData &data, u8 part)
    {
        switch(part)
        {
            case 0:
                data.in_object_ids[data.in_object_ids_size] = obj_id;
                data.in_object_ids_size++;
                break;
        }
    }

    InBuffer::InBuffer(const void *data, size_t size, u32 type) : buf({ MakeNormal(type), data, size })
    {
    }

    void InBuffer::Process(RequestData &data, u8 part)
    {
        switch(part)
        {
            case 0:
                data.in_bufs[data.in_bufs_size] = buf;
                data.in_bufs_size++;
                break;
        }
    }

    InStaticBuffer::InStaticBuffer(const void *data, size_t size, u32 index) : buf ({ MakeStatic(index), data, size })
    {
    }

    void InStaticBuffer::Process(RequestData &data, u8 part)
    {
        switch(part)
        {
            case 0:
                data.in_bufs_static[data.in_bufs_static_size] = buf;
                data.in_bufs_static_size++;
                break;
        }
    }

    InSmartBuffer::InSmartBuffer(const void *data, size_t size, u32 index, u64 expected_size) : buf_normal({ { BufferMode::Normal, 0, 0, 0 }, data, size }), buf_static({ { BufferMode::Static, 0, index, 0 }, NULL, 0 })
    {
        if((expected_size != 0) && (size <= expected_size))
        {
            buf_normal = { { BufferMode::Normal, 0, 0, 0 }, NULL, 0 };
            buf_static = { { BufferMode::Static, 0, index, 0 }, data, size };
        }
    }

    void InSmartBuffer::Process(RequestData &data, u8 part)
    {
        switch(part)
        {
            case 0:
                data.in_bufs[data.in_bufs_size] = buf_normal;
                data.in_bufs_size++;
                data.in_bufs_static[data.in_bufs_static_size] = buf_static;
                data.in_bufs_static_size++;
                break;
        }
    }

    OutProcessId::OutProcessId(u64 &out_pid) : pid(out_pid)
    {
    }

    void OutProcessId::Process(RequestData &data, u8 part)
    {
        switch(part)
        {
            case 4:
                pid = data.out_pid;
                break;
        }
    }

    OutBuffer::OutBuffer(const void *data, size_t size, u32 type) : buf({ MakeNormal(type), data, size })
    {
    }

    void OutBuffer::Process(RequestData &data, u8 part)
    {
        switch(part)
        {
            case 0:
                data.out_bufs[data.out_bufs_size] = buf;
                data.out_bufs_size++;
                break;
        }
    }

    OutStaticBuffer::OutStaticBuffer(const void *data, size_t size, u32 index) : buf({ MakeStatic(index), data, size })
    {
    }

    void OutStaticBuffer::Process(RequestData &data, u8 part)
    {
        switch(part)
        {
            case 0:
                data.out_bufs_static[data.out_bufs_static_size] = buf;
                data.out_bufs_static_size++;
                break;
        }
    }

    OutSmartBuffer::OutSmartBuffer(const void *data, size_t size, u32 index, u64 expected_size) : buf_normal({ { BufferMode::Normal, 0, 0, 0 }, data, size }), buf_static({ { BufferMode::Static, 0, index, 0 }, NULL, 0 })
    {
        if((expected_size != 0) && (size <= expected_size))
        {
            buf_normal = { { BufferMode::Normal, 0, 0, 0 }, NULL, 0 };
            buf_static = { { BufferMode::Static, 0, index, 0 }, data, size };
        }
    }

    void OutSmartBuffer::Process(RequestData &data, u8 part)
    {
        switch(part)
        {
            case 0:
                data.out_bufs[data.out_bufs_size] = buf_normal;
                data.out_bufs_size++;
                data.out_bufs_static[data.out_bufs_static_size] = buf_static;
                data.out_bufs_static_size++;
                break;
        }
    }
}