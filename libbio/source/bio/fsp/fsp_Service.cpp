#include <bio/fsp/fsp_Service.hpp>
#include <cstring>
#include <functional>

namespace bio::fsp
{
    #define MAKE_PATH_COPY(pp) char send_##pp[PathMax] = {0}; strncpy(send_##pp, pp, PathMax - 1);

    Result File::Read(u64 offset, void *buf, size_t size, Out<u64> read)
    {
        return ProcessRequest<0>(ipc::InRaw<u64>(0), ipc::InRaw<u64>(offset), ipc::InRaw<u64>(size), ipc::OutBuffer(buf, size, 1), ipc::OutRaw<u64>(static_cast<u64&>(read)));
    }

    Result File::Write(u64 offset, const void *buf, size_t size)
    {
        return ProcessRequest<1>(ipc::InRaw<u64>(0), ipc::InRaw<u64>(offset), ipc::InRaw<u64>(size), ipc::InBuffer(buf, size, 1));
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
        return ProcessRequest<8>(ipc::InRaw<u32>(mode), ipc::InStaticBuffer(send_path, PathMax, 0), ipc::OutSession<0, File>(static_cast<std::shared_ptr<File>&>(out_file)));
    }

    Result FileSystem::OpenDirectory(u32 filter, const char *path, Out<std::shared_ptr<Directory>> out_dir)
    {
        MAKE_PATH_COPY(path);
        return ProcessRequest<9>(ipc::InRaw<u32>(filter), ipc::InStaticBuffer(send_path, PathMax, 0), ipc::OutSession<0, Directory>(static_cast<std::shared_ptr<Directory>&>(out_dir)));
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

    Service::Service() : ServiceSession("fsp-srv")
    {
    }

    std::shared_ptr<Service> Service::Initialize()
    {
        auto srv = std::make_shared<Service>();
        srv->ProcessRequest<1>(ipc::InProcessId(), ipc::InRaw<u64>(0)).Assert();
        srv->ConvertToDomain().Assert();
        return srv;
    }

    Result Service::OpenDataFileSystemByCurrentProcess(Out<std::shared_ptr<FileSystem>> fs)
    {
        return ProcessRequest<2>(ipc::OutSession<0, FileSystem>(static_cast<std::shared_ptr<FileSystem>&>(fs)));
    }
    
    Result Service::OpenSdCardFileSystem(Out<std::shared_ptr<FileSystem>> fs)
    {
        return ProcessRequest<18>(ipc::OutSession<0, FileSystem>(static_cast<std::shared_ptr<FileSystem>&>(fs)));
    }
}