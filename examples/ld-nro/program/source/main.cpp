#include <bio/log/log_Logging.hpp>
#include <bio/fs/fs_Types.hpp>
#include <bio/ld/ld_Dynamic.hpp>
#include <bio/sm/sm_Port.hpp>

#include <dlfcn.h> // Since Module uses dl* stuff, dlerror is useful for error detecting

using namespace bio;

int main()
{
    BIO_LOG("Hello from main!");

    fs::Initialize().Assert();
    fs::MountSdCard("sdcard").Assert();

    ld::Initialize();

    u64 tmpptr = 0;

    {
        std::shared_ptr<ld::Module> module; // bio::ld::Module is a wrapper for dlopen/dlsym/dlclose
        ld::LoadModule("sdcard:/demo.lib.nro", false, module).Assert();

        BIO_LOG("%s", "Module loaded (ld::LoadModule)");

        auto get_mod_func = module->ResolveSymbol<const char*(*)()>("GetModuleName");
        if(get_mod_func)
        {
            BIO_LOG("Function ptr (GetModuleName): %p", get_mod_func);
            BIO_LOG("Demo text: %s", get_mod_func());
        }
        else BIO_LOG("Error -> %s", dlerror());

        auto say_hello_func = module->ResolveSymbol<void(*)()>("SayHello");
        tmpptr = (u64)say_hello_func;
        if(say_hello_func)
        {
            BIO_LOG("Function ptr (SayHello): %p", say_hello_func);
            say_hello_func();
        }
        else BIO_LOG("Error -> %s", dlerror());

        // Module object is disposed by RAII
    }

    ld::Finalize();

    fs::Finalize();
    sm::Finalize();

    while(true);
    
    return 0;
}