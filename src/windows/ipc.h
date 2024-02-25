#ifndef IPC_H
#define IPC_H

#include <napi.h>
#include <windows.h>
#include <node_buffer.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include <thread>

class IPC : public Napi::ObjectWrap<IPC> {
public:
    static Napi::Object Init(Napi::Env env);
    IPC(const Napi::CallbackInfo& info);
    ~IPC();
private:
    // on event
    Napi::Value On(const Napi::CallbackInfo& args);
    void OnMessage(Napi::Env env, Napi::Function callback);

    Napi::Value GetPID(const Napi::CallbackInfo& args);
    Napi::Value GetMainThreadId(const Napi::CallbackInfo& args);
    Napi::Value GetMessageThreadId(const Napi::CallbackInfo& args);
    Napi::Value WaitThread(const Napi::CallbackInfo& args);
    Napi::Value Send(const Napi::CallbackInfo& args);
    void StartMessageThread();
    void StopMessageThread();

    Napi::Function messageCallback;
    DWORD pid;
    DWORD mainThreadId;
    DWORD messageThreadId;
    Napi::ThreadSafeFunction tsfn;
    std::thread nativeThread;
    bool nativeThreadTerminated;
};

#endif
