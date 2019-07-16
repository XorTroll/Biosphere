#include <bio/log/log_Logging.hpp>
#include <bio/err/err_Assertion.hpp>
#include <cstring>
#include <cerrno>
#include <bio/input/input_Player.hpp>


int main()
{
    BIO_LOG("%s", "hello from main!");

    bio::input::Initialize(0).Assert();
    
    auto player = bio::input::GetMainPlayer();

    while(true)
    {
        if(player->GetInputDown() & bio::input::Key::A)
        {
            BIO_LOG("%s", "A pressed!");
        }
    }

    return 0;
}