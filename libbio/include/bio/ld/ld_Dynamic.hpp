
#pragma once
#include <bio/bio_Types.hpp>
#include <vector>
#include <elf.h>
#include <bio/ld/ld_Results.hpp>
#include <bio/ro/ro_Service.hpp>
#include <functional>

namespace bio::ld
{
    struct ModuleHeader
    {
        u32 magic;
        u32 dynamic;
        u32 bss_start;
        u32 bss_end;
        u32 unwind_start;
        u32 unwind_end;
        u32 module_object;
    };

    enum class ModuleState
    {
        Invalid,
        Queued,
        Scanned,
        Relocated,
        Initialized,
        Finalized,
        Unloaded
    };

    struct ModuleInput
    {
        char *name;
        void *base;
        void *nro;
        void *nrr;
        void *bss;
        void *loader_data;
        bool is_global;
        bool has_run_basic_relocations;
    };

    struct ModuleBlock
    {
        ModuleState state;
        int ref_count;
        ModuleInput input;
        std::vector<ModuleBlock*> dependencies;
        void *address;
        Elf64_Dyn *dynamic;
        Elf64_Sym *symtab;
        const char *strtab;
        uint32_t *hash;
    };

    Result Initialize();
    bool IsInitialized();
    void Finalize();
    std::shared_ptr<ro::Service> &GetRoSession();

    Result GetLatestDlResult();

    class Module
    {
        public:
            Module(ModuleBlock *raw);
            ~Module();
            ResultWith<void*> ResolveSymbolPtr(const char *name);

            template<typename F>
            ResultWith<F> ResolveSymbol(const char *name)
            {
                auto [res, ptr] = ResolveSymbolPtr(name);
                return MakeResultWith(res, reinterpret_cast<F>(ptr));
            }

        private:
            ModuleBlock *raw_module;
    };

    static constexpr u32 NRO0 = 0x304F524E;
    static constexpr u32 NRR0 = 0x3052524E;
    static constexpr u32 MOD0 = 0x30444F4D;

    ResultWith<std::shared_ptr<Module>> LoadModule(const char *path, bool global);
}