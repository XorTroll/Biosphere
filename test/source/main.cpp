#include <biosphere>
using namespace bio;

#include <fstream>
#include <cstdio>
#include <cstring>
#include <iostream>

extern "C"
{
    u64 __bio_svc_GetSystemTick();
}

u64 __bio_crt0_GetHeapSize()
{
    return 0x10000000; // Allow other applets to boot!
}

Result aeTest()
{
    auto [res, ae] = ipc::ServiceSession::CreateDomain("appletAE"); // Gets service and calls ConvertToDomain()
    BIO_TRY(res);

    std::shared_ptr<ipc::Session> ae_ilap;
    // appletAE->OpenLibraryAppletProxyOld
    BIO_TRY(ae->ProcessRequest<200>(ipc::InProcessId(), ipc::InRaw<u64>(0), ipc::InHandle<ipc::HandleMode::Copy>(svc::CurrentProcessPseudoHandle), ipc::OutSession<0, ipc::Session>(ae_ilap)));

    std::shared_ptr<ipc::Session> ae_ilac;
    // ILibraryAppletProxy->GetLibraryAppletCreator
    BIO_TRY(ae_ilap->ProcessRequest<11>(ipc::OutSession<0, ipc::Session>(ae_ilac)));

    std::shared_ptr<ipc::Session> ae_ilaa;
    // ILibraryAppletCreator->CreateLibraryApplet (playerSelect's AppletId = 0x10)
    BIO_TRY(ae_ilac->ProcessRequest<0>(ipc::InRaw<u32>(0x10), ipc::InRaw<u32>(0), ipc::OutSession<0, ipc::Session>(ae_ilaa)));

    printf("Services opened...");

    struct CommonArgs
    {
        u32 version;
        u32 size;
        u32 api_version;
        i32 theme_color;
        u8 unk2;
        u8 pad[7];
        u64 tick;
    };

    CommonArgs args = {};
    memset(&args, 0, sizeof(args));
    args.version = 1;
    args.size = sizeof(CommonArgs);
    args.tick = __bio_svc_GetSystemTick();

    std::shared_ptr<ipc::Session> ae_storage_common;
    // ILibraryAppletCreator->CreateStorage
    BIO_TRY(ae_ilac->ProcessRequest<10>(ipc::InRaw<u64>(sizeof(CommonArgs)), ipc::OutSession<0, ipc::Session>(ae_storage_common)));

    printf("Create storage...");

    std::shared_ptr<ipc::Session> ae_storage_common_accessor;
    // IStorage->Open
    BIO_TRY(ae_storage_common->ProcessRequest<0>(ipc::OutSession<0, ipc::Session>(ae_storage_common_accessor)));

    printf("Open accessor...");

    u64 bufsize = 0;
    BIO_TRY(ae_storage_common_accessor->QueryPointerBufferSize(bufsize));
    printf("Pointer buffer size: 0x%llX", bufsize);
    // IStorageAccessor->Write
    BIO_TRY(ae_storage_common_accessor->ProcessRequest<10>(ipc::InRaw<i64>(0), ipc::InSmartBuffer(&args, sizeof(CommonArgs), 0, bufsize)));

    printf("Write...");

    ae_storage_common_accessor->Close();

    // ILibraryAppletAccessor->PushInData
    BIO_TRY(ae_ilaa->ProcessRequest<100>(ipc::InSession<ipc::Session>(ae_storage_common)));

    ae_storage_common->Close();

    printf("Push...");

    static const size_t datasize = 0xa0;

    u8 data[datasize] = {0};
    data[0x96] = 1;

    std::shared_ptr<ipc::Session> ae_storage_data;
    // ILibraryAppletCreator->CreateStorage
    BIO_TRY(ae_ilac->ProcessRequest<10>(ipc::InRaw<u64>(datasize), ipc::OutSession<0, ipc::Session>(ae_storage_data)));

    printf("Create storage...");

    std::shared_ptr<ipc::Session> ae_storage_data_accessor;
    // IStorage->Open
    BIO_TRY(ae_storage_data->ProcessRequest<0>(ipc::OutSession<0, ipc::Session>(ae_storage_data_accessor)));

    printf("Open accessor...");

    BIO_TRY(ae_storage_data_accessor->QueryPointerBufferSize(bufsize));
    printf("Pointer buffer size [2]: 0x%llX", bufsize);
    // IStorageAccessor->Write
    BIO_TRY(ae_storage_data_accessor->ProcessRequest<10>(ipc::InRaw<i64>(0), ipc::InSmartBuffer(data, datasize, 0, bufsize)));

    printf("Write...");

    ae_storage_data_accessor->Close();

    // ILibraryAppletAccessor->PushInData
    BIO_TRY(ae_ilaa->ProcessRequest<100>(ipc::InSession<ipc::Session>(ae_storage_data)));

    ae_storage_data->Close();

    printf("Push...");

    std::shared_ptr<os::Event> ilaa_state_changed_ev;
    // ILibraryAppletAccessor->GetAppletStateChangedEvent
    BIO_TRY(ae_ilaa->ProcessRequest<0>(ipc::OutEvent<0, true>(ilaa_state_changed_ev)));

    // ILibraryAppletAccessor->Start
    BIO_TRY(ae_ilaa->ProcessRequest<10>());

    printf("Start!");

    BIO_TRY(ilaa_state_changed_ev->Wait(UINT64_MAX));

    return 0;
}

int main(int argc, char **argv)
{
    auto res = aeTest();
    if(res.IsFailure())
    {
        err::FatalAssertionFunction(res); // Will fatal with the error
    }
    return 0;
}