#include <napi.h>
#include "filemap.h"
#include "mutex.h"
#include "ipc.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    exports.Set("INFINITE", Napi::Number::New(env, INFINITE));
    exports.Set("FileMapping", FileMapping::Init(env));
    exports.Set("Mutex", Mutex::Init(env));
    exports.Set("IPC", IPC::Init(env));
    return exports;
}

NODE_API_MODULE(addon, InitAll)