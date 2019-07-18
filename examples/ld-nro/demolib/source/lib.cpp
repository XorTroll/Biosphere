#include <bio/log/log_Logging.hpp>

extern "C" // Declare with C linkage to avoid name mangling
{
    const char *GetModuleName()
    {
        return "Demo library";
    }

    void SayHello()
    {
        BIO_LOG("%s", "Hello from demo NRO library!");
    }
}