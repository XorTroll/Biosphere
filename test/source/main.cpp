#include <biosphere>
using namespace bio;

#include <fstream>
#include <cstdio>
#include <cstring>
#include <iostream>

int main(int argc, char **argv)
{
    fs::Initialize().Assert();
    fs::MountSdCard("sd").Assert();

    auto f = fopen("sd:/demo-newipc2.txt", "a");
    if(f)
    {
        fprintf(f, "%s %s!", "Biosphere", "rules");
        fclose(f);
    }
    
    fs::Finalize();

    return 0;
}