#include <biosphere>
using namespace bio;

#include <dlfcn.h> // Since Module uses dl* stuff, dlerror is useful for error detecting

int main()
{
    BIO_LOG("Hello from main!");

    fs::Initialize().Assert();
    fs::MountSdCard("sdcard").Assert();
    ld::Initialize().Assert();

    {
        // bio::ld::Module is a wrapper for dlopen/dlsym/dlclose
        auto [res, module] = ld::LoadModule("sdcard:/demo.lib.nro", false);

        if(res.IsSuccess())
        {
            BIO_LOG("%s", "Module loaded (ld::LoadModule)");

            auto [res2, get_mod_func] = module->ResolveSymbol<const char*(*)()>("GetModuleName");
            if(res2.IsSuccess())
            {
                BIO_LOG("Function ptr (GetModuleName) = %p", get_mod_func);
                BIO_LOG("Module name: %s", get_mod_func());
            }
            else BIO_LOG("Error -> %s", dlerror());

            auto [res3, say_hello_func] = module->ResolveSymbol<void(*)()>("SayHello");
            if(res3.IsSuccess())
            {
                BIO_LOG("Function ptr (SayHello) = %p", say_hello_func);
                say_hello_func();
            }
            else BIO_LOG("Error -> %s", dlerror()); // Normally would log results, but with dynamic loading dlerror will probably have the result
        }

        // Module object is disposed by RAII
    }

    ld::Finalize();

    fs::Finalize();
    sm::Finalize();

    while(true);
    
    return 0;
}