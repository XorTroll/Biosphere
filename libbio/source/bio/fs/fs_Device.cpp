#include <bio/fs/fs_Device.hpp>
#include <cstring>
#include <cerrno>
#include <vector>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace bio::fs
{
    static std::vector<std::shared_ptr<Device>> _inner_DeviceList;
    static std::vector<std::shared_ptr<DeviceFile>> _inner_FileList;
    static bool _inner_Initialized = false;

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
        if((strlen(processed.mount) > 0) && (strlen(processed.processed_path) == 0))
        {
            // User wants root dir, like in "sdmc:"
            strcpy(processed.processed_path, "/");
        }
        processed.valid = true;
        return processed;
    }

    static int _inner_ReturnSetErrno(Result res)
    {
        if(res.IsSuccess()) return 0;
        errno = Result::GetErrnoFrom(res);
        return -1;
    }

    static int _inner_ReturnSetErrnoReent(Result res, struct _reent *reent)
    {
        if(res.IsSuccess()) return 0;
        reent->_errno = Result::GetErrnoFrom(res);
        return -1;
    }

    static bool _inner_SearchDevice(Out<std::shared_ptr<Device>> device, const char *mount)
    {
        for(u32 i = 0; i < _inner_DeviceList.size(); i++)
        {
            if(strcasecmp(mount, _inner_DeviceList[i]->GetMount()) == 0)
            {
                (std::shared_ptr<Device>&)device = std::ref(_inner_DeviceList.at(i));
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

    Result Mount(std::shared_ptr<Device> &device)
    {
        for(u32 i = 0; i < _inner_DeviceList.size(); i++)
        {
            if(strcasecmp(device->GetMount(), _inner_DeviceList[i]->GetMount()) == 0) return 0x7802; // nn libs' error for already mounted with this mount name
        }
        _inner_DeviceList.push_back(std::move(device));
        return 0;
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

    void UnmountAll()
    {
        _inner_DeviceList.clear();
    }
}

// newlib implementation for fs-related standard C functions

extern "C"
{
    #define PROCESS_PATH(ret) bio::fs::_inner_ProcessedPathBlock ppath = bio::fs::_inner_ProcessPath(path); \
        if(!ppath.valid) { errno = ENOENT; return ret; } \
        std::shared_ptr<bio::fs::Device> dev; \
        if(!bio::fs::_inner_SearchDevice(dev, ppath.mount)) { errno = ENODEV; return ret; }

    int mkdir(const char *path, mode_t mode)
    {
        PROCESS_PATH(-1);
        auto res = dev->CreateDirectory(ppath.processed_path, mode);
        return bio::fs::_inner_ReturnSetErrno(res);
    }

    int _stat_r(struct _reent *reent, const char *path, struct stat *st)
    {
        PROCESS_PATH(-1);
        auto res = dev->Stat(ppath.processed_path, *st);
        return bio::fs::_inner_ReturnSetErrnoReent(res, reent);
    }

    int rmdir(const char *path)
    {
        PROCESS_PATH(-1);
        auto res = dev->RemoveDirectory(ppath.processed_path);
        return bio::fs::_inner_ReturnSetErrno(res);
    }

    int _open_r(struct _reent *reent, const char *path, int flags, int mode)
    {
        PROCESS_PATH(-1);
        std::shared_ptr<bio::fs::DeviceFile> devf;
        auto res = dev->OpenFile(ppath.processed_path, flags, mode, devf);
        if(res.IsSuccess())
        {
            bio::fs::_inner_FileList.push_back(std::move(devf));
            return (bio::fs::_inner_FileList.size() - 1 + 200);
        }
        return bio::fs::_inner_ReturnSetErrnoReent(res, reent);
    }

    off_t _lseek_r(struct _reent *reent, int file, off_t pos, int whence)
    {
        int vecidx = (file - 200);
        off_t ret = 0;
        if(vecidx < bio::fs::_inner_FileList.size())
        {
            std::shared_ptr<bio::fs::DeviceFile> fd = std::ref(bio::fs::_inner_FileList.at(vecidx));
            auto res = fd->Seek(pos, whence, ret);
            if(res.IsFailure()) return bio::fs::_inner_ReturnSetErrnoReent(res, reent);
        }
        else
        {
            reent->_errno = EBADF;
            return -1;
        }
        return ret;
    }

    ssize_t _read_r(struct _reent *reent, int file, void *ptr, size_t len)
    {
        int vecidx = (file - 200);
        bio::u64 ret = 0;
        if(vecidx < bio::fs::_inner_FileList.size())
        {
            std::shared_ptr<bio::fs::DeviceFile> fd = std::ref(bio::fs::_inner_FileList.at(vecidx));
            auto res = fd->Read(ptr, len, ret);
            off_t tmp;
            if(res.IsSuccess()) res = fd->Seek(ret, SEEK_CUR, tmp);
            if(res.IsFailure()) return bio::fs::_inner_ReturnSetErrnoReent(res, reent);
        }
        else
        {
            reent->_errno = EBADF;
            return -1;
        }
        return (ssize_t)ret;   
    }

    ssize_t _write_r(struct _reent *reent, int file, const void *ptr, size_t len)
    {
        // Special case for stdout/err, handled by bio::log logging functions
        if(file == STDOUT_FILENO)
        {
            extern bio::log::LogWriteFunction global_StdoutLog;
            global_StdoutLog(ptr, len);
            return len;
        }
        else if(file == STDERR_FILENO)
        {
            extern bio::log::LogWriteFunction global_StderrLog;
            global_StderrLog(ptr, len);
            return len;
        }

        int vecidx = (file - 200);
        ssize_t ret = 0;
        if(vecidx < bio::fs::_inner_FileList.size())
        {
            std::shared_ptr<bio::fs::DeviceFile> fd = std::ref(bio::fs::_inner_FileList.at(vecidx));
            auto res = fd->Write(ptr, len);
            off_t tmp;
            if(res.IsSuccess()) res = fd->Seek(ret, SEEK_CUR, tmp);
            if(res.IsSuccess()) res = fd->Flush();
            if(res.IsFailure()) return bio::fs::_inner_ReturnSetErrnoReent(res, reent);
            ret = len;
        }
        else
        {
            reent->_errno = EBADF;
            return -1;
        }
        return ret;  
    }

    int _close_r(struct _reent *reent, int file)
    {
        int vecidx = (file - 200);
        if(vecidx < bio::fs::_inner_FileList.size())
        {
            bio::fs::_inner_FileList.erase(bio::fs::_inner_FileList.begin() + vecidx);
        }
        else
        {
            reent->_errno = EBADF;
            return -1;
        }
        return 0;
    }
}

DIR *opendir(const char *path)
{
    PROCESS_PATH(NULL);
    DIR *dir = new DIR;
    if(dir == NULL)
    {
        errno = ENOMEM;
        return NULL;
    }
    auto res = dev->OpenDirectory(ppath.processed_path, dir->devdir);
    if(res.IsFailure())
    {
        errno = bio::Result::GetErrnoFrom(res);
        delete dir;
        return NULL;
    }
    return dir;
}

struct dirent *readdir(DIR *dp)
{
    if(dp == NULL) return NULL;
    bio::fsp::DirectoryEntry ent = {};
    auto res = dp->devdir->Next(ent);
    if(res.IsSuccess())
    {
        memset(&dp->ent, 0, sizeof(dp->ent));
        strcpy(dp->ent.d_name, ent.name);
        dp->ent.d_namlen = strlen(ent.name);
        dp->ent.d_name[dp->ent.d_namlen] = '\0';
        dp->ent.d_ino = 0;
        return &dp->ent;
    }
    return NULL;
}

int closedir(DIR *dp)
{
    delete dp;
	return 0;
}