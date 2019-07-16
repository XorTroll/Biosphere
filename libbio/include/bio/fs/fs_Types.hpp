
#pragma once
#include <bio/bio_Types.hpp>
#include <bio/fsp/fsp_Service.hpp>
#include <sys/stat.h>

namespace bio::fs
{
    class Device
    {
        public:
            Device(const char *mount);
            virtual Result CreateDirectory(const char *path, u32 mode) = 0;
            virtual Result RemoveDirectory(const char *path) = 0;
            virtual Result Stat(const char *path, struct stat *out) = 0;

            const char *GetMount();
        private:
            char dev_mount[fsp::PathMax];
    };

    class FileSystemDevice final : public Device
    {
        public:
            FileSystemDevice(const char *mount, std::shared_ptr<fsp::FileSystem> &fs);
            virtual Result CreateDirectory(const char *path, u32 mode) override;
            virtual Result RemoveDirectory(const char *path) override;
            virtual Result Stat(const char *path, struct stat *out) override;
        private:
            std::shared_ptr<fsp::FileSystem> ifs;
    };

    Result Initialize();
    bool IsInitialized();
    void Exit();
    std::shared_ptr<fsp::Service> &GetFsSession();

    Result Mount(std::shared_ptr<Device> &device);
    Result MountFileSystem(std::shared_ptr<fsp::FileSystem> &fs, const char *name);
    void Unmount(const char *mount);

    Result MountSdCard(const char *name);
}