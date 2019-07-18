#include <bio/log/log_Logging.hpp>
#include <bio/err/err_Assertion.hpp>
#include <cstring>
#include <cerrno>
#include <bio/fs/fs_Types.hpp>
#include <bio/input/input_Player.hpp>
#include <bio/sm/sm_Port.hpp>
#include <fstream>
#include <malloc.h>
#include <sha256.h>
#include <bio/ld/ld_Dynamic.hpp>
#include <dlfcn.h>

using namespace bio;

int main()
{
    BIO_LOG("%s", "Hello from main");

    fs::Initialize().Assert();
    fs::MountSdCard("sdcard").Assert();

    FILE *f = fopen("sdcard:/demo.lib.nro", "rb");
    if(!f)
    {
        BIO_LOG("NRO file isn't in SD card!");
        while(true);
    }
    fclose(f);

    ld::Initialize();

    u64 tmpptr = 0;

    { // Suppors both C++-style Module class
        std::shared_ptr<ld::Module> module;
        ld::LoadModule("sdcard:/demo.lib.nro", false, module).Assert();

        BIO_LOG("%s", "Module loaded (ld::LoadModule)");

        auto get_mod_func = module->ResolveSymbol<const char*(*)()>("GetModuleName");
        if(get_mod_func)
        {
            BIO_LOG("Function ptr (GetModuleName): %p", get_mod_func);
            BIO_LOG("Demo text: %s", get_mod_func());
        }

        auto say_hello_func = module->ResolveSymbol<void(*)()>("SayHello");
        tmpptr = (u64)say_hello_func;
        if(say_hello_func)
        {
            BIO_LOG("Function ptr (SayHello): %p", say_hello_func);
            say_hello_func();
        }

        // Disposed by RAII
    }

    ld::Finalize();

    fs::Finalize();
    sm::Finalize();
    
    return tmpptr;
}