#include "Swizzle.h"

#include <Surface.h>
#include <Exception.h>

/**
 *  Instance of the new swizzle manager.
 */
PhobosSwizzleManagerClass PhobosSwizzleManager;

void Clear_All_Surfaces()
{
	if (DSurface::Primary()) DSurface::Primary->Clear();
	if (DSurface::Hidden()) DSurface::Hidden->Clear();
	if (DSurface::Hidden_2()) DSurface::Hidden_2->Clear();
	if (DSurface::Alternate()) DSurface::Alternate->Clear();
	if (DSurface::Sidebar()) DSurface::Sidebar->Clear();
	if (DSurface::Composite()) DSurface::Composite->Clear();
}

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
PhobosSwizzleManagerClass::PhobosSwizzleManagerClass() :
	RequestTable(),
	PointerTable()
{
	RequestTable.reserve(1000);
	PointerTable.reserve(1000);
}


/**
 *  The class destructor.
 *
 *  @author: CCHyper
 */
PhobosSwizzleManagerClass::~PhobosSwizzleManagerClass()
{
	Process_Tables();
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
			Debug::Log("SwizzleManager::Process_Tables() - Processing request \"%s\" from %s.\n", request.Variable.c_str(), request.Function.c_str());
#endif

			auto it = PointerTable.find(request.ID);
			if (it != PointerTable.end())
			{

				/**
				 *  The id's match, remap the pointer.
				 */
				uintptr_t* ptr = (uintptr_t*)request.Pointer;
				*ptr = reinterpret_cast<uintptr_t>(it->second.Pointer);

#ifdef ENABLE_SWIZZLE_DEBUG_PRINTING
				Debug::Log("SwizzleManager::Process_Tables() - Remapped \"%s\" (ID: %08X) to 0x%08X.\n", request.Variable.c_str(), request.ID, reinterpret_cast<uintptr_t>(request.Pointer));
#endif
			}
			else
			{

				/**
				 *  The id's not present, remap failed.
				 */
				Debug::Log("SwizzleManager::Process_Tables() - Failed to remap a pointer from the save file!\n");

				///**
				// *  If there is additional debug information attached to this
				// *  pointer, then throw an assertion instead.
				// */
				if (!request.Variable.empty())
				{

					/**
					 *  If a variable value has been set, then it will be a
					 *  pointer from the original game code. Use this as we
					 *  have no line information.
					 */
					static char buffer[1024];

					Debug::Log("SwizzleManager::Process_Tables() - Request[%d] info:\n  File: %s\n  Line: %d\n  Function: %s\n  Variable: %s\n"
									, request.ID
									, !request.File.empty() ? request.File.c_str() : "<no-filename-info>"
									, request.Line
									, !request.Function.empty() ? request.Function.c_str() : "<no-function-info>"
									, !request.Variable.empty() ? request.Variable.c_str() : "<no-variable-info>");

					std::snprintf(buffer, sizeof(buffer),
								  "SwizzleManager failed to remap a pointer from the save file!\n\n"
								  "Additional debug information:\n"
								  "  File: %s\n"
								  "  Line: %d\n"
								  "  Function: %s\n"
								  "  Variable: %s\n"

								  "\nThe game will now exit.\n"

								, !request.File.empty() ? request.File.c_str() : "<no-filename-info>"
								, request.Line
								, !request.Function.empty() ? request.Function.c_str() : "<no-function-info>"
								, !request.Variable.empty() ? request.Variable.c_str() : "<no-variable-info>");

					Debug::Log("%s", buffer);
					//DebugBreak();
					//MessageBox(Game::hWnd(), buffer, "Phobos Swizzle", MB_OK | MB_ICONEXCLAMATION);

				}
				else
				{
					Debug::Log("SwizzleManager failed to remap a pointer from the save file! requestID[%d] .", request.ID);
					//MessageBox(Game::hWnd(), "SwizzleManager failed to remap a pointer from the save file!\n\nThe game will now exit.", "Phobos Swizzle", MB_OK | MB_ICONEXCLAMATION);
				}

				//// Fatal("SwizzleManager failed to remap a pointer from the save file!\n");
				//Debug::ExitGame(EXIT_FAILURE);
				//return; // For clean binary analysis.
			}
		}

		/**
		 *  We fixed up all pointers, clear the tables.
		 */
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