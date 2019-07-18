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

// If normal heap function is called this will be set to false, and since user can't change this...
static bool _inner_UserOverrideHeap = true;

// User can redefine this symbol to use a custom heap size, which will be re-set by the SVC even if HBABI specifies heap override.
bio::u64 BIO_WEAK __bio_crt0_GetHeapSize()
{
	_inner_UserOverrideHeap = false;
	return 0x20000000; // Is this an appropriate value?
}

/*

CRT0 entrypoint

Startup:

1 - Relocation
2 - TOFINISH - HBABI config processing (if NRO)
3 - TOFINISH - memory processing, for virtual memory
4 - heap processing
5 - TODO - argv processing

Execution:

1 - TODO - high level (services?) initialization not yet decided
2 - main()
3 - TODO - cleanup of possible IPC/mem leaks
4 - return exit code and exit

*/

void BIO_WEAK __bio_crt0_Startup(void *config, bio::u64 thread_handle, void *aslr)
{
	bio::u8 *mod_base = (bio::u8*)aslr;
    bio::ld::ModuleHeader *module = (bio::ld::ModuleHeader*)&mod_base[*(bio::u32*)&mod_base[4]];

	Elf64_Dyn *dynamic = (Elf64_Dyn*) (((uint8_t*)module) + module->dynamic);

    __bio_crt0_Relocate(mod_base, dynamic);

    if(__bio_crt0_ExecutableFormat == 0) BIO_LOG("%s", "NSO");
	else if(__bio_crt0_ExecutableFormat == 1) BIO_LOG("%s", "NRO");
	else if(__bio_crt0_ExecutableFormat == 2) BIO_LOG("%s", "libNRO");

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

	// If hbloader didn't give us a base heap, get the default one
	auto def_heap = __bio_crt0_GetHeapSize();
	if((global_HeapSize == 0) || _inner_UserOverrideHeap)
	{
		global_HeapSize = def_heap;
		bio::svc::SetHeapSize(global_HeapAddress, global_HeapSize).Assert();
	}
}

// User entrypoint
int main(int argc, char **argv);

int BIO_WEAK __bio_crt0_Execution()
{
	// Execution is just main for now, until libraries improve
	return main(0, NULL);
}

// Actual code called by crt0. The other crt0 sub-phases are can be redefined by the user, but not this function.

int __bio_crt0_Entrypoint(void *config, bio::u64 thread_handle, void *aslr)
{
    __bio_crt0_Startup(config, thread_handle, aslr);
	return __bio_crt0_Execution();
}