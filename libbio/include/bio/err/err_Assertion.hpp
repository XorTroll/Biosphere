
#pragma once
#include <bio/bio_Types.hpp>

namespace bio::err
{
    typedef void(*AssertionFunction)(Result res);

    void Assert(bool condition);
    void AssertResult(Result res);

    void SetAssertionFunction(AssertionFunction func);

    void FatalAssertionFunction(Result res);
    void ErrorAppletAssertionFunction(Result res);
    void SvcOutputAssertionFunction(Result res);
}