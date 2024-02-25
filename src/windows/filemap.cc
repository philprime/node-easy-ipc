#include "filemap.h"

Napi::Object FileMapping::Init(Napi::Env env) {

    Napi::Function func = DefineClass(env, "FileMapping",
                                      {
                                              InstanceMethod("createMapping", &FileMapping::CreateMapping),
                                              InstanceMethod("openMapping", &FileMapping::OpenMapping),
                                              InstanceMethod("closeMapping", &FileMapping::CloseMapping),
                                              InstanceMethod("writeBuffer", &FileMapping::WriteBuffer),
                                              InstanceMethod("readInto", &FileMapping::ReadInto),
                                      });

    Napi::FunctionReference *constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);
    env.SetInstanceData(constructor);

    return func;
}

FileMapping::FileMapping(const Napi::CallbackInfo &info)
        : Napi::ObjectWrap<FileMapping>(info) {
    this->m_fileHandle = INVALID_HANDLE_VALUE;
    this->m_mappingHandle = INVALID_HANDLE_VALUE;
}

FileMapping::~FileMapping() {
    if (m_fileHandle != INVALID_HANDLE_VALUE)
        CloseHandle(m_fileHandle);
    if (m_mappingHandle != INVALID_HANDLE_VALUE)
        CloseHandle(m_mappingHandle);
}

