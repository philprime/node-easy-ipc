#include <napi.h>
#include <windows.h>

Napi::Value createMap(const Napi::CallbackInfo &args) {
    Napi::Env env = args.Env();

    HANDLE m_mappingHandle = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            nullptr,
            PAGE_READWRITE,
            0,
            8,
            "wrapper");

    Napi::Number my_test_val = Napi::Value::From(env, (long)m_mappingHandle).As<Napi::Number>();

    return my_test_val;
}

Napi::Number setValue(const Napi::CallbackInfo &args) {
    Napi::Env env = args.Env();

    HANDLE m_mappingHandle = (HANDLE)args[0].As<Napi::Number>().Int64Value();

    HANDLE m_ptr = MapViewOfFile(
            m_mappingHandle,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            0);



    return Napi::Value::From(env, (long)m_ptr).As<Napi::Number>();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "createMap"), Napi::Function::New(env, createMap));
    exports.Set(Napi::String::New(env, "setValue"), Napi::Function::New(env, setValue));
    return exports;
}

NODE_API_MODULE(addon, Init)