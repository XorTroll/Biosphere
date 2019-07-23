#include <bio/ld/ld_Dynamic.hpp>
#include <elf.h>
#include <cstdio>
#include <malloc.h>
#include <sha256.h>
#include <bio/ro/ro_Service.hpp>
#include <cstring>
#include <dlfcn.h>
#include <cstdarg>

namespace bio::ld
{
    static std::shared_ptr<ro::Service> _inner_RoSession;
    static bool _inner_Initialized = false;

    struct _inner_Elf64_CustomRela
    {
        u64 r_offset;
        u32 r_reloc_type;
        u32 r_symbol;
        u64 r_addend;
    };

    struct _inner_Elf64_CustomRel
    {
        u64 r_offset;
        u32 r_reloc_type;
        u32 r_symbol;
    };

    static Result _inner_ELFDynamic_FindValue(Elf64_Dyn *dynamic, i64 tag, u64 *value)
    {
        u64 *found = NULL;
        *value = 0;
        for(; dynamic->d_tag != DT_NULL; dynamic++)
        {
            if(dynamic->d_tag == tag)
            {
                if(found != NULL) return ResultDuplicatedDtEntry;
                else found = &dynamic->d_un.d_val;
            }
        }
        if(found == NULL) return ResultMissingDtEntry;
        *value = *found;
        return 0;
    }

    static Result _inner_ELFDynamic_FindOffset(Elf64_Dyn *dynamic, i64 tag, void **value, void *aslr_base)
    {
        u64 intermediate;
        auto res = _inner_ELFDynamic_FindValue(dynamic, tag, &intermediate);
        *value = (u8*)aslr_base + intermediate;
        return res;
    }

    static u64 _inner_ELF_HashString(const char *name)
    {
        u64 h = 0;
        u64 g;
        while(*name)
        {
            h = (h << 4) + *(const u8*)name++;
            if((g = (h & 0xf0000000)) != 0)
            {
                h ^= g >> 24;
            }
            h&= ~g;
        }
        return h;
    }

    static Result _inner_Load(const char *path, ModuleBlock *mod)
    {
        Result res = ResultInvalidInputNro;
        auto f = fopen(path, "rb");
        if(f)
        {
            fseek(f, 0, SEEK_END);
            auto filesz = ftell(f);
            rewind(f);
            void *nro = memalign(0x1000, filesz);
            if(nro == NULL)
            {
                fclose(f);
                return res;
            }

            auto read = fread(nro, 1, filesz, f);
            if(read != filesz)
            {
                free(nro);
                fclose(f);
                return res;
            }
            fclose(f);

            u32 *nrr = (u32*)memalign(0x1000, 0x1000);
            if(nrr == NULL)
            {
                free(nro);
                return res;
            }
            nrr[0] = NRR0;
            nrr[(0x338 >> 2) + 0] = 0x1000;
            nrr[(0x340 >> 2) + 0] = 0x350;
            nrr[(0x340 >> 2) + 1] = 1; // NRO count

            u64 appid = 0x010000000000100D;
            svc::GetInfo(18, 0, svc::CurrentProcessPseudoHandle, appid);
            *(u64*)&((u8*)nrr)[0x330] = appid;

            SHA256_CTX ctx; // TODO: port libnx's hw-accelerated
            sha256_init(&ctx);
            sha256_update(&ctx, (u8*) nro, filesz);
            sha256_final(&ctx, (u8*) &nrr[0x350 >> 2]);

            u32 bss_sz = *(u32*)((u8*)nro + 0x38);
            void *bss = memalign(0x1000, bss_sz);
            if(bss == NULL)
            {
                free(nro);
                free(nrr);
                return res;
            }

            u64 nro_addr = 0;

            res = _inner_RoSession->LoadNrr(nrr, 0x1000);
            if(res.IsSuccess()) res = _inner_RoSession->LoadNro(nro, filesz, bss, bss_sz, nro_addr);

            if(res.IsSuccess())
            {
                mod->input.nro = nro;
                mod->input.nrr = nrr;
                mod->input.bss = bss;
                mod->input.base = (void*)nro_addr;
            }
        }
        return res;
    }

