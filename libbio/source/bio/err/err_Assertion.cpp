#include <bio/err/err_Assertion.hpp>
#include <svc/svc_Base.hpp>
#include <cstdio>
#include <cstring>

bio::err::AssertionFunction global_Assertion = bio::err::FatalAssertionFunction;

namespace bio::err
{
    void Assert(bool condition)
    {
        if(!condition) AssertResult(0xdead);
    }

    void AssertResult(Result res)
    {
        global_Assertion(res);
    }

    void SetAssertionFunction(AssertionFunction func)
    {
        global_Assertion = func;
    }

    void FatalAssertionFunction(Result res)
    {
        // fatal!
    }

    void ErrorAppletAssertionFunction(Result res)
    {
        // applet...
    }

    void SvcOutputAssertionFunction(Result res)
    {
        char out[0x100];
        sprintf(out, "Result assert: 0x%llX", res);
        svc::OutputDebugString(out, strlen(out));
    }
}