#include <biosphere>
using namespace bio;

#include <cstdio>
#include <cstring>

int main()
{
    fs::Initialize().Assert();
    fs::MountRom("rom").Assert();

    FILE *f = fopen("rom:/test.txt", "r");
    if(f)
    {
        char txt[0x20] = {0};
        fgets(txt, 0x20, f);
        BIO_LOG("File contents: %s", txt);
        fclose(f);
    }
    else BIO_LOG("File open error: %s", strerror(errno));

    fs::Finalize();
    return 0;
}