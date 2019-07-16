#include <bio/input/input_Player.hpp>
#include <bio/os/os_Memory.hpp>

namespace bio::input
{
    static std::shared_ptr<hid::Service> _inner_HidSession;
    static bool _inner_Initialized = false;
    static std::shared_ptr<hid::AppletResource> _inner_AppletResource;
    static std::shared_ptr<os::SharedMemory> _inner_HidSharedMem;

    Player::Player(InputMemory *mem, Controller controller) : shmem(mem), controller(controller), prev_state(0)
    {
    }

    bool Player::IsConnected()
    {
        ControllerData cd = shmem->controllers[static_cast<u32>(controller)];
        return (cd.main.entries[cd.main.latest_index].connection_state & BIO_BITMASK(0));
    }

    bool Player::AreJoyConsJoined()
    {
        return !(bool)(shmem->controllers[static_cast<u32>(controller)].is_joycon_half);
    }

    u64 Player::GetInputHeld()
    {
        ControllerData cd = shmem->controllers[static_cast<u32>(controller)];
        u64 cur_ipt = cd.main.entries[cd.main.latest_index].button_state;
        prev_state = cur_ipt;
        return cur_ipt;
    }

    u64 Player::GetInputDown()
    {
        ControllerData cd = shmem->controllers[static_cast<u32>(controller)];
        u64 cur_ipt = cd.main.entries[cd.main.latest_index].button_state;
        u64 down = ((~prev_state) & cur_ipt);
        prev_state = cur_ipt;
        return down;
    }

    u64 Player::GetInputUp()
    {
        ControllerData cd = shmem->controllers[static_cast<u32>(controller)];
        u64 cur_ipt = cd.main.entries[cd.main.latest_index].button_state;
        u64 up = (prev_state & (~cur_ipt));
        prev_state = cur_ipt;
        return up;
    }

    Result Initialize(u64 aruid)
    {
        if(IsInitialized()) return 0;
        _inner_HidSession = hid::Service::Initialize();
        auto res = _inner_HidSession->CreateAppletResource(aruid, _inner_AppletResource);
        if(res.IsSuccess())
        {
            KObject tmphandle;
            res = _inner_AppletResource->GetSharedMemoryHandle(tmphandle);
            if(res.IsSuccess())
            {
                _inner_HidSharedMem = std::make_shared<os::SharedMemory>(tmphandle.Claim(), 0x40000, Permission::Read);
                res = _inner_HidSharedMem->Map();
                if(res.IsSuccess())
                {
                    res = _inner_HidSession->ActivateNpad(aruid);
                    if(res.IsSuccess())
                    {
                        res = _inner_HidSession->SetSupportedNpadStyleSet(aruid, (static_cast<u32>(hid::NpadStyleTag::ProController) | static_cast<u32>(hid::NpadStyleTag::Handheld) | static_cast<u32>(hid::NpadStyleTag::JoyconPair) | static_cast<u32>(hid::NpadStyleTag::JoyconLeft) | static_cast<u32>(hid::NpadStyleTag::JoyconRight)));
                        if(res.IsSuccess())
                        {
                            u32 controllers[9] = { 0, 1, 2, 3, 4, 5, 6, 7, 0x20 };
                            res = _inner_HidSession->SetSupportedNpadIdType(aruid, controllers, 9);
                            if(res.IsSuccess())
                            {
                                _inner_Initialized = true;
                                for(u32 i = 0; i < 8; i++)
                                {
                                    auto res2 = _inner_HidSession->SetNpadJoyAssignmentModeDual(aruid, i);
                                    if(res2.IsFailure()) break;
                                }
                            }
                        }
                    }
                }
            }
        }
        return res;
    }

    bool IsInitialized()
    {
        return _inner_Initialized;
    }

    std::shared_ptr<hid::Service> &GetHidSession()
    {
        return _inner_HidSession;
    }

    InputMemory *GetInputMemory()
    {
        return (InputMemory*)_inner_HidSharedMem->GetAddress();
    }

    std::shared_ptr<Player> GetPlayer(Controller controller)
    {
        return std::make_shared<Player>(GetInputMemory(), controller);
    }

    std::shared_ptr<Player> GetMainPlayer()
    {
        ControllerData cd = GetInputMemory()->controllers[static_cast<u32>(Controller::Player1)];
        bool conn = (cd.main.entries[cd.main.latest_index].connection_state & BIO_BITMASK(0));
        if(conn) return GetPlayer(Controller::Player1);
        else return GetPlayer(Controller::HandHeld);
    }
}