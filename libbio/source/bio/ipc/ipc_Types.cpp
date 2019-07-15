#include <bio/ipc/ipc_Types.hpp>
#include <cstdlib>

namespace bio::ipc
{
    BufferInfo MakeNormal(u32 type)
    {
        return { BufferMode::Normal, type, 0, 0 };
    }

    BufferInfo MakeStatic(u32 index)
    {
        return { BufferMode::Static, 0, index, 0 };
    }

    BufferInfo MakeSmart(size_t buf_size, u32 index)
    {
        return { BufferMode::Smart, 0, index, buf_size };
    }
}