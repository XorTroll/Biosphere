#include <bio/log/log_Logging.hpp>
#include <bio/fs/fs_Types.hpp>
#include <cstring>
#include <cerrno>

extern size_t global_HeapSize;

void demostat(const char *p)
{
    struct stat st;
    int res = stat(p, &st);
    BIO_LOG("stat '%s': %i", p, res);

    if(res == 0)
    {
        bool dir = (st.st_mode & S_IFDIR);
        bool reg = (st.st_mode & S_IFREG);

        BIO_LOG("st_mode & s_IFDIR: %d", dir);
        BIO_LOG("st_mode & s_IFREG: %d", reg);
    }

    else
    {
        auto errstr = strerror(errno);
        BIO_LOG("errno: %i, strerror: %s", errno, errstr);
    }
}

int main()
{
    BIO_LOG("%s", "hello from main!");

    bio::fs::Initialize();
    bio::fs::MountSdCard("sd");

    demostat("sd:/switch");
    demostat("sd:/hbmenu.nro");
    demostat("sd:/demo-no-exist");

    return global_HeapSize;
}