
#pragma once
#include <bio/bio_Types.hpp>
#include <bio/fsp/fsp_Service.hpp>
#include <bio/fs/fs_Results.hpp>
#include <sys/stat.h>
#include <sys/dirent.h>

namespace bio::fs
{
    class DeviceDirectory
    {
        public:
            virtual Result Next(Out<fsp::DirectoryEntry> out) = 0;
    };

    class DeviceFile
    {
        public:
            virtual Result Read(void *ptr, size_t size, Out<u64> written) = 0;
            virtual Result Write(const void *ptr, size_t size) = 0;
            virtual Result Seek(int pos, int whence, Out<off_t> off) = 0;
            virtual Result Flush() = 0;
    };

    class Device
    {
        public:
            Device(const char *mount);
            virtual Result CreateDirectory(const char *path, u32 mode) = 0;
            virtual Result RemoveDirectory(const char *path) = 0;
            virtual Result Stat(const char *path, Out<struct stat> out_stat) = 0;
            virtual Result OpenDirectory(const char *path, Out<std::shared_ptr<DeviceDirectory>> out) = 0;
            virtual Result OpenFile(const char *path, int flags, int mode, Out<std::shared_ptr<DeviceFile>> out) = 0;

            const char *GetMount();
        private:
            char dev_mount[fsp::PathMax];
    };

    Result Mount(std::shared_ptr<Device> &device);
    void Unmount(const char *mount);
    void UnmountAll();
}

extern "C"
{
    struct DIR // DIR struct is declared in here!
    {
        std::shared_ptr<bio::fs::DeviceDirectory> devdir;
        struct dirent ent;
    };
}