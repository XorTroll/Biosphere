#include <bio_Types.hpp>
#include <cstring>
#include <bio/log/log_Logging.hpp>
#include <bio/err/err_Assertion.hpp>
#include <cerrno>

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

    int Result::GetErrnoFrom(Result res)
    {
        if(res.IsFailure())
        {
            switch(res)
            {
                case 0x202:
                    return ENOENT;
                case 0x402:
                case 0x177602:
                    return EEXIST;
                case 0x2ee602:
                    return ENAMETOOLONG;
                case 0x2ee202:
                    return EINVAL;
                case 0xe02:
                    return EBUSY;
                case 0x196002:
                case 0x196202:
                case 0x1a3e02:
                case 0x1a4002:
                case 0x1a4a02:
                    return ENOMEM;
                case 0xea01:
                    return ETIMEDOUT;
                // use EIO as default
                default:
                    return EIO;
            }
        }
        return 0;
    }
}