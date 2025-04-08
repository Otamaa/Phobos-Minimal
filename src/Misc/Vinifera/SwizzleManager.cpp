#include <Interface/ISwizzle.h>
#include <ArrayClasses.h>

#include <Utilities/Debug.h>

/*
	@author        CCHyper ,tomsons26
*/

static struct SwizzleInfoDatabaseEntry
{
	SwizzleInfoDatabaseEntry() :
		ReturnAddress(0x00000000),
		Line(0),
		File(),
		Function(),
		Variable()
	{
		__stosb(reinterpret_cast<unsigned char*>(File), 0, sizeof(File));
		__stosb(reinterpret_cast<unsigned char*>(Function), 0, sizeof(Function));
		__stosb(reinterpret_cast<unsigned char*>(Variable), 0, sizeof(Variable));
	}

	bool operator==(const SwizzleInfoDatabaseEntry& src) const { return false; }
	bool operator!=(const SwizzleInfoDatabaseEntry& src) const { return true; }

	uint32_t ReturnAddress;
	char Variable[512];
	char Function[512];
	char File[512];
	uint32_t Line;
};

static OPTIONALINLINE DynamicVectorClass<SwizzleInfoDatabaseEntry> SwizzleInfoDatabase;

static void Add_Swizzle_Database_Entry(uint32_t retaddr, char* function, char* variable, char* file, int line = -1)
{
	SwizzleInfoDatabaseEntry info;

	info.ReturnAddress = retaddr;
	std::strncpy(info.Variable, variable, sizeof(info.Variable));
	std::strncpy(info.Function, function, sizeof(info.Function));

	/**
	 *  The original Tiberian Sun source tree path.
	 */
	static std::string TIBSUN_SOURCE_PATH = "D:\\Projects\\Sun\\CodeFS\\";
	// Add the Tiberian Sun source path (as we know it) to the source name.
	const std::string filepath = TIBSUN_SOURCE_PATH + file;
	std::strncpy(info.File, filepath.data(), sizeof(info.File));
	info.Line = line;
	SwizzleInfoDatabase.AddItem(info);
}

static SwizzleInfoDatabaseEntry* Swizzle_Find_Database_Entry(uintptr_t retaddr)
{
	for (int i = 0; i < SwizzleInfoDatabase.Count; ++i) {
		if (SwizzleInfoDatabase[i].ReturnAddress == retaddr) {
			return &SwizzleInfoDatabase[i];
		}
	}

	return nullptr;
}


class PhobosSwizzleManagerClass : public ISwizzle
{
private:
	struct SwizzlePointerStruct
	{
		SwizzlePointerStruct() :
			ID(-1), Pointer(nullptr), File(nullptr), Line(-1), Function(nullptr), Variable(nullptr)
		{
		}

		SwizzlePointerStruct(LONG id, void* pointer, const char* file = nullptr, const int line = -1, const char* func = nullptr, const char* var = nullptr) :
			ID(id), Pointer(pointer), File(file), Line(line), Function(func), Variable(var)
		{
		}

		~SwizzlePointerStruct() { }

		void operator=(const SwizzlePointerStruct& that)
		{
			ID = that.ID;
			Pointer = that.Pointer;
			File = that.File;
			Line = that.Line;
			Function = that.Function;
			Variable = that.Variable;
		}

