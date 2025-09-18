#include "Swizzle.h"

#include <Surface.h>
#include <Exception.h>

void Clear_All_Surfaces()
{
	if (DSurface::Primary()) DSurface::Primary->Clear();
	if (DSurface::Hidden()) DSurface::Hidden->Clear();
	if (DSurface::Hidden_2()) DSurface::Hidden_2->Clear();
	if (DSurface::Alternate()) DSurface::Alternate->Clear();
	if (DSurface::Sidebar()) DSurface::Sidebar->Clear();
	if (DSurface::Composite()) DSurface::Composite->Clear();
}

#include <Utilities/Container.h>

std::unordered_map<uintptr_t, uintptr_t>  ExtensionSwizzleManager::extensionPointerMap;

void ExtensionSwizzleManager::RegisterExtensionPointer(void* savedAddress, void* currentExtension)
{
	extensionPointerMap[(uintptr_t)savedAddress] = (uintptr_t)currentExtension;
}

bool ExtensionSwizzleManager::SwizzleExtensionPointer(void** ptrToFix, AbstractClass* OwnerObj)
{
	if (!*ptrToFix) // nothing to fix
		return true;

	auto it = extensionPointerMap.find((uintptr_t)*ptrToFix);
	if (it != extensionPointerMap.end() && it->second)
	{
		auto currentExtension = (AbstractExtended*)it->second;

		// Fix bidirectional pointers
		Debug::Log("ExtensionSwizzleManager::SwizzleExtensionPointer Fixing Up Extension Pointer of %s from %x to %x \n"
	,	currentExtension->GetAttachedObjectName(), *ptrToFix, currentExtension);
		currentExtension->SetAttached(OwnerObj);
		*ptrToFix = currentExtension;
		it->second = 0u;
		return true;
	}

	Debug::Log("[ExtensionSwizzleManager] Cannot Find ext pointer of %x from %x !\n", *ptrToFix, OwnerObj);
	return false;
}

// Clean up orphaned extensions
void ExtensionSwizzleManager::CleanupUnmappedExtensions()
{
	for (const auto& [savedAddr, extension] : extensionPointerMap) {
		if(extension > 0) {
			Debug::Log("[ExtensionSwizzleManager] Cleaning up unmapped extension: %x -> %x\n",
				savedAddr , extension);

			delete (AbstractExtended*)extension;
		}
	}

	extensionPointerMap.clear();
}

