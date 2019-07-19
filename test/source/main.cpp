#include <bio/log/log_Logging.hpp>
#include <bio/err/err_Assertion.hpp>
#include <cstring>
#include <cerrno>
#include <bio/fs/fs_Types.hpp>
#include <bio/input/input_Player.hpp>
#include <bio/sm/sm_Port.hpp>
#include <fstream>
#include <malloc.h>
#include <bio/ro/ro_Service.hpp>
#include <dlfcn.h>

using namespace bio;

int main()
{
    fs::Initialize().Assert();
    fs::MountSdCard("sd").Assert();

    FILE *f = fopen("sd:/boot.config", "rb");
    fclose(f);

    BIO_LOG("yay");

    fs::Finalize();
    
    return 0;
}