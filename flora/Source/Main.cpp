#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <malloc.h>
#include <switch.h>
#include <stratosphere.hpp>

#include <flora/flora_Manager.hpp>
#include <flora/flora_ILogService.hpp>

#define INNER_HEAP_SIZE 0x75000

extern "C"
{
    extern u32 __start__;
    u32 __nx_applet_type = AppletType_None;
    size_t nx_inner_heap_size = INNER_HEAP_SIZE;
    char   nx_inner_heap[INNER_HEAP_SIZE];
    void __libnx_initheap(void);
    void __appInit(void);
    void __appExit(void);
    void __libnx_init_time(void);
}

void __libnx_initheap(void)
{
	void *addr = nx_inner_heap;
	size_t size = nx_inner_heap_size;
	extern char *fake_heap_start;
	extern char *fake_heap_end;
	fake_heap_start = (char*)addr;
	fake_heap_end = (char*)addr + size;
}

static const SocketInitConfig sockInitConf = // Thanks hid-mitm
{
    .bsdsockets_version = 1,
    .tcp_tx_buf_size = 0x400,
    .tcp_rx_buf_size = 0x800,
    .tcp_tx_buf_max_size = 0x800,
    .tcp_rx_buf_max_size = 0x1000,
    .udp_tx_buf_size = 0x100,
    .udp_rx_buf_size = 0x100,
    .sb_efficiency = 2,
    .serialized_out_addrinfos_max_size = 0x1000,
    .serialized_out_hostent_max_size = 0x200,
    .bypass_nsd = false,
    .dns_timeout = 0
};

void __appInit(void)
{
    Result rc = smInitialize();
    if(R_FAILED(rc)) fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

    rc = setsysInitialize();
    if(R_SUCCEEDED(rc))
    {
        SetSysFirmwareVersion fw;
        rc = setsysGetFirmwareVersion(&fw);
        if(R_SUCCEEDED(rc)) hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
        setsysExit();
    }

    rc = socketInitialize(&sockInitConf);
    if(R_FAILED(rc)) fatalSimple(rc);
}

void __appExit(void)
{
    smExit();
}

int main()
{
    svcSleepThread(4'000'000'000);
    flora::InitializeLogging();
    
    auto manager = new flora::ServerManager(2);

    manager->AddWaitable(new ServiceServer<flora::ILogService>("flora", 0x20));

    manager->Process();
    delete manager;

    return 0;
}