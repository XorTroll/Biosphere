#include <bio/fs/fs_Types.hpp>
#include <bio/err/err_Assertion.hpp>
#include <bio/sm/sm_Port.hpp>
using namespace bio;

#include <fstream>
#include <cstdio>
#include <cstring>


int main(int argc, char **argv)
{
    ipc::ServiceSession srv("demo:no");
    srv.GetInitialResult().Assert();

    log::SetStdoutLoggingFunction(log::FloraStdoutLoggingFunction);
    log::SetStderrLoggingFunction(log::FloraStderrLoggingFunction);

    BIO_LOG("%s %d", "Hello from Biosphere!", 34);

    return 0;
}