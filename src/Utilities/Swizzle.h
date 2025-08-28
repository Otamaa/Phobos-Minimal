#pragma once

#include <vector>
#include <type_traits>

#include <Objidl.h>
#include <SwizzleManagerClass.h>

#include <Utilities/Debug.h>

#include <unordered_map>

class PhobosSwizzleManagerClass : public ISwizzle
{
private:
    struct SwizzlePointerStruct {
        SwizzlePointerStruct() : ID(-1), Pointer(nullptr), Line(-1) {}

        SwizzlePointerStruct(LONG id, void* pointer, const char* file = nullptr, const int line = -1, const char* func = nullptr, const char* var = nullptr) : ID(id), Pointer(pointer), Line(line)
        {
            if (file != nullptr) {
                File = file;
            }

            if (func != nullptr) {
                Function = func;
            }

            if (var != nullptr) {
                Variable = var;
            }
        }

        /**
         *  Enable move semantics.
         */
        SwizzlePointerStruct(SwizzlePointerStruct&&) noexcept = default;
        SwizzlePointerStruct& operator=(SwizzlePointerStruct&&) noexcept = default;

        /**
         *  The id of the pointer to remap.
         */
        LONG ID;

        /**
         *  The pointer to fixup.
         */
        void* Pointer;

        /**
         *  Debugging information.
         */
        std::string File;
        int Line;
        std::string Function;
        std::string Variable;
    };

public:
    /**
     *  IUnknown
     */
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj) override;
    STDMETHOD_(ULONG, AddRef)() override;
    STDMETHOD_(ULONG, Release)() override;

    /**
     *  ISwizzle
     */
    STDMETHOD_(LONG, Reset)() override;
    STDMETHOD_(LONG, Swizzle)(void** pointer) override;
    STDMETHOD_(LONG, Fetch_Swizzle_ID)(void* pointer, LONG* id) override;
    STDMETHOD_(LONG, Here_I_Am)(LONG id, void* pointer) override;
    STDMETHOD(Save_Interface)(IStream* stream, IUnknown* pointer) override;
    STDMETHOD(Load_Interface)(IStream* stream, CLSID* riid, void** pointer) override;
    STDMETHOD_(LONG, Get_Save_Size)(LONG* size) override;

    /**
     *  New debug routines.
     */
    STDMETHOD_(LONG, Swizzle_Dbg)(void** pointer, const char* file, const int line, const char* func = nullptr, const char* var = nullptr);
    STDMETHOD_(LONG, Fetch_Swizzle_ID_Dbg)(void* pointer, LONG* id, const char* file, const int line, const char* func = nullptr, const char* var = nullptr);
    STDMETHOD_(LONG, Here_I_Am_Dbg)(LONG id, void* pointer, const char* file, const int line, const char* func = nullptr, const char* var = nullptr);

public:
    PhobosSwizzleManagerClass();
    ~PhobosSwizzleManagerClass();

private:
    void Process_Tables();

private:
    /**
     *  List of all the pointers that need remapping.
     */
    std::vector<SwizzlePointerStruct> RequestTable;

    /**
     *  List of all the new pointers.
     */
    std::unordered_map<LONG, SwizzlePointerStruct> PointerTable;
};

extern PhobosSwizzleManagerClass PhobosSwizzleManager;

#define PHOBOS_SWIZZLE_RESET(func) \
    { \
        PhobosSwizzleManager.Reset(); \
    }

#define PHOBOS_SWIZZLE_REQUEST_POINTER_REMAP(pointer, variable) \
    { \
        PhobosSwizzleManager.Swizzle_Dbg((void**)&pointer, __FILE__, __LINE__, __FUNCTION__##"()", variable); \
    }

#define PHOBOS_SWIZZLE_REQUEST_POINTER_REMAP_LIST(vector, variable) \
    { \
        for (int __i = 0; __i < vector.Count(); ++__i) { \
            PhobosSwizzleManager.Swizzle_Dbg((void**)&vector[__i], __FILE__, __LINE__, __FUNCTION__##"()", variable); \
        } \
    }

#define PHOBOS_SWIZZLE_FETCH_SWIZZLE_ID(pointer, id, variable) \
    { \
        PhobosSwizzleManager.Fetch_Swizzle_ID_Dbg((void*)pointer, (LONG*)&id, __FILE__, __LINE__, __FUNCTION__##"()", variable); \
    }

#define PHOBOS_SWIZZLE_REGISTER_POINTER(id, pointer, variable) \
    { \
        PhobosSwizzleManager.Here_I_Am_Dbg(id, pointer, __FILE__, __LINE__, __FUNCTION__##"()", variable); \
    }

