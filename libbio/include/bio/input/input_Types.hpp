
#pragma once
#include <bio/hid/hid_Types.hpp>

namespace bio::input
{
    enum class Controller
    {
        Player1,
        Player2,
        Player3,
        Player4,
        Player5,
        Player6,
        Player7,
        Player8,
        HandHeld,
        Unknown,
    };

    enum Key
    {
        A = BIO_BITMASK(0),
        B = BIO_BITMASK(1),
        X = BIO_BITMASK(2),
        Y = BIO_BITMASK(3),
        LStick = BIO_BITMASK(4),
        RStick = BIO_BITMASK(5),
        L = BIO_BITMASK(6),
        R = BIO_BITMASK(7),
        ZL = BIO_BITMASK(8),
        ZR = BIO_BITMASK(9),
        Plus = BIO_BITMASK(10),
        Minus = BIO_BITMASK(11),
        Left = BIO_BITMASK(12),
        Right = BIO_BITMASK(13),
        Up = BIO_BITMASK(14),
        Down = BIO_BITMASK(15),
        LStickLeft = BIO_BITMASK(16),
        LStickUp = BIO_BITMASK(17),
        LStickRight = BIO_BITMASK(18),
        LStickDown = BIO_BITMASK(19),
        RStickLeft = BIO_BITMASK(20),
        RStickUp = BIO_BITMASK(21),
        RStickRight = BIO_BITMASK(22),
        RStickDown = BIO_BITMASK(23),
        SLLeft = BIO_BITMASK(24),
        SRLeft = BIO_BITMASK(25),
        SLRight = BIO_BITMASK(26),
        SRRight = BIO_BITMASK(27),
        Touch = BIO_BITMASK(28),
    };

    struct TouchData
    {
        u64 timestamp;
        u32 pad1;
        u32 index;
        u32 x;
        u32 y;
        u32 diameter_x;
        u32 diameter_y;
        u32 angle;
        u32 pad2;
    };

    struct TouchEntry
    {
        u64 timestamp;
        u64 count;
        TouchData youches[16];
        u64 pad1;
    };

    struct TouchState
    {
        u64 timestamp_ticks;
        u64 entry_count;
        u64 latest_index;
        u64 max_index;
        u64 tmestamp;
        TouchEntry entries[17];
    };

    struct JoystickPosition
    {
        u32 x;
        u32 y;
    };

    struct ControllerStateEntry
    {
        u64 timestamp;
        u64 timestamp2;
        u64 button_state;
        JoystickPosition left_pos;
        JoystickPosition right_pos;
        u64 connection_state;
    };

    struct ControllerState
    {
        u64 timestamp;
        u64 entry_count;
        u64 latest_index;
        u64 max_index;
        ControllerStateEntry entries[17];
    };

    struct ControllerMACAddress
    {
        u8 address[0x10];
    };

    struct ControllerColor
    {
        u32 body;
        u32 buttons;
    };

    struct ControllerData
    {
        u32 status;
        u32 is_joycon_half;
        u32 colors_descriptor_single;
        ControllerColor color_single;
        u32 colors_descriptor_split;
        ControllerColor color_right;
        ControllerColor color_left;
        ControllerState pro_controller;
        ControllerState handleld;
        ControllerState joined;
        ControllerState left;
        ControllerState right;
        ControllerState main_no_analog;
        ControllerState main;
        u8 unk[0x2a78];
        ControllerMACAddress macs[0x2];
        u8 unk2[0xe10];
    };

    struct InputMemory
    {
        u8 header[0x400];
        TouchState touch_state;
        u8 pad[0x3c0];
        u8 mouse[0x400];
        u8 keyboard[0x400];
        u8 unk[0x400];
        u8 unk2[0x400];
        u8 unk3[0x400];
        u8 unk4[0x400];
        u8 unk5[0x200];
        u8 unk6[0x200];
        u8 unk7[0x200];
        u8 unk8[0x800];
        u8 controller_serials[0x4000];
        ControllerData controllers[10];
        u8 unk9[0x4600];
    };
}