#define _HAS_STD_BYTE 0
#include "example.h"
#include <windows.h>
#include <uiautomation.h>

using namespace std;

IUIAutomation *pAutomation;
Napi::ThreadSafeFunction callback;
IUIAutomationElement *focused;
IUIAutomationElement *prevFocused;

class EventHandler:
    public IUIAutomationFocusChangedEventHandler
{
private:
    LONG _refCount;

public:
    int _eventCount;

    //Constructor.
    EventHandler(): _refCount(1), _eventCount(0) 
    {
    }

    //IUnknown methods.
    ULONG STDMETHODCALLTYPE AddRef() 
    {
        ULONG ret = InterlockedIncrement(&_refCount);
        return ret;
    }

    ULONG STDMETHODCALLTYPE Release() 
    {
        ULONG ret = InterlockedDecrement(&_refCount);
        if (ret == 0) 
        {
            delete this;
            return 0;
        }
        return ret;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppInterface) 
    {
        if (riid == __uuidof(IUnknown))
            *ppInterface = static_cast<IUIAutomationFocusChangedEventHandler*>(this);
        else if(riid == __uuidof(IUIAutomationFocusChangedEventHandler)) 
            *ppInterface=static_cast<IUIAutomationFocusChangedEventHandler*>(this);
        else 
        {
            *ppInterface = NULL;
            return E_NOINTERFACE;
        }
        this->AddRef();
        return S_OK;
    }

    // IUIAutomationFocusChangedEventHandler methods.
    HRESULT STDMETHODCALLTYPE HandleFocusChangedEvent(IUIAutomationElement * pSender) 
    {
        _eventCount++;
        if (callback != NULL) {
          RECT rect;
          pSender->get_CurrentBoundingRectangle(&rect);

          IValueProvider* valueProvider = nullptr;
          HRESULT res = pSender->GetCurrentPattern(UIA_ValuePatternId, reinterpret_cast<IUnknown**>(&valueProvider));

          CONTROLTYPEID controlType ;
          pSender->get_CurrentControlType(&controlType);

          BOOL focusable = !(res != S_OK || (controlType != UIA_EditControlTypeId && controlType != UIA_TextControlTypeId));

          if (!focusable) {
            rect.left = 0;
            rect.top = 0;
            rect.bottom = 0;
            rect.right = 0;
          }

          auto cb = [=](Napi::Env env, Napi::Function jsCallback) {
            jsCallback.Call({
              Napi::Number::New(env, rect.left),
              Napi::Number::New(env, rect.top),
              Napi::Number::New(env, rect.right),
              Napi::Number::New(env, rect.bottom),
              Napi::Number::New(env, controlType),
            });
          };          
          if (focusable) {
            prevFocused = focused;
            focused = pSender;
          }
          callback.BlockingCall(cb);
        }
        return S_OK;
    }
};

Napi::Boolean example::init(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  HRESULT res = CoInitializeEx(NULL,COINIT_MULTITHREADED);
  if (res != S_OK)
  {
    example::destroy(info);
    return Napi::Boolean::New(env, FALSE);
  }

  HRESULT automRes = CoCreateInstance(CLSID_CUIAutomation, NULL,
                          CLSCTX_INPROC_SERVER, IID_IUIAutomation,
                          (void **)&pAutomation);
  if (automRes != S_OK)
  {
    example::destroy(info);
    return Napi::Boolean::New(env, FALSE);
  }

  return Napi::Boolean::New(env, TRUE);
}

Napi::String example::setText(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  try
  {
    if (!info[0].IsString()) {
      return Napi::String::New(env, "Not a string");
    }

    std::string s = info[0].As<Napi::String>().Utf8Value();
    std::wstring stemp = std::wstring(s.begin(), s.end());
    LPCWSTR text = stemp.c_str();
    if (focused != NULL) {
      IValueProvider* valueProvider = nullptr;

      HRESULT res = focused->GetCurrentPattern(UIA_ValuePatternId, reinterpret_cast<IUnknown**>(&valueProvider));
      if (res == S_OK) {
        valueProvider->SetValue(text);
        return Napi::String::New(env, "");
      }
    }
  }
  catch (std::exception& e)
  {
    return Napi::String::New(env, e.what());
  }

  return Napi::String::New(env, "Failed");
}

void example::destroy(const Napi::CallbackInfo &info)
{
  if (pAutomation != NULL)
  {
    pAutomation->Release();
  }
  CoUninitialize();
}

Napi::Boolean example::onFocus(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  if (pAutomation != NULL)
  {

    EventHandler *pEHTemp = new EventHandler();
    if (pEHTemp == NULL)
    {
      return Napi::Boolean::New(env, FALSE);
    }

    callback = Napi::ThreadSafeFunction::New(
        env, 
        info[0].As<Napi::Function>(),
        "Callback",
        0,
        1
    );

    HRESULT hr = pAutomation->AddFocusChangedEventHandler(NULL, pEHTemp); 
    if (FAILED(hr)) 
    {
      return Napi::Boolean::New(env, FALSE);
    }    
    return Napi::Boolean::New(env, TRUE);
  }
  return Napi::Boolean::New(env, FALSE);
}

Napi::Object example::Init(Napi::Env env, Napi::Object exports)
{
  // export Napi function
  exports.Set("init", Napi::Function::New(env, example::init));
  exports.Set("destroy", Napi::Function::New(env, example::destroy));
  exports.Set("onFocus", Napi::Function::New(env, example::onFocus));
  exports.Set("setText", Napi::Function::New(env, example::setText));
  return exports;
}