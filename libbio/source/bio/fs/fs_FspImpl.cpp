#include <bio/fs/fs_FspImpl.hpp>
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
    static std::shared_ptr<fsp::Service> _inner_FsSession;
    static bool _inner_Initialized = false;

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

    Result FileSystemDeviceFile::Flush()
    {
        return ifile->Flush();
    }

    FileSystemDeviceDirectory::FileSystemDeviceDirectory(std::shared_ptr<fsp::Directory> &dir) : idir(dir), entries_block(NULL)
    {
    }

    FileSystemDeviceDirectory::~FileSystemDeviceDirectory()
    {
        if(entries_block != NULL) delete[] entries_block;
    }

    Result FileSystemDeviceDirectory::Next(Out<fsp::DirectoryEntry> out)
    {
        Result res;
        if(entries_block == NULL)
        {
            entry_index = 0;
            entry_count = 0;
            res = idir->GetEntryCount(entry_count);
            if(res.IsFailure()) return res;
            entries_block = new fsp::DirectoryEntry[entry_count];
            u64 tmp;
            res = idir->Read(entries_block, entry_count, tmp);
        }

        if(res.IsSuccess())
        {
            if(entry_index < entry_count)
            {
                memcpy(out.AsPtr(), &entries_block[entry_index], sizeof(fsp::DirectoryEntry));
                entry_index++;
            }
            else res = ResultEndOfDirectory;
        }

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
            auto res1 = ifs->CreateFile(0, 0, path);
            if(res1.IsFailure()) return res1;
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
        Result res;
        if(!_inner_Initialized)
        {
            auto [res, fssrv] = fsp::Service::Initialize();
            if(res.IsSuccess())
            {
                _inner_FsSession = std::move(fssrv);
                _inner_Initialized = true;
            }
        }
        return res;
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

    Result MountFileSystem(std::shared_ptr<fsp::FileSystem> &fs, const char *name)
    {
        std::shared_ptr<Device> dev = std::make_shared<FileSystemDevice>(name, fs);
        return Mount(dev);
    }

    Result MountSdCard(const char *name)
    {
        std::shared_ptr<fsp::FileSystem> sdfs;
        auto res = _inner_FsSession->OpenSdCardFileSystem(sdfs);
        if(res.IsSuccess()) res = MountFileSystem(sdfs, name);
        return res;
    }

    extern "C"
    {
        extern bio::u8 __bio_crt0_ExecutableFormat;
    }

    extern void *global_NroRomImage;

    Result MountRom(const char *name)
    {
        Result res;
        if(__bio_crt0_ExecutableFormat == 0) // Regular title
        {
            std::shared_ptr<fsp::FileSystem> romfs;
            res = _inner_FsSession->OpenDataFileSystemByCurrentProcess(romfs);
            if(res.IsSuccess()) res = MountFileSystem(romfs, name);
        }
        else if(global_NroRomImage != NULL)
        {
            // mount romfs dev using global_NroRomImage
        }
        return res;
    }
}