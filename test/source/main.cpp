#include <biosphere>
using namespace bio;

#include <fstream>
#include <cstdio>
#include <cstring>

int main(int argc, char **argv)
{
    fs::Initialize();
    fs::MountSdCard("sdcard");

    FILE *f = fopen("sdcard:/out-demo.txt", "w");
    if(f)
    {
        fprintf(f, "%s %s!", "This is", "Biosphere");
        fclose(f);
    }

    fs::Finalize();

    return 0;
}