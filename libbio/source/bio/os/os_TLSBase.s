.global __bio_os_GetTLS
__bio_os_GetTLS:
    mrs x0, tpidrro_el0
    ret