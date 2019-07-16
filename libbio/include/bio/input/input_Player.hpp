
#pragma once
#include <bio/input/input_Types.hpp>
#include <bio/hid/hid_Service.hpp>

namespace bio::input
{
    class Player
    {
        public:
            Player(InputMemory *mem, Controller controller);
            bool IsConnected();
            bool AreJoyConsJoined();
            u64 GetInputHeld();
            u64 GetInputDown();
            u64 GetInputUp();
        private:
            InputMemory *shmem;
            Controller controller;
            u64 prev_state;
    };

    Result Initialize(u64 aruid);
    bool IsInitialized();
    std::shared_ptr<hid::Service> &GetHidSession();
    InputMemory *GetInputMemory();
    std::shared_ptr<Player> GetPlayer(Controller controller);
    std::shared_ptr<Player> GetMainPlayer(); // Get handheld/player1 depending on which one is the current state
}