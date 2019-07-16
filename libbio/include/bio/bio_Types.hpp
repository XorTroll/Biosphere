
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

    class Result
    {
        public:
            Result(u32 raw);
            bool IsSuccess();
            bool IsFailure();
            operator u32();
            void Assert();

            static void SetAutoAssert(bool auto_assert);
            static bool GetAutoAssert();
        private:
            u32 rc;
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
}