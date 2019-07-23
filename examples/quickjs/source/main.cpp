#include <bio/fs/fs_Types.hpp>
using namespace bio;

extern "C"
{
    #include "cutils.h"
    #include "quickjs-libc.h"
    #include "quickjs.h"
    
    static int eval_buf(JSContext *ctx, const void *buf, int buf_len, const char *filename, int eval_flags)
    {
        JSValue val;
        int ret;

        val = JS_Eval(ctx, (const char*)buf, buf_len, filename, eval_flags);
        if (JS_IsException(val)) {
            js_std_dump_error(ctx);
            ret = -1;
        } else {
            ret = 0;
        }
        JS_FreeValue(ctx, val);
        return ret;
    }

    static int eval_file(JSContext *ctx, const char *filename, int eval_flags)
    {
        uint8_t *buf;
        int ret;
        size_t buf_len;
        
        buf = js_load_file(ctx, &buf_len, filename);
        if (!buf) {
            perror(filename);
            exit(1);
        }
        ret = eval_buf(ctx, buf, buf_len, filename, eval_flags);
        js_free(ctx, buf);
        return ret;
    }
}



int main()
{
    fs::Initialize().Assert();
    fs::MountSdCard("sd").Assert();

    JSRuntime *rt = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(rt);

    js_init_module_std(ctx, "std");
    js_init_module_os(ctx, "os");

    const char *str = "import * as std from 'std';\n"
        "import * as os from 'os';\n"
        "std.global.std = std;\n"
        "std.global.os = os;\n";
    eval_buf(ctx, str, strlen(str), "<input>", JS_EVAL_TYPE_MODULE);

    eval_file(ctx, "sd:/demo.js", JS_EVAL_TYPE_GLOBAL);
}