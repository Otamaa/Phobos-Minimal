#pragma once

#include <vector>
#include <type_traits>

#include <Objidl.h>
#include <SwizzleManagerClass.h>

#include <Utilities/Debug.h>

#include <unordered_map>
#include <unordered_set>

#include <Interface/ISwizzle.h>

class ExtensionSwizzleManager
{
	struct ExtensionEntry
	{
		uintptr_t ptr;
		void (*deleter)(uintptr_t); // Type-specific deleter
		bool released = false;

		ExtensionEntry(uintptr_t p, void (*del)(uintptr_t))
			: ptr(p), deleter(del), released(false)
		{ }

		ExtensionEntry()
			: ptr(0), deleter(nullptr), released(true)
		{ }

		// Delete copy constructor and copy assignment
		ExtensionEntry(const ExtensionEntry&) = delete;
		ExtensionEntry& operator=(const ExtensionEntry&) = delete;

		// Default move constructor
		ExtensionEntry(ExtensionEntry&& other) noexcept
			: ptr(other.ptr)
			, deleter(other.deleter)
			, released(other.released)
		{
			other.released = true; // Prevent double-delete
		}

		// Move assignment operator
		ExtensionEntry& operator=(ExtensionEntry&& other) noexcept
		{
			if (this != &other)
			{
				// Clean up existing resource
				if (!released && ptr)
				{
					deleter(ptr);
				}

				// Move from other
				ptr = other.ptr;
				deleter = other.deleter;
				released = other.released;

				// Prevent double-delete
				other.released = true;
			}
			return *this;
		}

		~ExtensionEntry()
		{
			if (!released && ptr)
			{
				deleter(ptr);
			}
		}

		void release() { released = true; }
	};

	static std::unordered_map<uintptr_t, ExtensionEntry> extensionPointerMap;

public:

	template<typename T>
	static ExtensionEntry makeEntry(T* extension)
	{
		return {
			reinterpret_cast<uintptr_t>(extension),
			[](uintptr_t p) { delete reinterpret_cast<T*>(p); }
		};
	}

	template <typename T>
	static void RegisterExtensionPointer(void* savedAddress, T* currentExtension)
	{
		extensionPointerMap[(uintptr_t)savedAddress] = makeEntry(currentExtension);
	}

	static bool SwizzleExtensionPointer(void** ptrToFix, AbstractClass* OwnerObj);

	// Clean up orphaned extensions
	static void CleanupUnmappedExtensions();
};

class PhobosSwizzleManagerClass : public ISwizzle
{
private:
	struct SwizzlePointerStruct
	{
		SwizzlePointerStruct() : ID(-1), Pointer(nullptr), Line(-1) { }
		SwizzlePointerStruct(LONG id, void* pointer, const char* file = nullptr, const int line = -1, const char* func = nullptr, const char* var = nullptr)
			: ID(id), Pointer(pointer), Line(line)
		{
			if (file != nullptr)
			{
				File = file;
			}
			if (func != nullptr)
			{
				Function = func;
			}
			if (var != nullptr)
			{
				Variable = var;
			}
		}

		SwizzlePointerStruct(SwizzlePointerStruct&&) noexcept = default;
		SwizzlePointerStruct& operator=(SwizzlePointerStruct&&) noexcept = default;

		SwizzlePointerStruct(const SwizzlePointerStruct&) = default;
		SwizzlePointerStruct& operator=(const SwizzlePointerStruct&) = default;

		~SwizzlePointerStruct() = default;
	public:

		/** The id of the pointer to remap. */
		LONG ID;
		/** The pointer to fixup. */
		void* Pointer;
		/** Debugging information. */
		std::string File;
		int Line;
		std::string Function;
		std::string Variable;
	};

	struct UnresolvedPointerInfo
	{
		LONG UnresolvedID;
		std::vector<SwizzlePointerStruct> DanglingPointers;

		UnresolvedPointerInfo(LONG id) : UnresolvedID(id) { }
	};

public:
	/** IUnknown */
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj) override;
	STDMETHOD_(ULONG, AddRef)() override;
	STDMETHOD_(ULONG, Release)() override;

	/** ISwizzle */
	STDMETHOD_(LONG, Reset)() override;
	STDMETHOD_(LONG, Swizzle)(void** pointer) override;
	STDMETHOD_(LONG, Fetch_Swizzle_ID)(void* pointer, LONG* id) override;
	STDMETHOD_(LONG, Here_I_Am)(LONG id, void* pointer) override;
	STDMETHOD(Save_Interface)(IStream* stream, IUnknown* pointer) override;
	STDMETHOD(Load_Interface)(IStream* stream, CLSID* riid, void** pointer) override;
	STDMETHOD_(LONG, Get_Save_Size)(LONG* size) override;

	/** New debug routines. */
	STDMETHOD_(LONG, Swizzle_Dbg)(void** pointer, const char* file, const int line, const char* func = nullptr, const char* var = nullptr);
	STDMETHOD_(LONG, Fetch_Swizzle_ID_Dbg)(void* pointer, LONG* id, const char* file, const int line, const char* func = nullptr, const char* var = nullptr);
	STDMETHOD_(LONG, Here_I_Am_Dbg)(LONG id, void* pointer, const char* file, const int line, const char* func = nullptr, const char* var = nullptr);

	/** Enhanced cleanup methods */
	STDMETHOD_(LONG, Register_Dependent_Pointer)(LONG referenced_id, void** dependent_pointer, const char* file = nullptr, const int line = -1, const char* func = nullptr, const char* var = nullptr);
	STDMETHOD_(void, Cleanup_Dangling_Pointers)();
	STDMETHOD_(LONG, Get_Unresolved_Count)(LONG* count);
	STDMETHOD_(void, Set_Cleanup_Mode)(BOOL null_dangling_pointers);

public:
	PhobosSwizzleManagerClass();
	~PhobosSwizzleManagerClass();

private:
	void Process_Tables();
	void Track_Unresolved_Pointer(const SwizzlePointerStruct& request);
	void Cleanup_References_To_Unresolved(LONG unresolved_id);

private:
	/** List of all the pointers that need remapping. */
	std::vector<SwizzlePointerStruct> RequestTable;

	/** List of all the new pointers. */
	std::unordered_map<LONG, SwizzlePointerStruct> PointerTable;

	/** Bucket of unresolved pointer IDs and their dependent pointers */
	std::unordered_map<LONG, UnresolvedPointerInfo> UnresolvedBucket;

	/** Map of dependency relationships: dependent_pointer_address -> referenced_id */
	std::unordered_map<void**, LONG> DependencyMap;

	/** Whether to automatically null out dangling pointers during cleanup */
	BOOL AutoNullDanglingPointers;
};

extern PhobosSwizzleManagerClass PhobosSwizzleManager;

// Enhanced macros
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

// New macro for registering dependent pointers
#define PHOBOS_SWIZZLE_REGISTER_DEPENDENT(dependent_ptr, referenced_id, variable) \
    { \
        PhobosSwizzleManager.Register_Dependent_Pointer(referenced_id, (void**)&dependent_ptr, __FILE__, __LINE__, __FUNCTION__##"()", variable); \
    }