# Biosphere (libbio) - code style

## Modules

Modules are identified by namespaces.

As few as possible inner namespaces and as short as possible namespace names are preferred and suggested.

Simple global namespaces as `bio::sm` for SM-related content is a good example.

## Functions

Functions are named after PascalCase.

"Too" long function names aren't a problem, but it would be better to avoid too long function names.

## Services

Notice the difference between these two terms:

- IPC implementation is just the IPC implementation of the service(s). Only handles IPC commands and interfaces.

- Service wrapper is a more high-level or user-friendly implementation of the IPC implementation.

Some examples: `fsp` as IPC and `fs` as the wrapper, `hid` as IPC and `input` as the wrapper, `ro` as IPC and `ld` as the wrapper... (TODO: add more of these)

### IPC implementations

Must be in a seperate module, preferably named after the service name or a similar one (see `hid`, `sm`, `fsp`, `fatal`...)

The service must be obtained from a class named `Service` inheriting from `bio::ipc::ServiceSession` which must follow this simple structure:

```cpp
class Service
{
    public:
        Service();

        static ResultWith<std::shared_ptr<Service>> Initialize();

        // Other IPC command functions
};
```

The constructor musn't contain any code besides the `ServiceSession` initialization:

```cpp
Service::Service() : ServiceSession("service-name")
{
}
```

However, if the service would need to call a "Initialize"-like IPC command in order to be used (like in `fsp`, `ro`, `sm`...) that initialization call should only be present in the static shared pointer call.

Sample implementations:

```cpp
// If no initialization command is needed, just return the ServiceSession's result + the shared ptr
ResultWith<std::shared_ptr<Service>> Service::Initialize()
{
    auto srv = std::make_shared<Service>();
    return MakeResultWith(srv->GetInitialResult(), std::move(srv));
}

// If a command needs to be called before:
ResultWith<std::shared_ptr<Service>> Service::Initialize()
{
    auto res = srv->GetInitialResult();
    if(res.IsSuccess()) res = srv->ProcessRequest<cmd_id>(args);

    return MakeResultWith(res, std::move(srv));
}
```

#### Command implementations

These are simple functions returning a `bio::Result`, and which may use `bio::Out` wrapper for out parameters. It is preferred everything to be handled on one single line:

```cpp

Result Service::SampleCommand1(int a, Out<char> b) // In -> int, Out -> char
{
    // Notice that to make everything in one single line, Out<> parameters need to be static_cast'd
    return Processrequest<cmd_id>(ipc::InRaw<int>(a), ipc::OutRaw<char>(static_cast<char&>(b)));
}

Result Service::SampleCommand1(char *buf, size_t sz) // InBuffer
{
    // The 0 represents the buffer type, similar to libnx's system.
    return Processrequest<cmd_id>(ipc::InBuffer(buf, sz, 0));
}

Result Service::SampleCommand1(Out<std::shared_ptr<Interface>> out) // Out interface, IPC code already handles whether it is a domain or a regular handle object.
{
    // Like Out this also needs to be static_cast'd. The 0 represents the ID of the handle / object ID to use.
    return Processrequest<cmd_id>(ipc::OutSession<0, Interface>(static_cast<std::shared_ptr<Interface>&>(out)));
}

```

#### Sessions

Non-service sessions (returned from other sessions) are similar to a Service class, but this ones use `bio::ipc::Session`'s default constructor and destructor, and obviously inherit from it. IPC command functions are similar.

### Service wrappers

Wrappers' format will depend on it's purpose, but for this using a namespace and global Initialize/Finalize functions is suggested, with an internal shared pointer of the wrappped service.

## Naming

- As said above, functions are named after PascalCase.

```cpp
Result DoThisAndThat();
```

- Global (inner or actually accessable by `extern`) are named after PascalCase too, after their corresponding pre-names. (see below)

```cpp
static int _inner_MyIndex;
extern void *global_SomePtr;
```

- Types are also names after PascalCase. An exception can be POD or base types, but probably this exception is reserved to `bio_Types.hpp` types.

```cpp
class Service
{
    // ...
};

struct SomeOtherType
{
    // ...
};
```

- Macros must be uppercase, separated by underscores, and preceded by `BIO_`. If the macro corresponds to an specific module, then the macro should contain the module name after `BIO_`.

```cpp
#define BIO_YAY "Yay!"
#define BIO_SM_SERVICENAME "sm:"
```

- Class or struct members, function parameters or (preferably) temp variables should be named after snake_case. In this case it's the opposite to the other naming conventions: snake_case, usually all abbreviations...

```cpp
struct DemoStruct
{
    int idx;
    char ref_char;
    bool is_check_done;

    void Check(int &ref, void *ptr, const char *txt);
};
```

## Inner code

It is preferred that code only used inside the current source file is marked as `static`, and preceded by `_inner_`.

```cpp
static std::shared_ptr<Session> _inner_MySession;

static void _inner_EnsureInitialized()
{

}
```

## Global "extern" variables

Global variables must be preceded by `global_`, and accessed via `extern` by other sources. libbio's basic global variables (heap, argc/argv...) aren't within any namespace, but this isn't mandatory.

## Out parameters

For IPC commands / SVCs, always return `void`/`Result` and use `bio::Out<>` helper.

For static object creators (seen in `bio::ld::Module`, `bio::os::Thread` and service IPC session classes), which always return a single object + a Result, or in general other cases, using `bio::ResultWith<Types>` is recommended.