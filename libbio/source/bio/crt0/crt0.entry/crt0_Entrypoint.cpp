#include <cstring>
#include <cstdint>
#include <string>
#include <bio/bio_Types.hpp>
#include <elf.h>
#include <sys/time.h>
#include <sys/lock.h>
#include <sys/reent.h>
#include <bio/ld/ld_Dynamic.hpp>
#include <bio/os/os_Thread.hpp>
#include <bio/os/os_TLS.hpp>
#include <bio/svc/svc_Base.hpp>
#include <bio/ipc/ipc_Request.hpp>
#include <bio/log/log_Logging.hpp>
#include <errno.h>
#include <bio/sm/sm_Port.hpp>
#include <bio/err/err_Assertion.hpp>
#include <sys/stat.h>
#include <bio/env/env_HomebrewABI.hpp>
#include <bio/os/os_Memory.hpp>
#include <bio/os/os_Finalize.hpp>
#include <bio/fs/fs_FspImpl.hpp>

extern "C"
{
	extern bio::u8 __bio_crt0_ExecutableFormat;

    int __bio_crt0_Entrypoint(void *config, bio::u64 thread_handle, void *aslr);
}

// Keep global variables for heap address and size, declared here
extern void *global_HeapAddress;
extern size_t global_HeapSize;

// Making relocation function weak wouldn't be very useful
// Grabbed from libnx dynamic.c
void __bio_crt0_Relocate(bio::u8 *base, Elf64_Dyn *dyn)
{
    const Elf64_Rela* rela = NULL;
	bio::u64 relasz = 0;

	for(; dyn->d_tag != DT_NULL; dyn++)
	{
		switch(dyn->d_tag)
		{
			case DT_RELA:
				rela = (const Elf64_Rela*)(base + dyn->d_un.d_ptr);
				break;
			case DT_RELASZ:
				relasz = dyn->d_un.d_val / sizeof(Elf64_Rela);
				break;
		}
	}

	if(rela == NULL) return;

	for(; relasz--; rela++)
	{
		switch(ELF64_R_TYPE(rela->r_info))
		{
			case R_AARCH64_RELATIVE:
			{
				bio::u64* ptr = (bio::u64*)(base + rela->r_offset);
				*ptr = (bio::u64)base + rela->r_addend;
				break;
			}
		}
	}
}

extern bio::os::VirtualRegion global_AddressSpace;
extern bio::os::VirtualRegion global_Regions[4];

bio::Result __bio_crt0_ProcessRegion(bio::Out<bio::os::VirtualRegion> region, bio::u64 id_0, bio::u32 id_1)
{
	bio::u64 info;
	auto res = bio::svc::GetInfo(id_0, 0, bio::svc::CurrentProcessPseudoHandle, info);
	if(res.IsSuccess())
	{
		bio::u64 size;
		res = bio::svc::GetInfo(id_1, 0, bio::svc::CurrentProcessPseudoHandle, size);
		if(res.IsSuccess())
		{
			((bio::os::VirtualRegion&)region).start = info;
			((bio::os::VirtualRegion&)region).end = info + size;
		}
	}
	return res;
}

char **global_Argv = NULL;
int global_Argc = 0;
void *global_NroRomImage = NULL;
bio::os::ThreadBlock global_MainThreadBlock;

