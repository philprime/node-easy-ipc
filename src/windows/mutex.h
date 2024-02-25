#ifndef MUTEX_H
#define MUTEX_H

#include <napi.h>
#include <windows.h>
#include <node_buffer.h>
#include <string>

class Mutex : public Napi::ObjectWrap<Mutex> {
public:
    static Napi::Object Init(Napi::Env env);
    Mutex(const Napi::CallbackInfo& info);
    ~Mutex();

    // create mutex
    Napi::Value Create(const Napi::CallbackInfo& args);
    void create(const char *name, Napi::Env env);
    // open mutex
    Napi::Value Open(const Napi::CallbackInfo& args);
    void open(const char *name, Napi::Env env);
    // close mutex
    void Close(const Napi::CallbackInfo& args);
    // wait mutex
    Napi::Value Wait(const Napi::CallbackInfo& args);
    Napi::Value WaitMultiple(const Napi::CallbackInfo& args);
    // release mutex
    Napi::Value Release(const Napi::CallbackInfo& args);

private:
    HANDLE m_mutex;
};

#endif
