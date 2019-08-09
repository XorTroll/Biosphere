
#pragma once

// Attribute-related macros

#define BIO_NORETURN __attribute__((noreturn))
#define BIO_PACKED __attribute__((packed))
#define BIO_WEAK __attribute__((weak))

// Miscellaneous macros

#define BIO_IGNORE(var) ((void)(var))
#define BIO_BITMASK(n) (1 << n)
#define BIO_COUNTOF(array) (sizeof(array) / sizeof(array[0]))

// Result-related macros

#define BIO_TRY(expression) { auto _tmp_res = expression; if(_tmp_res.IsFailure()) { return _tmp_res; } }
#define BIO_MAKERESULT(mod, desc) ((mod & 0x1FF) | (desc & 0x1FFF) << 9)
#define BIO_DEFINERESULT(name, desc) static constexpr u32 Result##name = BIO_MAKERESULT(ResultModule, SubmoduleBase + desc);