/*
# Phobos Swizzle Manager - Complete System Analysis

## 🎯** What is Pointer Swizzling ? **

**Pointer Swizzling * *is a technique used in serialization where memory pointers are converted to persistent IDs when saving data, and then converted back to valid memory pointers when loading data.

```
SAVE PROCESS : Memory Pointer → Persistent ID
LOAD PROCESS : Persistent ID → New Memory Pointer
```

-- -

## 📊 * *System Architecture Diagram * *

```
┌─────────────────────────────────────────────────────────────────┐
│                    PHOBOS SWIZZLE MANAGER                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌─────────────────┐    ┌─────────────────┐    ┌──────────────┐ │
│  │  REQUEST TABLE  │    │  POINTER TABLE  │    │ UNRESOLVED   │ │
│  │                 │    │                 │    │   BUCKET     │ │
│  │ Pointers        │    │ ID → Real       │    │              │ │
│  │ waiting to be   │    │ Pointer         │    │ Failed       │ │
│  │ remapped        │    │ Mappings        │    │ Remappings   │ │
│  └─────────────────┘    └─────────────────┘    └──────────────┘ │
│           │                       │                       │     │
│           │                       │                       │     │
│           └───────────────────────┼───────────────────────┘     │
│                                   │                             │
│                    ┌──────────────▼──────────────┐              │
│                    │      PROCESS_TABLES()       │              │
│                    │                             │              │
│                    │  Matches IDs and remaps     │              │
│                    │  pointers to new addresses  │              │
│                    └─────────────────────────────┘              │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────────┤
│  │              DEPENDENCY TRACKING SYSTEM                     │
│  │                                                             │
│  │  ┌─────────────────┐              ┌─────────────────┐       │
│  │  │ DEPENDENCY MAP  │              │ CLEANUP SYSTEM  │       │
│  │  │                 │              │                 │       │
│  │  │ Tracks which    │──────────────▶│ Nulls dangling │       │
│  │  │ pointers depend │              │ pointers when  │       │
│  │  │ on which IDs    │              │ objects missing │       │
│  │  └─────────────────┘              └─────────────────┘       │
│  └─────────────────────────────────────────────────────────────┘
└─────────────────────────────────────────────────────────────────┘
```

-- -

## 🔧 * *Core Data Structures * *

### 1. * *SwizzlePointerStruct * *
```cpp
struct SwizzlePointerStruct
{
	LONG ID;              // Unique identifier for the object
	void* Pointer;        // Memory address of the pointer
	std::string File;     // Debug: source file
	int Line;            // Debug: line number
	std::string Function; // Debug: function name
	std::string Variable; // Debug: variable name
}
```
** Purpose** : Stores all information about a pointer that needs remapping, including debug info for troubleshooting.

### 2. * *UnresolvedPointerInfo * *
```cpp
struct UnresolvedPointerInfo
{
	LONG UnresolvedID;                          // The missing object ID
	std::vector<SwizzlePointerStruct> DanglingPointers; // All pointers waiting for this ID
}
```
** Purpose** : Tracks objects that haven't been found yet and all the pointers waiting for them.

-- -

## 🚀 * *Function - by - Function Analysis * *

## * *Phase 1: Registration Functions * *

### 🔵 * *`Swizzle()` - Request Pointer Remapping * *
```cpp
LONG Swizzle(void** pointer);
```

** What it does : **
1. Takes a pointer that contains an ID(disguised as a pointer address)
2. Adds it to the `RequestTable` for later processing
3. Stores debug information for troubleshooting

** Flow Diagram : **
```
Application Code
│
▼
┌─────────────┐    ┌──────────────┐    ┌─────────────┐
│ Object has  │───▶│   Swizzle()  │───▶│ Add to      │
│ ID instead  │    │   called     │    │ RequestTable│
│ of pointer  │    └──────────────┘    └─────────────┘
└─────────────┘
```

### 🔵** `Here_I_Am()` - Register Object Location * *
```cpp
LONG Here_I_Am(LONG id, void* pointer);
```

** What it does : **
1. Called when an object is loaded into memory
2. Associates the object's ID with its new memory address
3. Stores this mapping in `PointerTable`

* *Flow Diagram : **
```
Object Loaded
│
▼
┌─────────────┐    ┌──────────────┐    ┌─────────────┐
│ Object at   │───▶│ Here_I_Am()  │───▶│ Add ID→Ptr  │
│ new address │    │   called     │    │ to Pointer  │
│             │    │              │    │ Table       │
└─────────────┘    └──────────────┘    └─────────────┘
```

### 🔵 * *`Register_Dependent_Pointer()` - Track Dependencies * *
```cpp
LONG Register_Dependent_Pointer(LONG referenced_id, void** dependent_pointer, ...);
```

** What it does : **
1. Records that one pointer depends on another object
2. Enables cleanup of dangling pointers if the referenced object is never loaded
3. Creates a dependency graph

* *Example : **
```cpp
// If ObjectA points to ObjectB
// Register that ObjectA's pointer depends on ObjectB's ID
Register_Dependent_Pointer(ObjectB_ID, &ObjectA->pointerToB, ...);
```

-- -

## * *Phase 2: Processing Functions * *

### 🟢 * *`Process_Tables()` - The Main Engine * *
```cpp
void Process_Tables();
```

** What it does : **
This is the heart of the swizzling system.It processes all pending requests and performs the actual pointer remapping.

** Detailed Flow : **
```
┌─────────────────────────────────────────────────────┐
│                PROCESS_TABLES()                     │
├─────────────────────────────────────────────────────┤
│                                                     │
│ 1. Loop through each request in RequestTable        │
│                                                     │
│ 2. For each request : │
│    ┌─────────────────────────────────────────┐     │
│    │ Look up ID in PointerTable              │     │
│    │                                         │     │
│    │ ┌─────────────┐  ┌─────────────────────┐ │     │
│    │ │ ID Found ? │  │ ID Not Found ? │ │     │
│    │ │             │  │                     │ │     │
│    │ │ ✅ SUCCESS  │  │ ❌ UNRESOLVED       │ │     │
│    │ │             │  │                     │ │     │
│    │ │ Remap the   │  │ Add to Unresolved   │ │     │
│    │ │ pointer to  │  │ Bucket for later    │ │     │
│    │ │ new address │  │ cleanup             │ │     │
│    │ └─────────────┘  └─────────────────────┘ │     │
│    └─────────────────────────────────────────┘     │
│                                                     │
│ 3. Clear RequestTable and PointerTable             │
│                                                     │
└─────────────────────────────────────────────────────┘
```

* *Example of Successful Remapping : **
```
Before :
	RequestTable : [ID:12345, Pointer : 0x8000 (contains ID)]
	PointerTable : [12345 → 0xABCDEF00 (real object address)]

	After Process_Tables() :
	The memory at 0x8000 now contains 0xABCDEF00 instead of 12345
	```

	-- -

	## * *Phase 3: Cleanup and Error Handling * *

	### 🟡 * *`Track_Unresolved_Pointer()` - Handle Missing Objects * *
	```cpp
	void Track_Unresolved_Pointer(const SwizzlePointerStruct & request);
```

** What it does : **
1. Called when an object ID can't be found during processing
2. Adds the unresolved ID to the `UnresolvedBucket`
3. Finds all dependent pointers that reference this missing object

* *Flow:**
```
┌──────────────┐    ┌─────────────────┐    ┌──────────────────┐
│ Object ID    │───▶│ Track as        │───▶│ Find all         │
│ not found    │    │ Unresolved      │    │ dependent        │
│              │    │                 │    │ pointers         │
└──────────────┘    └─────────────────┘    └──────────────────┘
│
▼
┌──────────────────┐
│ Add to dangling  │
│ pointers list    │
└──────────────────┘
```

### 🔴 * *`Cleanup_Dangling_Pointers()` - Memory Safety * *
```cpp
void Cleanup_Dangling_Pointers();
```

** What it does : **
1. Goes through all unresolved objects
2. For each unresolved object, finds all pointers that were waiting for it
3. Optionally sets those pointers to `nullptr` to prevent crashes

* *Safety Mechanism : **
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ Unresolved      │───▶│ Find all        │───▶│ Set dangling    │
│ Object ID : 999  │    │ pointers that   │    │ pointers to     │
│                 │    │ reference 999   │    │ nullptr         │
└─────────────────┘    └─────────────────┘    └─────────────────┘

Result : Prevents crashes from accessing invalid memory
```

-- -

## * *Utility Functions * *

###  * *`Reset()`* * -Clean Slate
- Processes any pending requests
- Clears all internal tables
- Prepares for a new swizzling session

### ** `Get_Unresolved_Count()`* * -Diagnostics
- Returns how many objects couldn't be resolved
- Useful for debugging serialization problems

### ** `Set_Cleanup_Mode()`* * -Configuration
- Controls whether dangling pointers are automatically nulled
- Allows for different cleanup strategies

-- -

## 🎮 * *Usage Example - Complete Workflow * *

```cpp
// === SAVING PHASE ===
// Objects are converted to IDs before saving

// === LOADING PHASE ===
// 1. Reset the manager
PHOBOS_SWIZZLE_RESET();

// 2. As objects load, they register their new addresses
PhobosSwizzleManager.Here_I_Am(12345, objectA);
PhobosSwizzleManager.Here_I_Am(67890, objectB);

// 3. As pointers are encountered, request remapping
PhobosSwizzleManager.Swizzle(&somePointer); // contains ID instead of address

// 4. Register dependencies
PhobosSwizzleManager.Register_Dependent_Pointer(67890, &objectA->ptrToB);

// 5. Process all the remapping (automatic in destructor or manual)
// - IDs are converted back to real memory addresses
// - Missing objects are tracked
// - Dangling pointers are cleaned up
```

-- -

## ⚡ * *Key Benefits * *

1. * *Memory Safety * *: Prevents crashes from invalid pointers
2. * *Debug Support * *: Rich debugging information for troubleshooting
3. * *Dependency Tracking * *: Handles complex object relationships
4. * *Automatic Cleanup * *: Manages dangling pointers automatically
5. * *Flexible Configuration * *: Customizable cleanup behavior

-- -

## 🚨 * *Common Use Cases * *

- **Game Save / Load Systems * *: Player data, world state
- **Document Serialization * *: Complex document structures
- **Database Persistence * *: Object - relational mapping

This system provides a robust foundation for any application that needs to serialize complex object graphs while maintaining pointer relationships and memory safety.
*/

