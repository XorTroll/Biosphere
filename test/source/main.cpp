
#include <bio/fs/fs_Types.hpp>
using namespace bio;

int main(int argc, char **argv)
{
    fs::Initialize().Assert();
    fs::MountSdCard("sd").Assert();

    DIR *dp = opendir("sd:");
    if(dp)
    {
        dirent *dt;
        while(true)
        {
            dt = readdir(dp);
            if(dt == NULL) break;
            BIO_LOG("sdmc:/%s", dt->d_name);
        }
        closedir(dp);
    }

    fs::Finalize();

    return 0;
}