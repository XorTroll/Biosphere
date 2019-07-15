
#pragma once
#include <bio/bio_Types.hpp>
#include <bio/fsp/fsp_Service.hpp>

namespace bio::fs
{
    static constexpr size_t MaxPath = 0x301;

    class Device
    {
        public:
            Device(const char *mount);
            virtual Result CreateDirectory(const char *path, u32 mode) = 0;
            const char *GetMount();
        private:
            char dev_mount[MaxPath];
    };

    class FileSystemDevice final : public Device
    {
        public:
            FileSystemDevice(const char *mount, std::shared_ptr<fsp::FileSystem> &fs);
            virtual Result CreateDirectory(const char *path, u32 mode) override;
        private:
            std::shared_ptr<fsp::FileSystem> ifs;
    };

    Result Mount(std::shared_ptr<Device> &device);
    Result MountFileSystem(std::shared_ptr<fsp::FileSystem> &fs, const char *name);
}