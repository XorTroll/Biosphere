#include <cstring>
#include <cstdint>
#include <string>
#include <bio/bio_Types.hpp>
#include <elf.h>
#include <sys/time.h>
#include <sys/lock.h>
#include <sys/reent.h>
#include <bio/ld/ld_Types.hpp>
#include <bio/os/os_Thread.hpp>
#include <bio/os/os_TLS.hpp>
#include <bio/svc/svc_Base.hpp>
#include <bio/ipc/ipc_Request.hpp>
#include <bio/log/log_Logging.hpp>
#include <errno.h>
#include <bio/fs/fs_Types.hpp>
#include <sys/stat.h>

extern "C"
{
	extern bio::u8 __bio_bin_type;
	extern void *heap_addr;
    extern size_t heap_size;

    int __bio_entrypoint_startup(void *raw_config, uint64_t threadhandle, void *aslr, void *stack);
}

void __bio_relocate(bio::u8 *base, Elf64_Dyn *dyn)
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

bio::u64 smEncodeName(const char* name)
{
    bio::u64 name_encoded = 0;
    size_t i;

    for (i=0; i<8; i++)
    {
        if (name[i] == '\0')
            break;

        name_encoded |= ((bio::u64) name[i]) << (8*i);
    }

    return name_encoded;
}

int __bio_entrypoint_startup(void *raw_config, uint64_t thread_handle, void *aslr, void *stack)
{
    bio::u8 *mod_base = (bio::u8*)aslr;
    bio::ld::Module *module = (bio::ld::Module*)&mod_base[*(bio::u32*)&mod_base[4]];

	Elf64_Dyn *dynamic = (Elf64_Dyn*) (((uint8_t*)module) + module->dynamic);

    __bio_relocate(mod_base, dynamic);

    if(__bio_bin_type == 0) BIO_LOG("%s", "NSO");
	else if(__bio_bin_type == 1) BIO_LOG("%s", "NRO");
	else if(__bio_bin_type == 2) BIO_LOG("%s", "libNRO");

	bio::svc::SetHeapSize(heap_addr, 0x10000000).Assert();
	heap_size = 0x10000000;

	auto fspsrv = bio::fsp::Service::Initialize();

	std::shared_ptr<bio::fsp::FileSystem> fs;

	fspsrv->OpenSdCardFileSystem(fs).Assert();
	
	bio::fs::MountFileSystem(fs, "sd").Assert();

	int i = mkdir("sd:/fs-rules", 777);

	fs.reset();

	BIO_LOG("mkdir: %i", i);
	BIO_LOG("errno: %i", errno);

	return 0;
}