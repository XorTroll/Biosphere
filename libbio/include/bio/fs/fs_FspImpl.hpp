
#pragma once
#include <bio/fs/fs_Device.hpp>

namespace bio::fs
{
    class FileSystemDeviceDirectory final : public DeviceDirectory
    {
        public:
            FileSystemDeviceDirectory(std::shared_ptr<fsp::Directory> &dir);
            ~FileSystemDeviceDirectory();
            virtual Result Next(Out<fsp::DirectoryEntry> out) override;
        private:
            std::shared_ptr<fsp::Directory> idir;
            fsp::DirectoryEntry *entries_block;
            u64 entry_count;
            int entry_index;
    };

    class FileSystemDeviceFile final : public DeviceFile
    {
        public:
            FileSystemDeviceFile(std::shared_ptr<fsp::File> &file);
            virtual Result Read(void *ptr, size_t size, Out<u64> written) override;
            virtual Result Write(const void *ptr, size_t size) override;
            virtual Result Seek(int pos, int whence, Out<off_t> off) override;
            virtual Result Flush() override;
            
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

    Result MountFileSystem(std::shared_ptr<fsp::FileSystem> &fs, const char *name);
    Result MountSdCard(const char *name);
    Result MountRom(const char *name);
}