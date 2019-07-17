#include <bio/fsp/fsp_Service.hpp>
#include <cstring>
#include <functional>

namespace bio::fsp
{
    #define MAKE_PATH_COPY(pp) char send_##pp[PathMax] = {0}; strncpy(send_##pp, pp, PathMax - 1);

    // In File::Read/Write use inner buffers since apparently FS is restrictive and some buffers may not work properly

    Result File::Read(u64 offset, void *buf, size_t size, Out<u64> read)
    {
        u8 *tmpbuf = new u8[size];
        auto res = ProcessRequest<0>(ipc::InRaw<u64>(0), ipc::InRaw<u64>(offset), ipc::InRaw<u64>(size), ipc::OutBuffer(tmpbuf, size, 1), ipc::OutRaw<u64>(static_cast<u64&>(read)));
        if(res.IsSuccess()) memcpy(buf, tmpbuf, size);
        delete[] tmpbuf;
        return res;
    }

    Result File::Write(u64 offset, const void *buf, size_t size)
    {
        u8 *tmpbuf = new u8[size];
        memcpy(tmpbuf, buf, size);
        auto res = ProcessRequest<1>(ipc::InRaw<u64>(0), ipc::InRaw<u64>(offset), ipc::InRaw<u64>(size), ipc::InBuffer(tmpbuf, size, 1));
        delete[] tmpbuf;
        return res;
    }

    Result File::Flush()
    {
        return ProcessRequest<2>();
    }

    Result File::SetSize(u64 size)
    {
        return this->ProcessRequest<3>(ipc::InRaw<u64>(size));
    }

    Result File::GetSize(Out<u64> size)
    {
        return ProcessRequest<4>(ipc::OutRaw<u64>(static_cast<u64&>(size)));
    }

    Result Directory::Read(DirectoryEntry *entries, size_t max_count, Out<u64> count)
    {
        return ProcessRequest<0>(ipc::OutBuffer(entries, (sizeof(DirectoryEntry) * max_count), 0), ipc::InRaw<u64>(0), ipc::OutRaw<u64>(static_cast<u64&>(count)));
    }

    Result Directory::GetEntryCount(Out<u64> count)
    {
        return ProcessRequest<1>(ipc::OutRaw<u64>(static_cast<u64&>(count)));
    }

    Result FileSystem::CreateFile(u32 flags, u64 size, const char *path)
    {
        MAKE_PATH_COPY(path);
        return ProcessRequest<0>(ipc::InStaticBuffer(send_path, PathMax, 0), ipc::InRaw<u64>(0), ipc::InRaw<u64>(size), ipc::InRaw<u32>(flags));
    }

    Result FileSystem::DeleteFile(const char *path)
    {
        MAKE_PATH_COPY(path);
        return ProcessRequest<1>(ipc::InStaticBuffer(send_path, PathMax, 0));
    }

    Result FileSystem::CreateDirectory(const char *path)
    {
        MAKE_PATH_COPY(path);
        return ProcessRequest<2>(ipc::InStaticBuffer(send_path, PathMax, 0));
    }

    Result FileSystem::DeleteDirectory(const char *path)
    {
        MAKE_PATH_COPY(path);
        return ProcessRequest<3>(ipc::InStaticBuffer(send_path, PathMax, 0));
    }

    Result FileSystem::DeleteDirectoryRecursively(const char *path)
    {
        MAKE_PATH_COPY(path);
        return ProcessRequest<4>(ipc::InStaticBuffer(send_path, PathMax, 0));
    }

    Result FileSystem::RenameFile(const char *path, const char *new_path)
    {
        MAKE_PATH_COPY(path);
        MAKE_PATH_COPY(new_path);
        return ProcessRequest<5>(ipc::InStaticBuffer(send_path, PathMax, 0), ipc::InStaticBuffer(send_new_path, PathMax, 1));
    }

    Result FileSystem::RenameDirectory(const char *path, const char *new_path)
    {
        MAKE_PATH_COPY(path);
        MAKE_PATH_COPY(new_path);
        return ProcessRequest<6>(ipc::InStaticBuffer(send_path, PathMax, 0), ipc::InStaticBuffer(send_new_path, PathMax, 1));
    }

    Result FileSystem::GetEntryType(const char *path, Out<DirectoryEntryType> type)
    {
        MAKE_PATH_COPY(path);
        return ProcessRequest<7>(ipc::InStaticBuffer(send_path, PathMax, 0), ipc::OutRaw<u32>((u32&)(static_cast<DirectoryEntryType&>(type))));
    }

    Result FileSystem::OpenFile(u32 mode, const char *path, Out<std::shared_ptr<File>> out_file)
    {
        MAKE_PATH_COPY(path);
        u32 obj_id = 0;
        auto res = ProcessRequest<8>(ipc::InRaw<u32>(mode), ipc::InStaticBuffer(send_path, PathMax, 0), ipc::OutObjectId<0>(obj_id));
        if(res.IsSuccess()) (std::shared_ptr<File>&)out_file = std::make_shared<File>(*this, obj_id);
        return res;
    }

    Result FileSystem::OpenDirectory(u32 filter, const char *path, Out<std::shared_ptr<Directory>> out_dir)
    {
        MAKE_PATH_COPY(path);
        u32 obj_id = 0;
        auto res = ProcessRequest<9>(ipc::InRaw<u32>(filter), ipc::InStaticBuffer(send_path, PathMax, 0), ipc::OutObjectId<0>(obj_id));
        if(res.IsSuccess()) (std::shared_ptr<Directory>&)out_dir = std::make_shared<Directory>(*this, obj_id);
        return res;
    }

    Result FileSystem::Commit()
    {
        return ProcessRequest<10>(ipc::Simple());
    }

    Result FileSystem::GetFreeSpaceSize(const char *path, Out<u64> size)
    {
        MAKE_PATH_COPY(path);
        return ProcessRequest<11>(ipc::InStaticBuffer(send_path, PathMax, 0), ipc::OutRaw<u64>(static_cast<u64&>(size)));
    }

    Result FileSystem::GetTotalSpaceSize(const char *path, Out<u64> size)
    {
        MAKE_PATH_COPY(path);
        return ProcessRequest<12>(ipc::InStaticBuffer(send_path, PathMax, 0), ipc::OutRaw<u64>(static_cast<u64&>(size)));
    }

    Result FileSystem::CleanDirectoryRecursively(const char *path)
    {
        // if(os::GetFirmwareMajorVersion() < os::FirmwareMajor::Major3) return 0;
        MAKE_PATH_COPY(path);
        return ProcessRequest<13>(ipc::InStaticBuffer(send_path, PathMax, 0));
    }

    std::shared_ptr<Service> Service::Initialize()
    {
        return std::make_shared<Service>();
    }

    Result Service::OpenSdCardFileSystem(Out<std::shared_ptr<FileSystem>> fs)
    {
        u32 obj_id = 0;
        auto res = ProcessRequest<18>(ipc::OutObjectId<0>(obj_id));
        if(res.IsSuccess()) (std::shared_ptr<FileSystem>&)fs = std::make_shared<FileSystem>(*this, obj_id);
        return res;
    }

    Service::Service() : ServiceSession("fsp-srv")
    {
        ProcessRequest<1>(ipc::InProcessId(), ipc::InRaw<u64>(0)).Assert();
        ConvertToDomain().Assert();
    }
}