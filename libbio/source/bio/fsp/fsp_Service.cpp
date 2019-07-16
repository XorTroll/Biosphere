#include <bio/fsp/fsp_Service.hpp>
#include <cstring>

namespace bio::fsp
{
    #define MAKE_PATH_COPY(pp) char send_##pp[PathMax] = {0}; strncpy(send_##pp, pp, PathMax - 1);

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
        u32 detype = 0;
        auto res = ProcessRequest<7>(ipc::InStaticBuffer(send_path, PathMax, 0), ipc::OutRaw<u32>(detype));
        if(res.IsSuccess())
        {
            DirectoryEntryType dtype = static_cast<DirectoryEntryType>(dtype);
            type.Set(dtype);
        }
        return res;
    }

    /*
    ResultWrap<File*> FileSystem::OpenFile(u32 Mode, const char *path)
    {
        u32 fh = 0;
        Result rc = ProcessRequest<8>(ipc::InRaw<u32>(Mode), ipc::InStaticBuffer((char*)path, PathMax, 0), ipc::OutHandle<0>(fh));
        return ResultWrap<File*>(rc, new File(fh));
    }

    ResultWrap<Directory*> FileSystem::OpenDirectory(u32 Filter, const char *path)
    {
        u32 dh = 0;
        Result rc = ProcessRequest<9>(ipc::InRaw<u32>(Filter), ipc::InStaticBuffer((char*)path, PathMax, 0), ipc::OutHandle<0>(dh));
        return ResultWrap<Directory*>(rc, new Directory(dh));
    }
    */

    Result FileSystem::Commit()
    {
        return ProcessRequest<10>(ipc::Simple());
    }

    Result FileSystem::GetFreeSpaceSize(const char *path, Out<u64> size)
    {
        MAKE_PATH_COPY(path);
        u64 sz = 0;
        auto res = ProcessRequest<11>(ipc::InStaticBuffer(send_path, PathMax, 0), ipc::OutRaw<u64>(sz));
        if(res.IsSuccess()) size.Set(sz);
        return res;
    }

    Result FileSystem::GetTotalSpaceSize(const char *path, Out<u64> size)
    {
        MAKE_PATH_COPY(path);
        u64 sz = 0;
        auto res = ProcessRequest<12>(ipc::InStaticBuffer(send_path, PathMax, 0), ipc::OutRaw<u64>(sz));
        if(res.IsSuccess()) size.Set(sz);
        return res;
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
        if(res.IsSuccess())
        {
            auto shfs = std::make_shared<FileSystem>(*this, obj_id);
            fs.Set(shfs);
        }
        return res;
    }

    Service::Service() : ServiceSession("fsp-srv")
    {
        ProcessRequest<1>(ipc::InProcessId(), ipc::InRaw<u64>(0)).Assert();
        ConvertToDomain().Assert();
    }
}