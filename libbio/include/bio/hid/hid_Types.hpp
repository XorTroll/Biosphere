
#pragma once
#include <bio/bio_Types.hpp>

namespace bio::hid
{
    enum class NpadStyleTag
    {
        ProController = BIO_BITMASK(0),
        Handheld = BIO_BITMASK(1),
        JoyconPair = BIO_BITMASK(2),
        JoyconLeft = BIO_BITMASK(3),
        JoyconRight = BIO_BITMASK(4),
    };

    enum class NpadJoyDeviceType
    {
        Left,
        Right,
    };
}