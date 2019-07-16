#include <bio/os/os_Version.hpp>
#include <bio/ipc/ipc_Request.hpp>

namespace bio::os
{
    Version GetFirmwareVersion()
    {
        Version v = { 0, 0, 0 };
        ipc::ServiceSession setsys("set:sys");

        struct RawVersion
        {
            u8 major;
            u8 minor;
            u8 micro;
            u8 padding1;
            u8 revision_major;
            u8 revision_minor;
            u8 padding2;
            u8 padding3;
            char platform[0x20];
            char version_hash[0x40];
            char display_version[0x18];
            char display_title[0x80];
        } ver = {}; // Wrapper until setsys is properly implemented
        
        setsys.ProcessRequest<3>(ipc::OutStaticBuffer(&ver, sizeof(ver), 0));
        BIO_LOG("Version display title: %s", ver.display_title);

        v.major = ver.major;
        v.minor = ver.minor;
        v.micro = ver.micro;

        setsys.Close();

        return v;
    }
}