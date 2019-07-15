#include <bio/fsp/fsp_Service.hpp>

namespace bio::fsp
{
    Result FileSystem::CreateFile(u32 Flags, u64 Size, const char *Path)
    {
        return ProcessRequest<0>(ipc::InStaticBuffer((char*)Path, 0x301, 0), ipc::InRaw<u64>(0), ipc::InRaw<u64>(Size), ipc::InRaw<u32>(Flags));
    }

    Result FileSystem::DeleteFile(const char *Path)
    {
        return ProcessRequest<1>(ipc::InStaticBuffer((char*)Path, 0x301, 0));
    }

    Result FileSystem::CreateDirectory(const char *Path)
    {
        return ProcessRequest<2>(ipc::InStaticBuffer((char*)Path, 0x301, 0));
    }

    Result FileSystem::DeleteDirectory(const char *Path)
    {
        return ProcessRequest<3>(ipc::InStaticBuffer((char*)Path, 0x301, 0));
    }

    Result FileSystem::DeleteDirectoryRecursively(const char *Path)
    {
        return ProcessRequest<4>(ipc::InStaticBuffer((char*)Path, 0x301, 0));
    }

    Result FileSystem::RenameFile(const char *Path, const char *NewPath)
    {
        return ProcessRequest<5>(ipc::InStaticBuffer((char*)Path, 0x301, 0), ipc::InStaticBuffer((char*)NewPath, 0x301, 1));
    }

    Result FileSystem::RenameDirectory(const char *Path, const char *NewPath)
    {
        return ProcessRequest<6>(ipc::InStaticBuffer((char*)Path, 0x301, 0), ipc::InStaticBuffer((char*)NewPath, 0x301, 1));
    }

    /*
    ResultWrap<DirectoryEntryType> FileSystem::GetEntryType(const char *Path)
    {
        u32 detype = 0;
        Result rc = ProcessRequest<7>(ipc::InStaticBuffer((char*)Path, 0x301, 0), ipc::OutRaw<u32>(detype));
        return ResultWrap<DirectoryEntryType>(rc, static_cast<DirectoryEntryType>(detype));
    }

    ResultWrap<File*> FileSystem::OpenFile(u32 Mode, const char *Path)
    {
        u32 fh = 0;
        Result rc = ProcessRequest<8>(ipc::InRaw<u32>(Mode), ipc::InStaticBuffer((char*)Path, 0x301, 0), ipc::OutHandle<0>(fh));
        return ResultWrap<File*>(rc, new File(fh));
    }

    ResultWrap<Directory*> FileSystem::OpenDirectory(u32 Filter, const char *Path)
    {
        u32 dh = 0;
        Result rc = ProcessRequest<9>(ipc::InRaw<u32>(Filter), ipc::InStaticBuffer((char*)Path, 0x301, 0), ipc::OutHandle<0>(dh));
        return ResultWrap<Directory*>(rc, new Directory(dh));
    }
    */

    Result FileSystem::Commit()
    {
        return ProcessRequest<10>(ipc::Simple());
    }

    Result FileSystem::GetFreeSpaceSize(const char *Path, Out<u64> size)
    {
        u64 sz = 0;
        auto res = ProcessRequest<11>(ipc::InStaticBuffer((char*)Path, 0x301, 0), ipc::OutRaw<u64>(sz));
        if(res.IsSuccess()) size.Set(sz);
        return res;
    }

    Result FileSystem::GetTotalSpaceSize(const char *Path, Out<u64> size)
    {
        u64 sz = 0;
        auto res = ProcessRequest<12>(ipc::InStaticBuffer((char*)Path, 0x301, 0), ipc::OutRaw<u64>(sz));
        if(res.IsSuccess()) size.Set(sz);
        return res;
    }

    Result FileSystem::CleanDirectoryRecursively(const char *Path)
    {
        // if(os::GetFirmwareMajorVersion() < os::FirmwareMajor::Major3) return 0;
        return ProcessRequest<13>(ipc::InStaticBuffer((char*)Path, 0x301, 0));
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