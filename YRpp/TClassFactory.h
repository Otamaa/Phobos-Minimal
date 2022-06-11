#pragma once

#pragma once

#include <Unsorted.h>
#include <unknwn.h>
#include <YRCom.h>

/**
 *  Register a class-object with OLE.
 */
#define REGISTER_CLASS(_class) \
    { \
        DWORD dwRegister = 0; \
        TClassFactory<_class> *ptr = new TClassFactory<_class>(); \
        HRESULT hr = CoRegisterClassObject(__uuidof(_class), ptr, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &dwRegister); \
        if (FAILED(hr)) \
        { \
            std::printf("CoRegisterClassObject(TClassFactory<" #_class ">) failed with error code %d.\n", GetLastError()); \
        } else { \
            std::printf(#_class " factory registered.\n"); \
        } \
       ClassFactories.AddItem(dwRegister); \
    }


 /**
  *  Register a class-object with OLE (pre-created factory).
  */
#define REGISTER_FACT_CLASS(_class, _fact) \
    { \
        DWORD dwRegister = 0; \
        IClassFactory *ptr = _fact; \
        HRESULT hr = CoRegisterClassObject(__uuidof(_class), ptr, CLSCTX_INPROC_SERVER, REGCLS_MULTIPLEUSE, &dwRegister); \
        if (FAILED(hr)) \
        { \
            std::printf("CoRegisterClassObject(TClassFactory<" #_class ">) failed with error code %d.\n", GetLastError()); \
        } else { \
            std::printf(#_class " factory registered.\n"); \
        } \
        ClassFactories().AddItem(dwRegister); \
    }