#include <bio/bio_Types.hpp>
#include <cstring>
#include <bio/log/log_Logging.hpp>
#include <bio/err/err_Assertion.hpp>
#include <cerrno>

extern bio::err::AssertionFunction global_Assertion;

namespace bio
{
    Result::Result() : module(0), description(0)
    {
    }

    Result::Result(u32 raw) : module(raw & 0x1FF), description((raw >> 9) & 0x1FFF)
    {
        if(raw == 0)
        {
            module = 0;
            description = 0;
        }
    }

    Result::Result(u32 mod, u32 desc) : module(mod), description(desc)
    {
    }

    Result::operator u32()
    {
        return BIO_MAKERESULT(module, description);
    }

    bool Result::IsSuccess()
    {
        return (description == 0);
    }

    bool Result::IsFailure()
    {
        return !IsSuccess();
    }

    void Result::Assert()
    {
        global_Assertion(*this);
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