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

extern size_t global_HeapSize;

int main()
{
    BIO_LOG("Hello from main(), heap: 0x%X", global_HeapSize);

    fs::Initialize().Assert();
    fs::MountSdCard("sd").Assert();


    fs::Finalize();
    sm::Finalize();
    
    return 0;
}