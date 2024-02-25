#ifndef FILEMAP_H
#define FILEMAP_H

#include <napi.h>
#include <windows.h>
#include <node_buffer.h>
#include <string>

class FileMapping : public Napi::ObjectWrap<FileMapping> {
public:
    static Napi::Object Init(Napi::Env env);
    FileMapping(const Napi::CallbackInfo& info);
    ~FileMapping();
private:
    // createMapping
    Napi::Value CreateMapping(const Napi::CallbackInfo& args);
    void create_mapping(const char* filename, const char* mappingName, unsigned mappingSize, Napi::Env env);
    // openMapping
    Napi::Value OpenMapping(const Napi::CallbackInfo& args);
    void open_mapping(const char *mappingName, unsigned mappingSize, Napi::Env env);
    // closeMapping
    Napi::Value CloseMapping(const Napi::CallbackInfo& args);
    void close_mapping();
    // Buffer!
    Napi::Value WriteBuffer(const Napi::CallbackInfo& args);
    Napi::Value ReadInto(const Napi::CallbackInfo& args);

    double value_;
    HANDLE m_fileHandle;
    HANDLE m_mappingHandle;
    void *m_ptr;
};

#endif