/** Instance of the new swizzle manager. */
PhobosSwizzleManagerClass PhobosSwizzleManager;

/**
 *  Retrieves pointers to the supported interfaces on an object.
 *
 *  @author:    tomsons26, CCHyper, ZivDero
 *
 *  @param      riid    The interface to this object being queried for.
 *
 *  @param      ppv     Buffer to fill with obtained interface.
 *
 *  @return     S_OK if interface obtained; E_NOINTERFACE otherwise.
 */
LONG STDMETHODCALLTYPE PhobosSwizzleManagerClass::QueryInterface(REFIID riid, LPVOID* ppv)
{
	if (ppv == nullptr)
	{
		return E_POINTER;
	}

	*ppv = nullptr;
	if (riid == __uuidof(IUnknown))
	{
		*ppv = reinterpret_cast<IUnknown*>(this);
	}
	else if (riid == __uuidof(ISwizzle))
	{
		*ppv = reinterpret_cast<ISwizzle*>(this);
	}

	if (*ppv != nullptr)
	{
		static_cast<IUnknown*>(*ppv)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}


/**
 *  Increments the reference count for an interface pointer to a COM object.
 *
 *  @author: CCHyper
 */
ULONG STDMETHODCALLTYPE PhobosSwizzleManagerClass::AddRef()
{
	return S_FALSE;
}


/**
 *  Decrements the reference count for an interface on a COM object.
 *
 *  @author: CCHyper
 */
ULONG STDMETHODCALLTYPE PhobosSwizzleManagerClass::Release()
{
	return S_FALSE;
}


/**
 *  Reset swizzler in preparation for load.
 *
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE PhobosSwizzleManagerClass::Reset()
{
	Process_Tables();
	UnresolvedBucket.clear();
	DependencyMap.clear();
	return S_OK;
}



/**
 *  Swizzle a pointer after load (requests new pointer).
 *
 *  @author: CCHyper, ZivDero
 */
LONG STDMETHODCALLTYPE PhobosSwizzleManagerClass::Swizzle(void** pointer)
{
	if (pointer == nullptr)
	{
		return E_POINTER;
	}

	uintptr_t id = reinterpret_cast<uintptr_t>(*pointer);
	if (!id)
	{
		return S_OK;
	}

	RequestTable.emplace_back(id, pointer);

	*pointer = nullptr;

	return S_OK;
}


/**
 *  Convert pointer to ID number.
 *
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE PhobosSwizzleManagerClass::Fetch_Swizzle_ID(void* pointer, LONG* id)
{
	if (pointer == nullptr || id == nullptr)
	{
		return E_POINTER;
	}

	*id = reinterpret_cast<uintptr_t>(pointer);

	// Debug::Log("SwizzleManager::Fetch_Swizzle_ID() - ID: 0x%08X.\n", *id);

	return S_OK;
}


/**
 *  Inform (register) swizzler of new object location.
 *
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE PhobosSwizzleManagerClass::Here_I_Am(LONG id, void* pointer)
{
	PointerTable.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(id, pointer));

	return S_OK;
}


/**
 *  Save interface pointer to stream.
 *
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE PhobosSwizzleManagerClass::Save_Interface(IStream* stream, IUnknown* pointer)
{
	return E_NOTIMPL;
}


/**
 *  Loads interface pointer from stream.
 *
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE PhobosSwizzleManagerClass::Load_Interface(IStream* stream, CLSID* riid, void** pointer)
{
	return E_NOTIMPL;
}


/**
 *  Fetch bytes required to save interface pointer.
 *
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE PhobosSwizzleManagerClass::Get_Save_Size(LONG* psize)
{
	if (psize == nullptr)
	{
		return E_POINTER;
	}

	*psize = sizeof(LONG);

	return S_OK;
}


/**
 *  The default class constructor.
 *
 *  @author: CCHyper, ZivDero
 */
PhobosSwizzleManagerClass::PhobosSwizzleManagerClass()
	: RequestTable(), PointerTable(), UnresolvedBucket(), DependencyMap(), AutoNullDanglingPointers(TRUE)
{
	RequestTable.reserve(1000);
	PointerTable.reserve(1000);
	UnresolvedBucket.reserve(100);
	DependencyMap.reserve(500);
}



/**
 *  The class destructor.
 *
 *  @author: CCHyper
 */
PhobosSwizzleManagerClass::~PhobosSwizzleManagerClass()
{
	Process_Tables();
	Cleanup_Dangling_Pointers();
	ExtensionSwizzleManager::CleanupUnmappedExtensions();
}

#define ENABLE_SWIZZLE_DEBUG_PRINTING

/**
 *  Process and remap pointers in the tables.
 *
 *  @author: tomsons26, CCHyper, ZivDero
 */
void PhobosSwizzleManagerClass::Process_Tables()
{
	if (!RequestTable.empty())
	{
#ifdef ENABLE_SWIZZLE_DEBUG_PRINTING
		Debug::Log("SwizzleManager::Process_Tables() - RequestTable.Count %d.\n", RequestTable.size());
		Debug::Log("SwizzleManager::Process_Tables() - PointerTable.Count %d.\n", PointerTable.size());
#endif

		for (SwizzlePointerStruct& request : RequestTable)
		{
#ifdef ENABLE_SWIZZLE_DEBUG_PRINTING
			Debug::Log("SwizzleManager::Process_Tables() - Processing request \"%s\" from %s.\n",
					   request.Variable.c_str(), request.Function.c_str());
#endif

			auto it = PointerTable.find(request.ID);
			if (it != PointerTable.end())
			{
				/** The id's match, remap the pointer. */
				uintptr_t* ptr = (uintptr_t*)request.Pointer;
				*ptr = reinterpret_cast<uintptr_t>(it->second.Pointer);

#ifdef ENABLE_SWIZZLE_DEBUG_PRINTING
				Debug::Log("SwizzleManager::Process_Tables() - Remapped \"%s\" (ID: %08X) to 0x%08X.\n",
						   request.Variable.c_str(), request.ID, reinterpret_cast<uintptr_t>(it->second.Pointer));
#endif
			}
			else
			{
				/** The id's not present, track as unresolved. */
				Track_Unresolved_Pointer(request);

				Debug::Log("SwizzleManager::Process_Tables() - Failed to remap pointer ID:%08X, added to unresolved bucket.\n", request.ID);

				if (!request.Variable.empty())
				{
					Debug::Log("SwizzleManager::Process_Tables() - Unresolved pointer info:\n File: %s\n Line: %d\n Function: %s\n Variable: %s\n",
							   !request.File.empty() ? request.File.c_str() : "<no-filename-info>",
							   request.Line,
							   !request.Function.empty() ? request.Function.c_str() : "<no-function-info>",
							   !request.Variable.empty() ? request.Variable.c_str() : "<no-variable-info>");
				}
			}
		}

		/** Clear the request table after processing. */
		RequestTable.clear();
		PointerTable.clear();
	}
}

/**
 *  Swizzle a pointer after load (requests new pointer). [Debug version]
 *
 *  @author: CCHyper, ZivDero
 */
LONG STDAPICALLTYPE PhobosSwizzleManagerClass::Swizzle_Dbg(void** pointer, const char* file, const int line, const char* func, const char* var)
{
	if (pointer == nullptr)
	{
		return E_POINTER;
	}

	uintptr_t id = reinterpret_cast<uintptr_t>(*pointer);
	if (!id)
	{
		return S_OK;
	}

	RequestTable.emplace_back(id, pointer, file, line, func, var);

	*pointer = nullptr;

#ifdef ENABLE_SWIZZLE_DEBUG_PRINTING
	Debug::Log("SwizzleManager::Swizzle() - Requested remap for \"%s\" (0x%08X) in %s.\n", var, id, func);
#endif

	return S_OK;
}


/**
 *  Convert pointer to ID number. [Debug version]
 *
 *  @author: CCHyper, ZivDero
 */
LONG STDAPICALLTYPE PhobosSwizzleManagerClass::Fetch_Swizzle_ID_Dbg(void* pointer, LONG* id, const char* file, const int line, const char* func, const char* var)
{
	if (pointer == nullptr || id == nullptr)
	{
		return E_POINTER;
	}

	*id = reinterpret_cast<uintptr_t>(pointer);

#ifdef ENABLE_SWIZZLE_DEBUG_PRINTING
	Debug::Log("SwizzleManager::Fetch_Swizzle_ID() - ID: 0x%08X.\n", *id);
	Debug::Log("SwizzleManager::Fetch_Swizzle_ID() - File: %s.\n", file);
	if (line != -1)
	{
		Debug::Log("SwizzleManager::Fetch_Swizzle_ID() - Line: %d.\n", line);
	}
	if (func)
	{
		Debug::Log("SwizzleManager::Fetch_Swizzle_ID() - Func: %s.\n", func);
	}
	if (var)
	{
		Debug::Log("SwizzleManager::Fetch_Swizzle_ID() - Var: %s.\n", var);
	}
#endif

	return S_OK;
}


/**
 *  Inform (register) swizzler of new object location. [Debug version]
 *
 *  @author: CCHyper, ZivDero
 */
LONG STDAPICALLTYPE PhobosSwizzleManagerClass::Here_I_Am_Dbg(LONG id, void* pointer, const char* file, const int line, const char* func, const char* var)
{
	PointerTable.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(id, pointer, file, line, func, var));

#ifdef ENABLE_SWIZZLE_DEBUG_PRINTING
	Debug::Log("SwizzleManager::Here_I_Am() - PointerTable.Count = %d.\n", PointerTable.size());
	Debug::Log("SwizzleManager::Here_I_Am() - Informed swizzler of \"%s\" (0x%08X) in %s.\n", var, id, func);
#endif

	return S_OK;
}