void __bio_crt0_ProcessArgv(void *argv_ptr)
{
	if(argv_ptr == NULL) return;
	size_t arg_base_size = 0x400;

	std::vector<char*> vec_argv;
	char *argvstr = (char*)argv_ptr;

	bool quotes_open = false;
	std::string tmp_str;

	for(int i = 0; argvstr[i] != '\0'; i++)
	{
		char ch = argvstr[i];
		if(ch == '"')
		{
			if(!quotes_open)
			{
				quotes_open = true;
			}
			else
			{
				char *arg = new char[arg_base_size];
				strcpy(arg, tmp_str.c_str());
				vec_argv.push_back(arg);
				tmp_str = "";
				quotes_open = false;
			}
		}
		else if(ch == ' ')
		{
			if(quotes_open) tmp_str += ch;
			else if((i > 0) && (argvstr[i - 1] != '"'))
			{
				char *arg = new char[arg_base_size];
				strcpy(arg, tmp_str.c_str());
				vec_argv.push_back(arg);
				tmp_str = "";
			}
		}
		else tmp_str += ch;
	}

	if(!tmp_str.empty())
	{
		char *arg = new char[arg_base_size];
		strcpy(arg, tmp_str.c_str());
		vec_argv.push_back(arg);
	}

	if(!vec_argv.empty())
	{
		global_Argc = vec_argv.size();
		global_Argv = vec_argv.data();

		if(__bio_crt0_ExecutableFormat == 1)
		{
			// Check NRO romfs here (see below)
			char *filename = global_Argv[0]; // Path sent by hbloader via argv
			if(strncmp("sdmc:/", filename, 6) == 0)
			{
				// Handle NRO-fs romfs in here (before user code) since that way we ensure user hasn't mounted sd yet, so we mount, read and unmount it, and later bio::fs will use this image if found to mount romfs
				auto res = bio::fs::Initialize();
				if(res.IsSuccess())
				{
					res = bio::fs::MountSdCard("sdmc");
					if(res.IsSuccess())
					{
						struct nro_BaseHeader
						{
							bio::u32 magic;
							bio::u32 unk1;
							bio::u32 size; // We just care of size, so we don't care of the rest of the actual header
						};

						/// Custom assets
						struct nro_AssetSection
						{
							bio::u64 offset;
							bio::u64 size;
						};

						struct nro_AssetHeader
						{
							bio::u32 magic;
							bio::u32 version;
							nro_AssetSection icon;
							nro_AssetSection nacp;
							nro_AssetSection romfs;
						};

						FILE *self = fopen(filename, "rb");
						if(self)
						{
							fseek(self, 0, SEEK_END);
							size_t fullsz = ftell(self);
							rewind(self);
							fseek(self, 0x10, SEEK_SET); // Seek 16 bytes for NroStart shit
							nro_BaseHeader header = {};
							fread(&header, 1, sizeof(nro_BaseHeader), self);
							if(header.size < fullsz)
							{
								fseek(self, header.size, SEEK_SET);
								nro_AssetHeader asheader = {};
								fread(&asheader, 1, sizeof(nro_AssetHeader), self);
								if((asheader.romfs.offset > 0) && (asheader.romfs.size > 0))
								{
									global_NroRomImage = operator new(asheader.romfs.size); // If we round a valid romfs section, allocate and read it
									fseek(self, header.size + asheader.romfs.offset, SEEK_SET);
									fread(global_NroRomImage, 1, asheader.romfs.size, self);
								}
							}
							fclose(self);
						}
					}
					bio::fs::Finalize();
				}
			}
		}
	}
}

// If normal heap function is called this will be set to false, and since user can't change this...
static bool _inner_UserOverrideHeap = true;

// User can redefine this symbol to use a custom heap size, which will be re-set by the SVC even if HBABI specifies heap override.
bio::u64 BIO_WEAK __bio_crt0_GetHeapSize()
{
	_inner_UserOverrideHeap = false;
	return 0x20000000; // Is this an appropriate value?
}

/*

Biosphere's CRT0 entrypoint (only for NSO/executable NRO)

Startup:

1 - Relocation
2 - TOFINISH - HBABI config processing (if NRO)
3 - TOFINISH - memory processing, for virtual memory
4 - thread section processing
5 - heap processing
6 - TOFINISH - argv processing

Execution:

1 - TODO - high level (services?) initialization (not yet decided...)
2 - main()
3 - cleanup of possible IPC/mem leaks
4 - return exit code and exit

*/

// newlib stuff which needs to be initialized
static void __bio_crt0_PrepareNewlib()
{
	// stdout/err buffering
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
}

