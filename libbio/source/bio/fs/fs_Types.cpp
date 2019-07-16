#include <bio/fs/fs_Types.hpp>
#include <cstring>
#include <errno.h>
#include <vector>
#include <memory>

namespace bio::fs
{
    static std::vector<std::shared_ptr<Device>> _inner_DeviceList;
    static std::shared_ptr<fsp::Service> _inner_FsSession;

    struct _inner_ProcessedPathBlock
    {
        bool valid;
        char mount[fsp::PathMax];
        char processed_path[fsp::PathMax];
    };

    static _inner_ProcessedPathBlock _inner_ProcessPath(const char *path)
    {
        _inner_ProcessedPathBlock processed;
        memset(&processed, 0, sizeof(processed));
        int i = 0;
        int imount = 0;
        int ipath = 0;
        while(path[i] != '\0')
        {
            if(path[i] == '/')
            {
                if(imount > 0)
                {
                    memset(&processed, 0, sizeof(processed));
                    return processed;
                }
                if(path[i + 1] != '/')
                {
                    processed.processed_path[ipath] = path[i];
                    ipath++;
                }
            }
            else if(path[i] == ':')
            {
                if(imount <= 0)
                {
                    memset(&processed, 0, sizeof(processed));
                    return processed;
                }
                else if(imount > 0)
                {
                    imount = -1;
                }
            }
            else
            {
                if(imount == -1)
                {
                    processed.processed_path[ipath] = path[i];
                    ipath++;
                }
                else
                {
                    processed.mount[imount] = path[i];
                    imount++;
                }
            }
            i++;
        }
        processed.valid = true;
        return processed;
    }

    static int _inner_ErrnoWithResult(Result res)
    {
        if(res.IsFailure())
        {
            switch(res)
            {
                case 0x202:
                    errno = ENOENT;
                    break;
                case 0x402:
                case 0x177602:
                    errno = EEXIST;
                    break;
                case 0x2ee602:
                    errno = ENAMETOOLONG;
                    break;
                case 0x2ee202:
                    errno = EINVAL;
                    break;
                case 0xe02:
                    errno = EBUSY;
                    break;
                case 0x196002:
                case 0x196202:
                case 0x1a3e02:
                case 0x1a4002:
                case 0x1a4a02:
                    errno = ENOMEM;
                    break;
                // use EIO as default
                default:
                    errno = EIO;
                    break;
            }
            return -1;
        }
        return 0;
    }

    static bool _inner_SearchDevice(std::shared_ptr<Device> &out, const char *mount)
    {
        for(u32 i = 0; i < _inner_DeviceList.size(); i++)
        {
            if(strcasecmp(mount, _inner_DeviceList[i]->GetMount()) == 0)
            {
                out = _inner_DeviceList.at(i);
                return true;
            }
        }
        return false;
    }

    Device::Device(const char *mount)
    {
        strcpy(dev_mount, mount);
    }

    const char *Device::GetMount()
    {
        return dev_mount;
    }

    FileSystemDevice::FileSystemDevice(const char *mount, std::shared_ptr<fsp::FileSystem> &fs) : Device(mount), ifs(fs)
    {
    }

    Result FileSystemDevice::CreateDirectory(const char *path, u32 mode)
    {
        BIO_IGNORE(mode);
        return ifs->CreateDirectory(path);
    }

    Result FileSystemDevice::RemoveDirectory(const char *path)
    {
        return ifs->DeleteDirectoryRecursively(path);
    }

    Result FileSystemDevice::Stat(const char *path, Out<struct stat> out_stat)
    {
        fsp::DirectoryEntryType type;
        auto res = ifs->GetEntryType(path, type);
        if(res.IsSuccess())
        {
            struct stat out;
            out.st_mode = (type == fsp::DirectoryEntryType::File) ? S_IFREG : S_IFDIR;
            memcpy(out_stat.AsPtr(), &out, sizeof(struct stat));
        }
        return res;
    }

    Result Initialize()
    {
        _inner_FsSession = fsp::Service::Initialize();
        return 0;
    }

    bool IsInitialized()
    {
        return _inner_FsSession->IsValid();
    }

    void Exit()
    {

    }

    std::shared_ptr<fsp::Service> &GetFsSession()
    {
        return _inner_FsSession;
    }

    Result Mount(std::shared_ptr<Device> &device)
    {
        for(u32 i = 0; i < _inner_DeviceList.size(); i++)
        {
            if(strcasecmp(device->GetMount(), _inner_DeviceList[i]->GetMount()) == 0) return 0x7802; // nn libs' error for already mounted with this mount name
        }
        _inner_DeviceList.push_back(device);
        return 0;
    }

    Result MountFileSystem(std::shared_ptr<fsp::FileSystem> &fs, const char *name)
    {
        std::shared_ptr<Device> dev = std::make_shared<FileSystemDevice>(name, fs);
        return Mount(dev);
    }

    void Unmount(const char *mount)
    {
        for(u32 i = 0; i < _inner_DeviceList.size(); i++)
        {
            if(strcasecmp(mount, _inner_DeviceList[i]->GetMount()) == 0)
            {
                _inner_DeviceList.erase(_inner_DeviceList.begin() + i);
                break;
            }
        }
    }

    Result MountSdCard(const char *name)
    {
        std::shared_ptr<fsp::FileSystem> sdfs;
        auto res = _inner_FsSession->OpenSdCardFileSystem(sdfs);
        if(res.IsSuccess()) res = MountFileSystem(sdfs, name);
        return res;
    }
}

extern "C"
{
    #define PROCESS_PATH bio::fs::_inner_ProcessedPathBlock ppath = bio::fs::_inner_ProcessPath(path); \
        if(!ppath.valid) { errno = ENOENT; return -1; } \
        std::shared_ptr<bio::fs::Device> dev; \
        if(!bio::fs::_inner_SearchDevice(dev, ppath.mount)) { errno = ENODEV; return -1; }

    int mkdir(const char *path, mode_t mode)
    {
        PROCESS_PATH;
        auto res = dev->CreateDirectory(ppath.processed_path, mode);
        return bio::fs::_inner_ErrnoWithResult(res);
    }

    int _stat_r(struct _reent *reent, const char *path, struct stat *st)
    {
        PROCESS_PATH;
        auto res = dev->Stat(ppath.processed_path, *st);
        return bio::fs::_inner_ErrnoWithResult(res);
    }

    int rmdir(const char *path)
    {
        PROCESS_PATH;
        auto res = dev->RemoveDirectory(ppath.processed_path);
        return bio::fs::_inner_ErrnoWithResult(res);
    }
}