    static Result _inner_Scan(ModuleBlock *mod)
    {
        u8 *module_base = (u8*)mod->input.base;
        ModuleHeader *mod_header = (ModuleHeader*)&module_base[*(uint32_t*)&module_base[4]];
        Elf64_Dyn *dynamic = (Elf64_Dyn*)((u8*)mod_header + mod_header->dynamic);
        mod->dynamic = dynamic;

        if(mod_header->magic != MOD0) return ResultInvalidInputNro;

        auto res = _inner_ELFDynamic_FindOffset(dynamic, DT_HASH, (void**)&mod->hash, module_base);
        if(res.IsFailure() && (res != ResultMissingDtEntry)) return res;

        res = _inner_ELFDynamic_FindOffset(dynamic, DT_STRTAB, (void**)&mod->strtab, module_base);
        if(res.IsFailure() && (res != ResultMissingDtEntry)) return res;

        res = _inner_ELFDynamic_FindOffset(mod->dynamic, DT_SYMTAB, (void**)&mod->symtab, mod->input.base);
        if(res.IsFailure() && (res != ResultMissingDtEntry)) return res;

        u64 syment = 0;
        res = _inner_ELFDynamic_FindValue(mod->dynamic, DT_SYMENT, &syment);
        if(res.IsSuccess())
        {
            if(syment != sizeof(Elf64_Sym)) return ResultInvalidSymEnt;
        }
        else if(res != ResultMissingDtEntry) return res;

        for(Elf64_Dyn *walker = dynamic; walker->d_tag != DT_NULL; walker++)
        {
            if(walker->d_tag == DT_NEEDED)
            {
                ModuleBlock *dep = new ModuleBlock;
                res = _inner_Load(mod->strtab + walker->d_un.d_val, dep);
                if(res.IsSuccess()) mod->dependencies.push_back(dep);
            }
        }

        mod->state = ModuleState::Scanned;
        return 0;
    }

    static Result _inner_TryResolveSymbol(ModuleBlock *try_mod, const char *find_name, u64 find_name_hash, Elf64_Sym **def, ModuleBlock **defining_module, bool require_global)
    {
        if(require_global && !try_mod->input.is_global) return ResultCouldNotResolveSymbol;
        if(try_mod->symtab == NULL) return ResultCouldNotResolveSymbol;
        if(try_mod->strtab == NULL) return ResultCouldNotResolveSymbol;
        if(try_mod->hash != NULL)
        {
            u32 nbucket = try_mod->hash[0];
            u32 nchain = try_mod->hash[1];
            BIO_IGNORE(nchain);
            u32 index = try_mod->hash[2 + (find_name_hash % nbucket)];
            u32 *chains = try_mod->hash + 2 + nbucket;
            while((index != 0) && (strcmp(find_name, try_mod->strtab + try_mod->symtab[index].st_name) != 0)) index = chains[index];
            if(index == STN_UNDEF) return ResultCouldNotResolveSymbol;
            Elf64_Sym *sym = &try_mod->symtab[index];
            if(sym->st_shndx == SHN_UNDEF) return ResultCouldNotResolveSymbol;
            *def = sym;
            defining_module = &try_mod;
            return 0;
        }
        return ResultCouldNotResolveSymbol;
    }

    static Result _inner_ResolveLoadSymbol(ModuleBlock *find_mod, const char *find_name, Elf64_Sym **def, ModuleBlock **defining_module)
    {
        u64 hash = _inner_ELF_HashString(find_name);
        return _inner_TryResolveSymbol(find_mod, find_name, hash, def, defining_module, false);
    }

