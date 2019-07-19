
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

    class FileSystemDeviceDirectory final : public DeviceDirectory
    {
        public:
            FileSystemDeviceDirectory(std::shared_ptr<fsp::Directory> &dir);
            virtual Result Next(Out<fsp::DirectoryEntry> out) override;
        private:
            std::shared_ptr<fsp::Directory> idir;
            int entryread_count;
    };

    class FileSystemDeviceFile final : public DeviceFile
    {
        public:
            FileSystemDeviceFile(std::shared_ptr<fsp::File> &file);
            virtual Result Read(void *ptr, size_t size, Out<u64> written) override;
            virtual Result Write(const void *ptr, size_t size) override;
            virtual Result Seek(int pos, int whence, Out<off_t> off) override;
            
        private:
            std::shared_ptr<fsp::File> ifile;
            u64 offset;
    };

    class FileSystemDevice final : public Device
    {
        public:
            FileSystemDevice(const char *mount, std::shared_ptr<fsp::FileSystem> &fs);
            virtual Result CreateDirectory(const char *path, u32 mode) override;
            virtual Result RemoveDirectory(const char *path) override;
            virtual Result Stat(const char *path, Out<struct stat> out_stat) override;
            virtual Result OpenDirectory(const char *path, Out<std::shared_ptr<DeviceDirectory>> out) override;
            virtual Result OpenFile(const char *path, int flags, int mode, Out<std::shared_ptr<DeviceFile>> out) override;
        private:
            std::shared_ptr<fsp::FileSystem> ifs;
    };

    Result Initialize();
    bool IsInitialized();
    void Finalize();

    std::shared_ptr<fsp::Service> &GetFsSession();

    Result Mount(std::shared_ptr<Device> &device);
    Result MountFileSystem(std::shared_ptr<fsp::FileSystem> &fs, const char *name);
    void Unmount(const char *mount);
    void UnmountAll();

    Result MountSdCard(const char *name);
}

extern "C"
{
    struct DIR // DIR struct is declared in here!
    {
        std::shared_ptr<bio::fs::DeviceDirectory> devdir;
        struct dirent ent;
    };
}