		bool operator==(const SwizzlePointerStruct& that) const { return ID == that.ID; }
		bool operator!=(const SwizzlePointerStruct& that) const { return ID != that.ID; }
		bool operator<(const SwizzlePointerStruct& that) const { return ID < that.ID; }
		bool operator>(const SwizzlePointerStruct& that) const { return ID > that.ID; }

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
		const char* File;
		/*const*/ int Line;
		const char* Function;
		const char* Variable;
	};

public:
	/**
	 *  IUnknown
	 */
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj) override
	{
		if (ppvObj == nullptr)
		{
			return E_POINTER;
		}

		*ppvObj = nullptr;
		if (riid == __uuidof(IUnknown))
		{
			if (reinterpret_cast<IUnknown*>(this) != nullptr)
			{
				*ppvObj = reinterpret_cast<IUnknown*>(this);
			}
		}
		else if (riid == __uuidof(ISwizzle))
		{
			if (reinterpret_cast<ISwizzle*>(this) != nullptr)
			{
				*ppvObj = reinterpret_cast<ISwizzle*>(this);
			}
		}

		if (*ppvObj != nullptr)
		{
			reinterpret_cast<IUnknown*>(*ppvObj)->AddRef();
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	STDMETHOD_(ULONG, AddRef)() override
	{
		return S_FALSE;
	}

	STDMETHOD_(ULONG, Release)() override
	{
		return S_FALSE;
	}

	/**
	 *  ISwizzle
	 */
	STDMETHOD_(LONG, Reset)() override
	{
		Process_Tables();

		return S_OK;
	}

	STDMETHOD_(LONG, Swizzle)(void** pointer) override
	{
		if (pointer == nullptr)
		{
			return E_POINTER;
		}

		uintptr_t id = uintptr_t(*pointer);
		if (!id)
		{
			return S_OK;
		}

		SwizzlePointerStruct pair(id, pointer);
		bool added = RequestTable.AddItem(pair);
		*pointer = nullptr;

		return (added == true ? S_OK : S_FALSE);
	}

	STDMETHOD_(LONG, Fetch_Swizzle_ID)(void* pointer, LONG* id) const override
	{
		if (pointer == nullptr || id == nullptr)
		{
			return E_POINTER;
		}

		*id = reinterpret_cast<uintptr_t>(pointer);

		//DEV_DEBUG_INFO("SwizzleManager::Fetch_Swizzle_ID() - ID: 0x%08X.\n", *id);

		return S_OK;
	}

	STDMETHOD_(LONG, Here_I_Am)(LONG id, void* pointer) override
	{
		SwizzlePointerStruct pair(id, pointer);
		return (PointerTable.AddItem(pair) == true ? S_OK : S_FALSE);
	}

	STDMETHOD(Save_Interface)(IStream* stream, IUnknown* pointer) override
	{
		return E_NOTIMPL;
	}

	STDMETHOD(Load_Interface)(IStream* stream, CLSID* riid, void** pointer) override
	{
		return E_NOTIMPL;
	}

	STDMETHOD_(LONG, Get_Save_Size)(LONG* size) const override
	{
		if (size == nullptr)
		{
			return E_POINTER;
		}

		*size = sizeof(LONG);

		return S_OK;
	}

	/**
	 *  New debug routines.
	 */
	STDMETHOD_(LONG, Swizzle_Dbg)(void** pointer, const char* file, const int line, const char* func = nullptr, const char* var = nullptr)
	{
		if (pointer == nullptr)
		{
			return E_POINTER;
		}

		uintptr_t id = uintptr_t(*pointer);
		if (!id)
		{
			return S_OK;
		}

		SwizzlePointerStruct pair(id, pointer, file, line, func, var);
		bool added = RequestTable.AddItem(pair);

		Debug::LogInfo("SwizzleManager::Swizzle() - Requested remap for \"{}\" (0x{}) in {}.", var, id, func);

		*pointer = nullptr;
		return (added == true ? S_OK : S_FALSE);
	}

	STDMETHOD_(LONG, Fetch_Swizzle_ID_Dbg)(void* pointer, LONG* id, const char* file, const int line, const char* func = nullptr, const char* var = nullptr)
	{
		if (pointer == nullptr || id == nullptr)
		{
			return E_POINTER;
		}

		*id = reinterpret_cast<uintptr_t>(pointer);

		Debug::LogInfo("SwizzleManager::Fetch_Swizzle_ID() - ID: 0x{}.", *id);
		Debug::LogInfo("SwizzleManager::Fetch_Swizzle_ID() - File: {}.", file);

		if (line != -1) {
			Debug::LogInfo("SwizzleManager::Fetch_Swizzle_ID() - Line: {}.", line);
		}

		if (func) {
			Debug::LogInfo("SwizzleManager::Fetch_Swizzle_ID() - Func: {}.", func);
		}

		if (var) {
			Debug::LogInfo("SwizzleManager::Fetch_Swizzle_ID() - Var: {}.", var);
		}

		return S_OK;
	}



	STDMETHOD_(LONG, Here_I_Am_Dbg)(LONG id, void* pointer, const char* file, const int line, const char* func = nullptr, const char* var = nullptr)
	{
		SwizzlePointerStruct pair(id, pointer, file, line, func, var);

		bool added = PointerTable.AddItem(pair);

		Debug::LogInfo("SwizzleManager::Here_I_Am() - PointerTable.Count = {}.", PointerTable.Count);
		Debug::LogInfo("SwizzleManager::Here_I_Am() - Informed swizzler of \"{}\" (0x{}) in {}.", var, id, func);

		return (added == true ? S_OK : S_FALSE);
	}

public:
	PhobosSwizzleManagerClass() :
		RequestTable(),
		PointerTable()
	{
		RequestTable.CapacityIncrement = (1000);
		PointerTable.CapacityIncrement = (1000);
	}

	~PhobosSwizzleManagerClass()
	{
		Process_Tables();
	}

private:
	void Sort_Tables()
	{
		if (PointerTable.Count > 0)
		{
			std::qsort(&PointerTable[0], PointerTable.Count, sizeof(SwizzlePointerStruct), ptr_compare_func);
		}
		if (RequestTable.Count > 0)
		{
			std::qsort(&RequestTable[0], RequestTable.Count, sizeof(SwizzlePointerStruct), ptr_compare_func);
		}
	}

	void Process_Tables()
	{
		if (RequestTable.Count > 0)
		{

			Sort_Tables();

			int request_index = 0;
			int request_count = RequestTable.Count;

			int pointer_index = 0;
			int pointer_count = PointerTable.Count;

			Debug::LogInfo("SwizzleManager::Process_Tables() - RequestTable.Count {}.", request_count);
			Debug::LogInfo("SwizzleManager::Process_Tables() - PointerTable.Count {}.", pointer_count);

			while (request_count > 0)
			{

				int pre_search_id = RequestTable[request_index].ID;
				int ptr_id = PointerTable[pointer_index].ID;

				Debug::LogInfo("SwizzleManager::Process_Tables() - Processing request \"{}\" from {}.", RequestTable[request_index].Variable, RequestTable[request_index].Function);

				if (pre_search_id == ptr_id)
				{

					/**
					 *  The id's match, remap the pointer.
					 */
					uintptr_t* ptr = (uintptr_t*)RequestTable[request_index].Pointer;
					*ptr = (uintptr_t)PointerTable[pointer_index].Pointer;

					Debug::LogInfo("SwizzleManager::Process_Tables() - Remapped \"{}\" (ID: {}) to 0x{}.",
							RequestTable[request_index].Variable, RequestTable[request_index].ID, (uintptr_t)PointerTable[pointer_index].Pointer);

					++request_index;
					--request_count;

					continue;

				}

				/**
				 *  Perform a quick search.
				 */
				while (pre_search_id > ptr_id)
				{
					++pointer_index;
					--pointer_count;
					ptr_id = PointerTable[pointer_index].ID;
				}

				void* old_ptr = RequestTable[request_index].Pointer;
				int new_id = PointerTable[pointer_index].ID;

				/**
				 *  #NOTE: Original code was divide by zero to force a crash!
				 */
				bool failed = (pre_search_id != new_id);

				/**
				 *  Non matching id's means we failed to remap!
				 */
				if (failed)
				{

					Debug::LogInfo("SwizzleManager::Process_Tables() - Failed to remap a pointer from the save file!");

					/**
					 *  If there is additional debug information attached to this
					 *  pointer, then throw an assertion instead.
					 */
					if (RequestTable[request_index].Variable != nullptr)
					{

						SwizzlePointerStruct& req = RequestTable[request_index];

						/**
						 *  If a variable value has been set, then it will be a
						 *  pointer from the original game code. Use this as we
						 *  have no line information.
						 */
						static char buffer[1024];

						Debug::LogInfo("SwizzleManager::Process_Tables() - Request info:\n  File: {}\n  Line: {}\n  Function: {}\n  Variable: {}",
											req.File ? req.File : "<no-filename-info>",
											req.Line,
											req.Function ? req.Function : "<no-function-info>",
											req.Variable ? req.Variable : "<no-variable-info>");

						std::snprintf(buffer, sizeof(buffer),
								"SwizzleManager failed to remap a pointer from the save file!\n\n"
								"Additional debug information:\n"
								"  File: %s\n"
								"  Line: %d\n"
								"  Function: %s\n"
								"  Variable: %s\n"

								"\nThe game will now return to the main menu.\n",

								req.File ? req.File : "<no-filename-info>",
								req.Line,
								req.Function ? req.Function : "<no-function-info>",
								req.Variable ? req.Variable : "<no-variable-info>");

						MessageBox(Game::hWnd(), buffer, "Phobos", MB_OK | MB_ICONEXCLAMATION);

					}
					else
					{

					MessageBox(Game::hWnd(), "SwizzleManager failed to remap a pointer from the save file!\n\nThe game will now return to the main menu.", "Phobos", MB_OK | MB_ICONEXCLAMATION);


					}


					/**
					 *  #BUGFIX:
					 *  Clear all surfaces to remove any blitting artifacts.
					 */
					Debug::FreeMouse();

					/**
					 *  Return to the main menu. This is abusing the exception return
					 *  address information, which points back to the Select_Game
					 *  call in Main_Game.
					 */
					{
						static CONTEXT _ctx;
						ZeroMemory(&_ctx, sizeof(_ctx));

						RtlCaptureContext(&_ctx);

						DWORD* ebp = &(_ctx.Ebp);
						DWORD* esp = &(_ctx.Esp);
						DWORD* eip = &(_ctx.Eip);
						*ebp = Game::ExceptionReturnBase;
						*esp = Game::ExceptionReturnStack;
						*eip = Game::ExceptionReturnAddress;
					}

					return; // For clean binary analysis.
				}

			}

			/**
			 *  We fixed up all pointers, clear the tables.
			 */
			RequestTable.Clear();
			PointerTable.Clear();
		}

	}

private:
	/**
	 *  List of all the pointers that need remapping.
	 */
	DynamicVectorClass<SwizzlePointerStruct> RequestTable;

	/**
	 *  List of all the new pointers.
	 */
	DynamicVectorClass<SwizzlePointerStruct> PointerTable;

private:
	static int __cdecl ptr_compare_func(const void* ptr1, const void* ptr2)
	{
		const SwizzlePointerStruct* p1 = static_cast<const SwizzlePointerStruct*>(ptr1);
		const SwizzlePointerStruct* p2 = static_cast<const SwizzlePointerStruct*>(ptr2);

		if (p1->ID == p2->ID)
		{
			return 0;
		}
		if (p1->ID < p2->ID)
		{
			return -1;
		}
		return 1;
	}
};


static class SwizzleManagerClassExt final : public PhobosSwizzleManagerClass
{
public:
	COM_DECLSPEC_NOTHROW LONG STDAPICALLTYPE _Reset()
	{
	//DEV_DEBUG_INFO("SwizzleManager::Reset - retaddr 0x%08X id 0x%08X pointer 0x%08X\n", (uintptr_t)_ReturnAddress(), id, pointer);

	/**
	 *  Get the caller return address, we use this to identify a location in which the request was made.
	 */
		uintptr_t retaddr = (uintptr_t)_ReturnAddress();

		switch (retaddr)
		{
		case 0x0067E4F7:
			Debug::LogInfo("Reset() - From Load_Game");
			break;
		case 0x00687C00:
			Debug::LogInfo("Reset() - From Read_Scenario_INI");
			break;
		};

		return Reset();
	}


	COM_DECLSPEC_NOTHROW LONG STDAPICALLTYPE _Swizzle(void** pointer)
	{
		//DEV_DEBUG_INFO("SwizzleManager::Swizzle - retaddr 0x%08X id 0x%08X pointer 0x%08X\n", (uintptr_t)_ReturnAddress(), id, pointer);

		/**
		 *  Get the caller return address, we use this to identify a location in which the request was made.
		 */
		uintptr_t retaddr = (uintptr_t)_ReturnAddress();

		/**
		 *  Fetch the caller debug information based off the return address.
		 */
		SwizzleInfoDatabaseEntry* info = Swizzle_Find_Database_Entry(retaddr);

		if (!info) {
			Debug::LogInfo("Swizzle() - Failed to find debug information for 0x{}!", retaddr);


			/**
			 *  Return failure!
			 */
			return E_UNEXPECTED;

		} else {
			Debug::LogInfo("Swizzle() - Debug info found:\n  File: {}\n  Line: {}\n  Function: {}\n  Var: {}", info->File, info->Line, info->Function, info->Variable);
		}

		return Swizzle_Dbg(pointer, info->File, info->Line, info->Function, info->Variable);
	}

	COM_DECLSPEC_NOTHROW LONG STDAPICALLTYPE _Fetch_Swizzle_ID(void* pointer, LONG* id)
	{
		//DEV_DEBUG_INFO("SwizzleManager::Fetch_Swizzle_ID - retaddr 0x%08X id 0x%08X pointer 0x%08X\n", (uintptr_t)_ReturnAddress(), id, pointer);

		/**
		 *  Get the caller return address, we use this to identify a location in which the request was made.
		 */
		uintptr_t retaddr = (uintptr_t)_ReturnAddress();

		/**
		 *  Fetch the caller debug information based off the return address.
		 */
		SwizzleInfoDatabaseEntry* info = Swizzle_Find_Database_Entry(retaddr);
		if (!info)
		{
			Debug::LogInfo("Fetch_Swizzle_ID() - Failed to find debug information for 0x{}!", retaddr);

			/**
			 *  Return failure!
			 */
			return E_UNEXPECTED;

		} else {
			Debug::LogInfo("Fetch_Swizzle_ID() - Debug info found:\n  File: {}\n  Line: {}\n  Function: {}\n  Var: {}", info->File, info->Line, info->Function, info->Variable);
		}

		return Fetch_Swizzle_ID_Dbg(pointer, id, info->File, info->Line, info->Function, info->Variable);
	}


	COM_DECLSPEC_NOTHROW LONG STDAPICALLTYPE _Here_I_Am(LONG id, void* pointer)
	{
		//DEV_DEBUG_INFO("SwizzleManager::Here_I_Am - retaddr 0x%08X id 0x%08X pointer 0x%08X\n", (uintptr_t)_ReturnAddress(), id, pointer);

		/**
		 *  Get the caller return address, we use this to identify a location in which the annoucement was made.
		 */
		uintptr_t retaddr = (uintptr_t)_ReturnAddress();

		/**
		 *  Fetch the caller debug information based off the return address.
		 */
		SwizzleInfoDatabaseEntry* info = Swizzle_Find_Database_Entry(retaddr);

		if (!info)
		{
			Debug::LogInfo("Here_I_Am() - Failed to find debug information for 0x{}!", retaddr);

			/**
			 *  Return failure!
			 */
			return E_UNEXPECTED;

		} else {
			Debug::LogInfo("Here_I_Am() - Debug info found:\n  File: {}\n  Line: {}\n  Function: {}\n  Var: {}", info->File, info->Line, info->Function, info->Variable);
		}

		return Here_I_Am_Dbg(id, pointer, info->File, info->Line, info->Function, info->Variable);
	}
};
