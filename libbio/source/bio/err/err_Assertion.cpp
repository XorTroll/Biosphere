#include <bio/err/err_Assertion.hpp>
#include <bio/fatal/fatal_Service.hpp>
#include <svc/svc_Base.hpp>
#include <cstdio>
#include <cstring>

bio::err::AssertionFunction global_Assertion = bio::err::SvcOutputAssertionFunction;

namespace bio::err
{
    void Assert(bool condition)
    {
        if(!condition) AssertResult(0xdead);
    }

    void AssertResult(Result res)
    {
        if(res.IsFailure()) global_Assertion(res);
    }

    void SetAssertionFunction(AssertionFunction func)
    {
        global_Assertion = func;
    }

    void FatalAssertionFunction(Result res)
    {
        auto &[rc, fatalsrv] = fatal::Service::Initialize().Assert();
        if(rc.IsSuccess()) fatalsrv->ThrowWithPolicy(res, fatal::ThrowMode::ErrorScreen);
    }

    void ErrorAppletAssertionFunction(Result res)
    {
        // applet...
    }

    void SvcOutputAssertionFunction(Result res)
    {
        char out[0x100] = {0};
        sprintf(out, "Result assert: %04d-%04d (0x%X)", 2000 + res.module, res.description, (u32)res);
        svc::OutputDebugString(out, 0x100);
    }
}