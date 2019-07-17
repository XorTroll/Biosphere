#include <bio/log/log_Logging.hpp>
#include <bio/err/err_Assertion.hpp>
#include <cstring>
#include <cerrno>
#include <bio/fs/fs_Types.hpp>
#include <bio/input/input_Player.hpp>

#include <fstream>

int main()
{
    BIO_LOG("%s", "hello from main!");

    bio::fs::Initialize().Assert();
    bio::fs::MountSdCard("sdhc").Assert();

    std::ifstream ifs("sdhc:/boot.config");
    std::string line;
    while(std::getline(ifs, line))
    {
        BIO_LOG("Line: '%s'", line.c_str());
    }
    ifs.close();

    return 0;
}