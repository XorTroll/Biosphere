
#pragma once
#include <cstdlib>
#include <cstdint>
#include <bio/bio_Macros.hpp>

namespace bio
{
    typedef uint8_t u8;
    typedef int8_t i8;
    typedef uint16_t u16;
    typedef int16_t i16;
    typedef uint32_t u32;
    typedef int32_t i32;
    typedef uint64_t u64;
    typedef int64_t i64;

    struct Result
    {
        Result();
        Result(u32 raw);
        Result(u32 mod, u32 desc);
        bool IsSuccess();
        bool IsFailure();
        operator u32();
        void Assert();
        static int GetErrnoFrom(Result res);

        u32 module;
        u32 description;
    };

    template<typename T>
    class Out
    {
        public:
            Out(T &val_ref) : ref(val_ref)
            {
            }

            operator T&()
            {
                return ref;
            }

            T *AsPtr()
            {
                return &ref;
            }

        private:
            T &ref;
    };

    enum class Permission
    {
        NoPermission = 0,
        Read = BIO_BITMASK(0),
        Write = BIO_BITMASK(1),
        Execute = BIO_BITMASK(2),
        ReadWrite = (Read | Write),
        ReadExecute = (Read | Execute),
        DontCare = BIO_BITMASK(28),
    };

    static const u32 ResultModule = 420;
}