Napi::Value FileMapping::CreateMapping(const Napi::CallbackInfo &args) {
    Napi::Env env = args.Env();

    if (args.Length() < 3) {
        Napi::TypeError::New(env, "Not enough arguments to FileMapping.createMapping").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    if (!((args[0].IsNull() || args[0].IsString()) &&
          args[1].IsString() &&
          args[2].IsNumber())) {
        Napi::TypeError::New(env, "Wrong type arguments to FileMapping.createMapping").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    const char *filename = args[0].IsNull() ? nullptr : args[0].As<Napi::String>().Utf8Value().c_str();

    const std::string mappingNameStr = args[1].As<Napi::String>().Utf8Value();
    const char *mappingName = mappingNameStr.c_str();

    auto mappingSize = args[2].As<Napi::Number>().Uint32Value();

    this->create_mapping(filename, mappingName, mappingSize, env);

    return Napi::Number::New(env, 1);
}

void FileMapping::create_mapping(const char *fileName, const char *mappingName, unsigned mappingSize, Napi::Env env) {
    if (fileName == nullptr) {
        m_fileHandle = INVALID_HANDLE_VALUE; // Used for plain shared memory, with no backing file
    } else {
        m_fileHandle = CreateFile(
                fileName,
                GENERIC_READ | GENERIC_WRITE,
                0,
                nullptr,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                nullptr);

        if (m_fileHandle == INVALID_HANDLE_VALUE) {
            int lastErr = GetLastError();
            Napi::TypeError::New(env, "Failed to create file, error code:" +
                                      std::to_string(lastErr)).ThrowAsJavaScriptException();
            return;
        }
    }

    m_mappingHandle = CreateFileMapping(
            m_fileHandle,
            nullptr,
            PAGE_READWRITE,
            0,
            mappingSize,
            mappingName);

    if (m_mappingHandle == INVALID_HANDLE_VALUE) {
        int lastErr = GetLastError();
        Napi::TypeError::New(env, "Failed to create file mapping, error code: " +
                                  std::to_string(lastErr)).ThrowAsJavaScriptException();
        return;
    }

    m_ptr = MapViewOfFile(
            m_mappingHandle,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            mappingSize);

    if (m_ptr == nullptr) {
        int lastErr = GetLastError();
        Napi::TypeError::New(env, "Failed to map view of file, error code: " +
                                  std::to_string(lastErr)).ThrowAsJavaScriptException();
        return;
    }
}

Napi::Value FileMapping::OpenMapping(const Napi::CallbackInfo &args) {
    Napi::Env env = args.Env();

    if (args.Length() < 2) {
        Napi::TypeError::New(env, "Not enough arguments to FileMapping.openMapping").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    if (!(args[0].IsString() && args[1].IsNumber())) {
        Napi::TypeError::New(env, "Wrong type arguments to FileMapping.openMapping").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    const std::string mappingNameStr = args[0].As<Napi::String>().Utf8Value();
    const char *mappingName = mappingNameStr.c_str();

    auto mappingSize = args[1].As<Napi::Number>().Uint32Value();

    this->open_mapping(mappingName, mappingSize, env);

    return Napi::Number::New(env, 1);
}

void FileMapping::open_mapping(const char *mappingName, unsigned mappingSize, Napi::Env env) {

    //MessageBox(NULL, mappingName, "mappingName", MB_OK);

    this->m_mappingHandle = OpenFileMapping(
            FILE_MAP_ALL_ACCESS,
            FALSE,
            mappingName);

    //std::string mymessage = std::to_string((long) this->m_mappingHandle);
    //MessageBox(NULL, mymessage.c_str(), "m_mappingHandle", MB_OK);

    if (this->m_mappingHandle == INVALID_HANDLE_VALUE) {
        int lastErr = GetLastError();
        Napi::TypeError::New(env, "Failed to open file mapping, error code: " +
                                  std::to_string(lastErr)).ThrowAsJavaScriptException();
        return;
    }

    this->m_ptr = MapViewOfFile(
            m_mappingHandle,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            mappingSize);

    if (this->m_ptr == nullptr) {
        int lastErr = GetLastError();
        Napi::TypeError::New(env, "Failed to map view of file, error code: " +
                                  std::to_string(lastErr)).ThrowAsJavaScriptException();
        return;
    }
}

Napi::Value FileMapping::CloseMapping(const Napi::CallbackInfo &args) {
    Napi::Env env = args.Env();
    close_mapping();
    return Napi::Number::New(env, 1);
}

void FileMapping::close_mapping() {
    if (m_fileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_fileHandle);
        m_fileHandle = INVALID_HANDLE_VALUE;
    }
    if (m_mappingHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_mappingHandle);
        m_mappingHandle = INVALID_HANDLE_VALUE;
    }
}

Napi::Value FileMapping::WriteBuffer(const Napi::CallbackInfo &args) {
    Napi::Env env = args.Env();

    if (args.Length() < 4) {
        Napi::TypeError::New(env, "Not enough arguments to FileMapping.writeBuffer").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    if (!(args[0].IsBuffer() && args[1].IsNumber() && args[2].IsNumber() && args[3].IsNumber())) {
        Napi::TypeError::New(env, "Wrong type arguments to FileMapping.writeBuffer").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    char *bufferData = args[0].As < Napi::Buffer < char >> ().Data();
    auto destOffest = args[1].As<Napi::Number>().Uint32Value();
    auto srcOffset = args[2].As<Napi::Number>().Uint32Value();
    auto length = args[3].As<Napi::Number>().Uint32Value();

    memcpy(reinterpret_cast<char *>(this->m_ptr) + destOffest, bufferData + srcOffset, length);

    return Napi::Number::New(env, 1);
}

Napi::Value FileMapping::ReadInto(const Napi::CallbackInfo &args) {
    Napi::Env env = args.Env();

    if (args.Length() < 3) {
        Napi::TypeError::New(env, "Not enough arguments to FileMapping.readInto").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    if (!(args[0].IsNumber() && args[1].IsNumber() && args[2].IsBuffer())) {
        Napi::TypeError::New(env, "Wrong type arguments to FileMapping.readInto").ThrowAsJavaScriptException();
        return Napi::Number::New(env, -1);
    }

    auto offset = args[0].As<Napi::Number>().Uint32Value();
    auto length = args[1].As<Napi::Number>().Uint32Value();
	char *bufferData = args[2].As < Napi::Buffer < char >> ().Data();

    memcpy(bufferData + offset, reinterpret_cast<char *>(this->m_ptr), length);

    return Napi::Number::New(env, 1);
}
