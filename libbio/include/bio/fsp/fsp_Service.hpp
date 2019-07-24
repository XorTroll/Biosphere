
#pragma once
#include <bio/ipc/ipc_Request.hpp>
#include <bio/fsp/fsp_Types.hpp>

namespace bio::fsp
{
    class File : public ipc::Session
    {
        public:
            using Session::Session;
            Result Read(u64 offset, void *buf, size_t size, Out<u64> read);
            Result Write(u64 offset, const void *buf, size_t size);
            Result Flush();
            Result SetSize(u64 Size);
            Result GetSize(Out<u64> size);
    };

    class Directory : public ipc::Session
    {
        public:
            using Session::Session;
            Result Read(DirectoryEntry *entries, size_t max_count, Out<u64> count);
            Result GetEntryCount(Out<u64> count);
    };

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

            Result OpenFile(u32 mode, const char *path, Out<std::shared_ptr<File>> out_file);
            Result OpenDirectory(u32 filter, const char *path, Out<std::shared_ptr<Directory>> out_dir);

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
            
            Result OpenDataFileSystemByCurrentProcess(Out<std::shared_ptr<FileSystem>> fs);
            Result OpenSdCardFileSystem(Out<std::shared_ptr<FileSystem>> fs);
    };
}