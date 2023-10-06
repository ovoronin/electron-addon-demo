#include <napi.h>
#include <iostream>
#include <windows.h>
#include <uiautomation.h>

using namespace std;
namespace example
{
    Napi::Boolean init(const Napi::CallbackInfo &info);
    void destroy(const Napi::CallbackInfo &info);
    Napi::Boolean onFocus(const Napi::CallbackInfo &info);
    Napi::String setText(const Napi::CallbackInfo &info);
    // Export API
    Napi::Object Init(Napi::Env env, Napi::Object exports);
    NODE_API_MODULE(addon, Init)
}