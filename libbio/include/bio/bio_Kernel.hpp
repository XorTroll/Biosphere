
#pragma once
#include <bio/bio_Types.hpp>

namespace bio
{
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

    class KObject
    {
        public:
            KObject();
            KObject(u32 handle);
            KObject(const KObject &) = delete;
            KObject &operator=(const KObject &) = delete;
            KObject(KObject &&other);
            KObject &operator=(KObject &&other);
            ~KObject();

            u32 Claim();
            
            u32 handle = 0;
    };

    static KObject InvalidObject = KObject(0);
}