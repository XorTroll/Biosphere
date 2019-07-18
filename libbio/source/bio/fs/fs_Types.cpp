#include <bio/fs/fs_Types.hpp>
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
    static std::shared_ptr<fsp::Service> _inner_FsSession;
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

    FileSystemDeviceFile::FileSystemDeviceFile(std::shared_ptr<fsp::File> &file) : ifile(file), offset(0)
    {
    }

    Result FileSystemDeviceFile::Read(void *ptr, size_t size, Out<u64> written)
    {
        return ifile->Read(offset, ptr, size, written);
    }

    Result FileSystemDeviceFile::Write(const void *ptr, size_t size)
    {
        return ifile->Write(offset, ptr, size);
    }

    Result FileSystemDeviceFile::Seek(int pos, int whence, Out<off_t> off)
    {
        u64 tmpoff = 0;
        switch(whence)
        {
            case SEEK_SET:
                tmpoff = 0;
                break;
            case SEEK_CUR:
                tmpoff = offset;
                break;
            case SEEK_END:
                ifile->GetSize(tmpoff);
                break;
            default:
                return 0x2ee202;
        }
        if((pos < 0) && (tmpoff < -pos)) return 0x2ee202;
        offset = tmpoff + pos;
        (off_t&)off = offset;
        return 0;
    }

    FileSystemDeviceDirectory::FileSystemDeviceDirectory(std::shared_ptr<fsp::Directory> &dir) : idir(dir), entryread_count(-1)
    {
    }

    Result FileSystemDeviceDirectory::Next(Out<fsp::DirectoryEntry> out)
    {
        u64 count = 0;
        auto res = idir->GetEntryCount(count);
        if((entryread_count + 1) < count)
        {
            fsp::DirectoryEntry *entries = new fsp::DirectoryEntry[count];
            u64 countnew = 0;
            res = idir->Read(entries, count, countnew);
            if(res.IsSuccess())
            {
                entryread_count++;
                memcpy(out.AsPtr(), &entries[entryread_count], sizeof(fsp::DirectoryEntry));
            }
            delete[] entries;
        }
        else res = 0xdead; // custom results...
        return res;
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

    Result FileSystemDevice::OpenDirectory(const char *path, Out<std::shared_ptr<DeviceDirectory>> out)
    {
        std::shared_ptr<fsp::Directory> dir;
        auto res = ifs->OpenDirectory(3, path, dir);
        if(res.IsSuccess()) (std::shared_ptr<DeviceDirectory>&)out = std::make_shared<FileSystemDeviceDirectory>(dir);
        return res;
    }

    Result FileSystemDevice::OpenFile(const char *path, int flags, int mode, Out<std::shared_ptr<DeviceFile>> out)
    {
        u32 fspfileflags = 0;
        switch(flags & O_ACCMODE)
        {
            /* read-only: do not allow O_APPEND */
            case O_RDONLY:
                fspfileflags |= BIO_BITMASK(0);
                if(flags & O_APPEND)
                {
                    errno = EINVAL;
                    return -1;
                }
            break;

            /* write-only */
            case O_WRONLY:
                fspfileflags |= (BIO_BITMASK(1) | BIO_BITMASK(2));
            break;

            /* read and write */
            case O_RDWR:
                fspfileflags |= (BIO_BITMASK(0) | BIO_BITMASK(1) | BIO_BITMASK(2));
            break;

            /* an invalid option was supplied */
            default:
                errno = EINVAL;
            return -1;
        }

        if(flags & O_CREAT)
        {
            auto res = ifs->CreateFile(0, 0, path);
            if(res.IsSuccess()) res = 0xdead4;
            return res;
        }

        std::shared_ptr<fsp::File> file;
        auto res = ifs->OpenFile(fspfileflags, path, file);
        if(res.IsSuccess())
        {
            if((flags & O_ACCMODE) != O_RDONLY && (flags & O_TRUNC)) res = file->SetSize(0);
            if(res.IsSuccess()) (std::shared_ptr<DeviceFile>&)out = std::make_shared<FileSystemDeviceFile>(file);
        }

        return res;
    }

    Result Initialize()
    {
        _inner_FsSession = fsp::Service::Initialize();
        _inner_Initialized = true;
        return 0;
    }

    bool IsInitialized()
    {
        return _inner_Initialized;
    }

    void Finalize()
    {
        if(_inner_Initialized)
        {
            UnmountAll();
            _inner_FsSession.reset();
            _inner_Initialized = false;
        }
    }

    std::shared_ptr<fsp::Service> &GetFsSession()
    {
        return std::ref(_inner_FsSession);
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

    void UnmountAll()
    {
        _inner_DeviceList.clear();
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
        if(res == 0xdead4) return 0; // File created, not opened
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
        if(file == STDOUT_FILENO)
        {
            extern bio::log::LogWriteFunction global_StdoutLog;
            global_StdoutLog(ptr, len);
            return len;
        }
        int vecidx = (file - 200);
        ssize_t ret = 0;
        if(vecidx < bio::fs::_inner_FileList.size())
        {
            std::shared_ptr<bio::fs::DeviceFile> fd = std::ref(bio::fs::_inner_FileList.at(vecidx));
            auto res = fd->Write(ptr, len);
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
        strcpy(dp->ent.d_name, ent.path);
        dp->ent.d_namlen = strlen(ent.path);
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