void BIO_WEAK __bio_crt0_Startup(void *config, bio::u64 thread_handle, void *aslr)
{
	bio::u8 *mod_base = (bio::u8*)aslr;
    bio::ld::ModuleHeader *module = (bio::ld::ModuleHeader*)&mod_base[*(bio::u32*)&mod_base[4]];

	Elf64_Dyn *dynamic = (Elf64_Dyn*) (((uint8_t*)module) + module->dynamic);

    __bio_crt0_Relocate(mod_base, dynamic);
	__bio_crt0_PrepareNewlib();

    if(__bio_crt0_ExecutableFormat == 0) BIO_DEBUG_LOG("%s", "Binary format: NSO");
	else if(__bio_crt0_ExecutableFormat == 1) BIO_DEBUG_LOG("%s", "Binary format: NRO");

	void *argv = NULL;

	if((__bio_crt0_ExecutableFormat == 1) && (config != NULL))
	{
		// Just check config if hbloader launched us, thus NRO
		bio::env::ABIConfigEntry *entry = (bio::env::ABIConfigEntry*)config;
		while(true)
		{
			bio::env::ABIConfigKey key = static_cast<bio::env::ABIConfigKey>(entry->key);
			if(key == bio::env::ABIConfigKey::EOL) break;
			switch(key)
			{
				case bio::env::ABIConfigKey::OverrideHeap:
				{
					global_HeapAddress = (void*)entry->value[0];
					global_HeapSize = (size_t)entry->value[1];
					break;
				}
				case bio::env::ABIConfigKey::Argv:
				{
					argv = (void*)entry->value[1];
					break;
				}
				
			}
			entry++;
		}
	}

	auto res = __bio_crt0_ProcessRegion(global_AddressSpace, 12, 13);
	if(res.IsFailure())
	{
		// Handle 1.0.0!!! TODO
	}
	else __bio_crt0_ProcessRegion(global_Regions[static_cast<bio::u32>(bio::os::Region::Stack)], 14, 15).Assert();
	__bio_crt0_ProcessRegion(global_Regions[static_cast<bio::u32>(bio::os::Region::Heap)], 4, 5).Assert();
	__bio_crt0_ProcessRegion(global_Regions[static_cast<bio::u32>(bio::os::Region::NewStack)], 2, 3).Assert();

	auto th_section = bio::os::GetThreadSection();
	memset(bio::os::GetThreadSection(), 0, 0x200);
	
	memset(&global_MainThreadBlock, 0, sizeof(global_MainThreadBlock));
	global_MainThreadBlock.handle = (bio::u32)thread_handle;
	global_MainThreadBlock.owns_stack = false;
	global_MainThreadBlock.arg = NULL;
	global_MainThreadBlock.pthread = NULL;
	strcpy(global_MainThreadBlock.name, "bio.MainThread");
	_REENT_INIT_PTR(&global_MainThreadBlock.reent);
	bio::os::GetThreadSection()->thread = &global_MainThreadBlock;

	// If hbloader didn't give us a base heap, get the default one
	auto def_heap = __bio_crt0_GetHeapSize();
	if((global_HeapSize == 0) || _inner_UserOverrideHeap)
	{
		global_HeapSize = def_heap;
		bio::svc::SetHeapSize(global_HeapAddress, global_HeapSize).Assert();
	}

	if(__bio_crt0_ExecutableFormat == 0) // NSO specific argv (TODO)
	{

	}

	__bio_crt0_ProcessArgv(argv);
}

// User entrypoint
int main(int argc, char **argv);

int BIO_WEAK __bio_crt0_Execution()
{
	// What shall we initialize here...?
	int ret = main(global_Argc, global_Argv);
	bio::os::CallFinalizeFunctions(); // This should only be called here!
	if(global_NroRomImage != NULL) operator delete(global_NroRomImage); // If we allocated the romfs image from NRO, dispose it
	return ret;
}

// Actual code called by crt0. The other crt0 sub-phases can be redefined by the user, but not this function.

int __bio_crt0_Entrypoint(void *config, bio::u64 thread_handle, void *aslr)
{
    __bio_crt0_Startup(config, thread_handle, aslr);
	return __bio_crt0_Execution();
}