    static Result _inner_ResolveDependencySymbol(ModuleBlock *find_mod, const char *find_name, Elf64_Sym **def, ModuleBlock **defining_module)
    {
        u64 find_name_hash = _inner_ELF_HashString(find_name);
        auto res = _inner_TryResolveSymbol(find_mod, find_name, find_name_hash, def, defining_module, false);
        if(res != ResultCouldNotResolveSymbol) return res;

        for(u32 i = 0; i < find_mod->dependencies.size(); i++)
        {
            res = _inner_TryResolveSymbol(find_mod->dependencies[i], find_name, find_name_hash, def, defining_module, false);
            if(res == ResultCouldNotResolveSymbol) continue;
            else return res;
        }

        for(u32 i = 0; i < find_mod->dependencies.size(); i++)
        {
            res = _inner_ResolveDependencySymbol(find_mod->dependencies[i], find_name, def, defining_module);
            if(res == ResultCouldNotResolveSymbol) continue;
            else return res;
        }

        return ResultCouldNotResolveSymbol;
    }

    static Result _inner_RunRelocationTable(ModuleBlock *mod, u32 offset_tag, u32 size_tag)
    {
        void *raw_table;
        Elf64_Dyn *dynamic = mod->dynamic;
        auto res = _inner_ELFDynamic_FindOffset(dynamic, offset_tag, &raw_table, mod->input.base);
        if(res == ResultMissingDtEntry) return 0;
        if(res.IsFailure()) return res;

        u64 table_size = 0;
        u64 table_type = offset_tag;
        res = _inner_ELFDynamic_FindValue(dynamic, size_tag, &table_size);
        if(res.IsFailure()) return res;

        if(offset_tag == DT_JMPREL)
        {
            res = _inner_ELFDynamic_FindValue(dynamic, DT_PLTREL, &table_type);
            if(res.IsFailure()) return res;
        }

        u64 ent_size = 0;
        switch(table_type)
        {
            case DT_RELA:
                res = _inner_ELFDynamic_FindValue(dynamic, DT_RELAENT, &ent_size);
                if(res == ResultMissingDtEntry) ent_size = sizeof(Elf64_Rela);
                else if(res.IsSuccess() && (ent_size != sizeof(Elf64_Rela))) return ResultInvalidRelocEnt;
                else if(res.IsFailure()) return res;
                break;
            case DT_REL:
                res = _inner_ELFDynamic_FindValue(dynamic, DT_RELENT, &ent_size);
                if(res == ResultMissingDtEntry) ent_size = sizeof(Elf64_Rel);
                else if(res.IsSuccess() && (ent_size != sizeof(Elf64_Rel))) return ResultInvalidRelocEnt;
                else if(res.IsFailure()) return res;
                break;
            default:
                return ResultInvalidRelocTableType;
        }

        if((table_size % ent_size) != 0) return ResultInvalidRelocTableSize;

        for(size_t offset = 0; offset < table_size; offset += ent_size)
        {
            _inner_Elf64_CustomRela rela;
            switch(table_type)
            {
                case DT_RELA:
                    rela = *(_inner_Elf64_CustomRela*)((u8*)raw_table + offset);
                    break;
                case DT_REL:
                {
                    _inner_Elf64_CustomRel rel = *(_inner_Elf64_CustomRel*)((u8*)raw_table + offset);
                    rela.r_offset = rel.r_offset;
                    rela.r_reloc_type = rel.r_reloc_type;
                    rela.r_symbol = rel.r_symbol;
                    break;
                }
            }

            void *symbol = NULL;
            ModuleBlock *defining_module = mod;
            if(rela.r_symbol != 0)
            {
                if(mod->symtab == NULL) return ResultNeedsSymTab;
                if(mod->strtab == NULL) return ResultNeedsStrTab;
                Elf64_Sym *sym = &mod->symtab[rela.r_symbol];
                
                Elf64_Sym *def;
                res = _inner_ResolveLoadSymbol(mod, mod->strtab + sym->st_name, &def, &defining_module);
                if(res.IsFailure()) continue;
                symbol = (u8*)defining_module->input.base + def->st_value;
            }
            void *delta_symbol = defining_module->input.base;
            
            switch(rela.r_reloc_type)
            {
                case 257:
                case 1025:
                case 1026:
                {
                    void **target = (void**)((u8*)mod->input.base + rela.r_offset);
                    if(table_type == DT_REL) rela.r_addend = (u64)*target;
                    *target = (u8*)symbol + rela.r_addend;
                    break;
                }
                case 1027:
                {
                    if(!mod->input.has_run_basic_relocations)
                    {
                        void **target = (void**)((u8*)mod->input.base + rela.r_offset);
                        if(table_type == DT_REL) rela.r_addend = (u64)*target;
                        *target = (u8*)delta_symbol + rela.r_addend;
                    }
                    break;
                }
                default:
                    return ResultUnrecognizedRelocType;
            }
        }
        
        return 0;
    }

