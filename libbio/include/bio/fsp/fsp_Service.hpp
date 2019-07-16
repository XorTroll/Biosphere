
#pragma once
#include <bio/ipc/ipc_Request.hpp>
#include <bio/fsp/fsp_Types.hpp>

namespace bio::fsp
{
    class FileSystem : public ipc::Session
    {
        public:
            using Session::Session;
            Result CreateFile(u32 flags, u64 size, const char *path);
            Result DeleteFile(const char *path);
            Result CreateDirectory(const char *path);
            Result DeleteDirectory(const char *path);
            Result DeleteDirectoryRecursively(const char *path);
            Result RenameFile(const char *path, const char *new_path);
            Result RenameDirectory(const char *path, const char *new_path);
            Result GetEntryType(const char *Path, Out<DirectoryEntryType> type);
            /*
            ResultWrap<File*> OpenFile(u32 Mode, const char *Path);
            ResultWrap<Directory*> OpenDirectory(u32 Filter, const char *Path);
            */
            Result Commit();
            Result GetFreeSpaceSize(const char *path, Out<u64> size);
            Result GetTotalSpaceSize(const char *path, Out<u64> size);
            Result CleanDirectoryRecursively(const char *path);
    };

    class Service : public ipc::ServiceSession
    {
        public:
            Service();
            static std::shared_ptr<Service> Initialize();
            
            Result OpenSdCardFileSystem(Out<std::shared_ptr<FileSystem>> fs);
        private:
    };
}