// Reset method

// Register a dependent pointer that references another swizzled object
LONG STDMETHODCALLTYPE PhobosSwizzleManagerClass::Register_Dependent_Pointer(
	LONG referenced_id,
	void** dependent_pointer,
	const char* file,
	const int line,
	const char* func,
	const char* var)
{

	if (dependent_pointer == nullptr)
	{
		return E_POINTER;
	}

	// Store the dependency relationship
	DependencyMap[dependent_pointer] = referenced_id;

#ifdef ENABLE_SWIZZLE_DEBUG_PRINTING
	Debug::Log("SwizzleManager::Register_Dependent_Pointer() - Registered dependency: %s -> ID:%08X in %s.\n",
			   var ? var : "unknown", referenced_id, func ? func : "unknown");
#endif

	return S_OK;
}

// Track an unresolved pointer and find its dependents
void PhobosSwizzleManagerClass::Track_Unresolved_Pointer(const SwizzlePointerStruct& request)
{
	auto& unresolved_info = UnresolvedBucket.try_emplace(request.ID, request.ID).first->second;

	// Find all pointers that depend on this unresolved ID
	for (const auto& [dependent_ptr, referenced_id] : DependencyMap)
	{
		if (referenced_id == request.ID)
		{
			SwizzlePointerStruct dangling_ptr;
			dangling_ptr.ID = referenced_id;
			dangling_ptr.Pointer = dependent_ptr;
			dangling_ptr.Variable = "dependent_pointer";
			dangling_ptr.Function = "unknown";
			dangling_ptr.File = "unknown";
			dangling_ptr.Line = -1;

			unresolved_info.DanglingPointers.push_back(dangling_ptr);
		}
	}

#ifdef ENABLE_SWIZZLE_DEBUG_PRINTING
	Debug::Log("SwizzleManager::Track_Unresolved_Pointer() - Tracked %d dangling pointers for unresolved ID:%08X.\n",
			   unresolved_info.DanglingPointers.size(), request.ID);
#endif
}