    static Result _inner_Relocate(ModuleBlock *mod)
    {
        auto res = _inner_RunRelocationTable(mod, DT_RELA, DT_RELASZ);
        if(res.IsFailure()) return res;
        res = _inner_RunRelocationTable(mod, DT_REL, DT_RELSZ);
        if(res.IsFailure()) return res;
        res = _inner_RunRelocationTable(mod, DT_JMPREL, DT_PLTRELSZ);
        mod->state = ModuleState::Relocated;
        return res;
    }

    static Result _inner_Initialize(ModuleBlock *mod)
    {
        if(mod->state != ModuleState::Relocated) return ResultInvalidModuleState;
        
        void (**init_array)(void);
        size_t init_array_size;

        auto res = _inner_ELFDynamic_FindOffset(mod->dynamic, DT_INIT_ARRAY, (void**)&init_array, mod->input.base);
        if(res.IsSuccess())
        {
            res = _inner_ELFDynamic_FindValue(mod->dynamic, DT_INIT_ARRAYSZ, &init_array_size);
            if(res.IsFailure()) return res;
            for(size_t i = 0; i < (init_array_size / sizeof(init_array[0])); i++) init_array[i]();
        }
        else if(res != ResultMissingDtEntry) return res;

        mod->state = ModuleState::Initialized;
        return 0;
    }

    static Result _inner_Finalize(ModuleBlock *mod)
    {
        if(mod->state != ModuleState::Initialized) return ResultInvalidModuleState;
        
        void (**fini_array)(void);
        size_t fini_array_size;

        auto res = _inner_ELFDynamic_FindOffset(mod->dynamic, DT_FINI_ARRAY, (void**)&fini_array, mod->input.base);
        if(res.IsSuccess())
        {
            res = _inner_ELFDynamic_FindValue(mod->dynamic, DT_FINI_ARRAYSZ, &fini_array_size);
            if(res.IsFailure()) return res;
            for(size_t i = 0; i < (fini_array_size / sizeof(fini_array[0])); i++) fini_array[i]();
        }
        else if(res != ResultMissingDtEntry) return res;

        mod->state = ModuleState::Finalized;
        return 0;
        fail:
        return res;
    }

    static Result _inner_Destroy(ModuleBlock *mod);

    static void _inner_Decref(ModuleBlock *mod)
    {
        mod->ref_count--;
        if(mod->ref_count == 0) _inner_Destroy(mod);
    }

    static Result _inner_Unload(ModuleBlock *mod)
    {
        Result res;
        if(mod != NULL)
        {
            if(mod->input.base != NULL)
            {
                free(mod->input.nrr);
                free(mod->input.nro);
                free(mod->input.bss);
                res = _inner_RoSession->UnloadNro(mod->input.base);
                if(res.IsSuccess()) res = _inner_RoSession->UnloadNrr(mod->input.nrr);
            }
            delete mod;
        }
        return res;
    }

    static Result _inner_Destroy(ModuleBlock *mod)
    {
        bio::Result res;
        if(mod->state == ModuleState::Initialized)
        {
            res = _inner_Finalize(mod);
            if(res.IsSuccess()) res = _inner_Unload(mod);
        }

        for(u32 i = 0; i < mod->dependencies.size(); i++) _inner_Decref(mod->dependencies[i]);

        return res;
    }

    Result Initialize()
    {
        if(!_inner_Initialized)
        {
            _inner_RoSession = ro::Service::Initialize();
            _inner_Initialized = true;
        }
        return 0;
    }

