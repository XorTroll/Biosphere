# Biosphere

> Bringing life to Nintendo Switch homebrew!

## **IMPORTANT!** this project is no longer continued.

Biosphere (previously a devkitA64 homebrew library) is a homebrew toolchain / development kit using LLVM and clang.

## WIP status (TODO)

- Libraries boot and work fine, but libbio still lacks some essential features:

  - Graphics support, console logs to `fauna-flora` system or via SVC

  - RomFs support, and finish the filesystem implementation, which is not fully implemented.

  - Sockets via bsd services

  - Finish CRT0 - left stuff: HBABI key/value system, argv for NSOs, address space for 1.0.0

  - Current assertion system seems to be badly implemented, and needs a rewrite

  - Newlib implementation, where many basic calls aren't present/are stubbed

- Examples work (except `rom`, since NRO-based RomFs isn't yet implemented)

### Status of fauna/flora

- Both seem to work fine in general

- If system sleeps and is reawaken a reboot is needed for fauna to reconnect

- If fauna is closed, same as above ^

## libbio

Biosphere's high-level homebrew library, with no C and almost 100% C++ (except some bits of assembly).

### Features

- Taking advantage of C++17 for some nice things, such as structured binding:

```cpp
auto [res, fsp_session] = bio::fsp::Service::Initialize();
if(res.IsSuccess())
{
    // Do things with fsp session ( fsp_session->OpenSdCardFileSystem(...) )
}
```

- Simple and easy-to-use IPC interface focusing on simplicity, based on libtransistor's one, and internal IPC handling ported from libnx's source:

```cpp
// Sample, call psm->GetBatteryChargePrecentage()

// libnx way

Service psm;
Result rc = smGetService(&psm, "psm");
if(R_SUCCEEDED(rc))
{
    IpcCommand c;
    ipcInitialize(&c);

    struct InRaw
    {
        u64 magic;
        u64 cmdid;
    } *in_raw = (InRaw*)ipcPrepareHeader(%c, sizeof(*in_raw));

    in_raw->magic = SFCI_MAGIC;
    in_raw->cmdid = 0;

    rc = serviceIpcDispatch(&psm);
    if(R_SUCCEEDED(rc))
    {
        IpcParsedCommand r;
        ipcParse(&r);

        struct OutRaw
        {
            u64 magic;
            u64 res;
            u32 out_percentage;
        } *out_raw = (OutRaw*)r.Raw;

        rc = out_raw->res;
        if(R_SUCCEEDED(rc))
        {
            u32 battery = resp->out_percentage;
        }
    }
}

// Biosphere way

bio::ipc::ServiceSession psm("psm");
auto rc = psm.GetInitialResult();
if(rc.IsSuccess())
{
    u32 battery;
    rc = psm.ProcessRequest<0>(bio::ipc::OutRaw<u32>(battery));
}
```

## fauna and flora

`fauna` and `flora` are the two tools used for console <-> PC logging.

### fauna

Fauna is the PC tool which will wait for data sent by `flora` process.

### flora

Flora is the background process (and service) which will be used to log data to `fauna` PC tool via TCP.

## Building

Clone with `git clone --recurse-submodules` and run `make`. This should build every default library (newlib, libcxx), libbio and other projects.

### Required dependencies (for building the toolchain)

- Ubuntu/Linux is suggested. For Windows users, WSL should be used.  I (XorTroll) personally compile in Windows, with WSL/Ubuntu.

- Install `llvm`, `llvm-config`, `clang`, `cmake` and `make` (if problems happen, try to install `autoconf`?).

If the build succeeded, the built Biosphere "devkit" will be located at `/Biosphere`. you should see a script named `SetBiosphereRoot.sh` in that directory. After compiling, you can move the output directory anywhere you wish to. After you've located it, run the script and will set Biosphere's env var in there.

## Developing homebrew

Download the latest release. Will contain the built toolchain inside `Biosphere` directory.

Set `BIOSPHERE_ROOT` environment variable to the directory.

In order to build homebrew, check the example projects in `examples`.

**IMPORTANT!** if you happen to move Biosphere's main directory, re-run the root `SetBiosphereRoot.sh` script to re-set the env var if you would like to use Biosphere ever again ;)

## Credits

- **libtransistor** project and team for the base of using LLVM for homebrew development. This project is a fork of `libtransistor-base` in the end, and much `libbio` content (such as `ld` module) is ported from libtransistor :P

- **linkle** project, since it's the tool used for NRO/NSO generation.

- **switchbrew** and **libnx** as a base for resources, since they also are the base for the [old Biosphere project](https://github.com/XorTroll/Biosphere-old), whose code is being ported/reused for this new project.