// Cleanup references to a specific unresolved pointer
void PhobosSwizzleManagerClass::Cleanup_References_To_Unresolved(LONG unresolved_id)
{
	auto it = UnresolvedBucket.find(unresolved_id);
	if (it == UnresolvedBucket.end())
	{
		return;
	}

	const auto& unresolved_info = it->second;

#ifdef ENABLE_SWIZZLE_DEBUG_PRINTING
	Debug::Log("SwizzleManager::Cleanup_References_To_Unresolved() - Cleaning up %d dangling pointers for ID:%08X.\n",
			   unresolved_info.DanglingPointers.size(), unresolved_id);
#endif

	// Clean up all dangling pointers
	for (const auto& dangling_ptr : unresolved_info.DanglingPointers)
	{
		if (AutoNullDanglingPointers)
		{
			// Null out the dangling pointer
			void** ptr_addr = static_cast<void**>(dangling_ptr.Pointer);
			*ptr_addr = nullptr;

#ifdef ENABLE_SWIZZLE_DEBUG_PRINTING
			Debug::Log("SwizzleManager::Cleanup_References_To_Unresolved() - Nulled dangling pointer at 0x%p.\n", ptr_addr);
#endif
		}
	}
}

// Cleanup all dangling pointers
void STDMETHODCALLTYPE PhobosSwizzleManagerClass::Cleanup_Dangling_Pointers()
{
#ifdef ENABLE_SWIZZLE_DEBUG_PRINTING
	Debug::Log("SwizzleManager::Cleanup_Dangling_Pointers() - Cleaning up %d unresolved pointer buckets.\n",
			   UnresolvedBucket.size());
#endif

	for (const auto& [unresolved_id, unresolved_info] : UnresolvedBucket)
	{
		Cleanup_References_To_Unresolved(unresolved_id);
	}

	UnresolvedBucket.clear();
	DependencyMap.clear();
}

// Get count of unresolved pointers
LONG STDMETHODCALLTYPE PhobosSwizzleManagerClass::Get_Unresolved_Count(LONG* count)
{
	if (count == nullptr)
	{
		return E_POINTER;
	}

	*count = static_cast<LONG>(UnresolvedBucket.size());
	return S_OK;
}

// Set cleanup mode
void STDMETHODCALLTYPE PhobosSwizzleManagerClass::Set_Cleanup_Mode(BOOL null_dangling_pointers)
{
	AutoNullDanglingPointers = null_dangling_pointers;
}