    bool IsInitialized()
    {
        return _inner_Initialized;
    }

    void Finalize()
    {
        if(_inner_Initialized)
        {
            _inner_RoSession.reset();
            _inner_Initialized = false;
        }
    }

    std::shared_ptr<ro::Service> &GetRoSession()
    {
        return _inner_RoSession;
    }
}

static char dlerror_buf[0x200];
static bio::Result dl_latest_res;

static void _inner_UpdateDlerror(const char *error, ...)
{
    va_list vargs;
    va_start(vargs, error);
    memset(dlerror_buf, 0, 0x200);
    vsprintf(dlerror_buf, error, vargs);
    va_end(vargs);
}

void *dlopen(const char *path, int flags)
{
	bio::ld::ModuleBlock *module = new bio::ld::ModuleBlock;
	if(module == NULL)
    {
        _inner_UpdateDlerror("Could not allocate memory");
		return module;
	}
	if(path == NULL)
    {
		_inner_UpdateDlerror("Invalid input NRO path");
		return NULL;
	}

    strcpy(module->input.name, path);
    module->input.has_run_basic_relocations = false;
    module->input.is_global = (flags & RTLD_GLOBAL);

    auto res = bio::ld::_inner_Load(path, module);

    if(res.IsFailure())
    {
        _inner_UpdateDlerror("Library load failed: 0x%X", (bio::u32)res);
        dl_latest_res = res;
        delete module;
		return NULL;
    }

    module->ref_count = 1;
    module->state = bio::ld::ModuleState::Queued;

    res = bio::ld::_inner_Scan(module);
    if(res.IsFailure())
    {
        _inner_UpdateDlerror("Library scan failed: 0x%X", (bio::u32)res);
        dl_latest_res = res;
        delete module;
		return NULL;
    }

    res = bio::ld::_inner_Relocate(module);
    if(res.IsFailure())
    {
        _inner_UpdateDlerror("Library relocation failed: 0x%X", (bio::u32)res);
        dl_latest_res = res;
        delete module;
		return NULL;
    }

    res = bio::ld::_inner_Initialize(module);
    if(res.IsFailure())
    {
        _inner_UpdateDlerror("Library object initialization failed: 0x%X", (bio::u32)res);
        dl_latest_res = res;
        delete module;
		return NULL;
    }

	return module;
}

void *dlsym(void *ptr, const char *symbol)
{
	bio::ld::ModuleBlock *module = (bio::ld::ModuleBlock*)ptr;

	Elf64_Sym *def;
	bio::ld::ModuleBlock *def_mod;

	auto res = bio::ld::_inner_ResolveDependencySymbol(module, symbol, &def, &def_mod);
	if(res.IsFailure())
    {
		_inner_UpdateDlerror("Symbol resolving failed: 0x%X", (bio::u32)res);
        dl_latest_res = res;
		return NULL;
	}
    return (bio::u8*)module->input.base + def->st_value;
}

int dlclose(void *ptr)
{
	bio::ld::ModuleBlock *module = (bio::ld::ModuleBlock*)ptr;
	if(module != NULL)
    {
		bio::ld::_inner_Decref(module);
		delete module;
	}
	return 0;
}

char *dlerror(void)
{
	return dlerror_buf;
}

namespace bio::ld
{
    Module::Module(ModuleBlock *raw) : raw_module(raw)
    {
    }

    Module::~Module()
    {
        dlclose(raw_module);
    }

    void *Module::ResolveSymbolPtr(const char *name)
    {
        return dlsym(raw_module, name);
    }

    Result LoadModule(const char *path, bool global, Out<std::shared_ptr<Module>> module)
    {
        int flags = RTLD_LOCAL;
        if(global) flags = RTLD_GLOBAL;

        bio::ld::ModuleBlock *raw_module = (bio::ld::ModuleBlock*)dlopen(path, flags);
        if(raw_module != NULL)
        {
            (std::shared_ptr<Module>&)module = std::make_shared<Module>(raw_module);
            return dl_latest_res;
        }

        return 0;
    }
}