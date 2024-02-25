#include "mutex.h"
#include <malloc.h> // alloca

Napi::Object Mutex::Init(Napi::Env env) {

    Napi::Function func = DefineClass(env, "Mutex",
                                      {
                                              //InstanceValue("INFINITE", Napi::Number::New(env, INFINITE)),
                                              InstanceMethod("create", &Mutex::Create),
                                              InstanceMethod("open", &Mutex::Open),
                                              InstanceMethod("close", &Mutex::Close),
                                              InstanceMethod("wait", &Mutex::Wait),
                                              InstanceMethod("waitMultiple", &Mutex::WaitMultiple),
                                              InstanceMethod("release", &Mutex::Release),
                                      });

    Napi::FunctionReference *constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);
    env.SetInstanceData(constructor);

    return func;
}

Mutex::Mutex(const Napi::CallbackInfo &info)
        : Napi::ObjectWrap<Mutex>(info) {
    m_mutex = INVALID_HANDLE_VALUE;
}

Mutex::~Mutex() {
    if (m_mutex != INVALID_HANDLE_VALUE)
        CloseHandle(m_mutex);
}

Napi::Value Mutex::Create(const Napi::CallbackInfo &args) {
    Napi::Env env = args.Env();

    if (args.Length() < 1) {
        Napi::TypeError::New(env, "Not enough arguments to Mutex.create").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    if (!args[0].IsString()) {
        Napi::TypeError::New(env, "Wrong type arguments to Mutex.create").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    const std::string nameStr = args[0].As<Napi::String>().Utf8Value();
    const char *name = nameStr.c_str();

    this->create(name, env);

    return Napi::Number::New(env, 1);
}

void Mutex::create(const char *name, Napi::Env env) {
    m_mutex = CreateMutex(
            nullptr,
            FALSE,
            name);

    if (m_mutex == INVALID_HANDLE_VALUE) {
        int lastErr = GetLastError();
        Napi::TypeError::New(env, "Failed to create mutex, error code: " +
                                  std::to_string(lastErr)).ThrowAsJavaScriptException();
        return;
    }
}

Napi::Value Mutex::Open(const Napi::CallbackInfo &args) {
    Napi::Env env = args.Env();

    if (args.Length() < 1) {
        Napi::TypeError::New(env, "Not enough arguments to Mutex.open").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    if (!args[0].IsString()) {
        Napi::TypeError::New(env, "Wrong type arguments to Mutex.open").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    const std::string nameStr = args[0].As<Napi::String>().Utf8Value();
    const char *name = nameStr.c_str();

    this->open(name, env);

    return Napi::Number::New(env, 1);
}

void Mutex::open(const char *name, Napi::Env env) {
    m_mutex = OpenMutex(
            SYNCHRONIZE,
            FALSE,
            name);

    if (m_mutex == INVALID_HANDLE_VALUE) {
        int lastErr = GetLastError();
        Napi::TypeError::New(env, "Failed to create mutex, error code: " +
                                  std::to_string(lastErr)).ThrowAsJavaScriptException();
        return;
    }
}

void Mutex::Close(const Napi::CallbackInfo &args) {
    if (this->m_mutex != INVALID_HANDLE_VALUE) {
        CloseHandle(this->m_mutex);
    }
}

Napi::Value Mutex::Wait(const Napi::CallbackInfo &args) {
    Napi::Env env = args.Env();

    DWORD ms;

    if (args.Length() < 1) {
        ms = INFINITE;
    } else {
        if (!args[0].IsNumber()) {
            Napi::TypeError::New(env, "Wrong type arguments to Mutex.wait").ThrowAsJavaScriptException();
            return Napi::Number::New(env, -1);
        }

        ms = args[0].As<Napi::Number>().Uint32Value();
    }

    DWORD waitResult = WaitForSingleObject(this->m_mutex, ms);

    if (waitResult == WAIT_FAILED) {
        int lastErr = GetLastError();
        Napi::TypeError::New(env, "Failed to wait on mutex, error code: " +
                                  std::to_string(lastErr)).ThrowAsJavaScriptException();
        return Napi::Number::New(env, waitResult);
    }

    return Napi::Number::New(env, waitResult);
}

Napi::Value Mutex::WaitMultiple(const Napi::CallbackInfo &args) {
    Napi::Env env = args.Env();

    if (args.Length() < 3) {
        Napi::TypeError::New(env, "Not enough arguments to Mutex.waitMultiple").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    if (!(args[0].IsArray() &&
          args[1].IsBoolean() &&
          args[2].IsNumber())) {
        Napi::TypeError::New(env, "Wrong type arguments to Mutex.waitMultiple").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    auto mutexList = args[0].As<Napi::Array>();
    bool waitAll = args[1].As<Napi::Boolean>().Value();
    auto waitFor = args[2].As<Napi::Number>().Uint32Value();


    auto len = mutexList.Length();
    HANDLE *mutexArr = reinterpret_cast<HANDLE *>(alloca(len * sizeof(HANDLE)));
    for (unsigned i = 0; i < mutexList.Length(); ++i) {
        auto it = mutexList.Get(i);
        Mutex *obj = Napi::ObjectWrap<Mutex>::Unwrap(it.As<Napi::Object>());
        mutexArr[i] = obj->m_mutex;
    }

    DWORD waitResult = WaitForMultipleObjects(
            mutexList.Length(),
            mutexArr,
            waitAll,
            waitFor);

    if (waitResult == WAIT_FAILED) {
        int lastErr = GetLastError();
        Napi::TypeError::New(env, "Failed to wait on mutex, error code: " +
                                  std::to_string(lastErr)).ThrowAsJavaScriptException();
        return Napi::Number::New(env, waitResult);
    }

    return Napi::Number::New(env, waitResult);
}

Napi::Value Mutex::Release(const Napi::CallbackInfo &args) {
    Napi::Env env = args.Env();
    ReleaseMutex(this->m_mutex);
    return Napi::Number::New(env, 1);
}
