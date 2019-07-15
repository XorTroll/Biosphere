#include <bio_Types.hpp>
#include <cstring>
#include <bio/log/log_Logging.hpp>
#include <bio/err/err_Assertion.hpp>

extern bio::err::AssertionFunction global_Assertion;

namespace bio
{
    static bool _inner_ResultAutoAssert = false;

    Result::Result(u32 raw) : rc(raw)
    {
        if(_inner_ResultAutoAssert) Assert();
    }

    Result::operator u32()
    {
        return rc;
    }

    bool Result::IsSuccess()
    {
        return (rc == 0);
    }

    bool Result::IsFailure()
    {
        return !IsSuccess();
    }

    void Result::Assert()
    {
        global_Assertion(rc);
    }

    void Result::SetAutoAssert(bool auto_assert)
    {
        _inner_ResultAutoAssert = auto_assert;
    }

    bool Result::GetAutoAssert()
    {
        return _inner_ResultAutoAssert;
    }
}