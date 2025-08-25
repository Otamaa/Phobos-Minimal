
#include "Header.h"

#include <Utilities/Patch.h>

#include <EventClass.h>
#include <FPSCounter.h>

#include <stdnoreturn.h>

#include <Utilities/Macro.h>

#include <Misc/PhobosGlobal.h>
#include <Misc/SyncLogging.h>

#include <AnimClass.h>
#include <FootClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <AircraftClass.h>
#include <BuildingClass.h>

#include <GameModeOptionsClass.h>
#include <GameOptionsClass.h>
#include <Phobos.version.h>

#pragma warning(push)
#pragma warning(disable: 4646) // this function does not return, though it isn't declared VOID
#pragma warning(disable: 4477)
#pragma warning(disable: 4715)

static COMPILETIMEEVAL reference<DynamicVectorClass<ObjectClass*>*, 0x87F778u> const Logics {};

[[ noreturn ]] static void DoRecon(REGISTERS* R)
{
	// mimic an increment because decrement happens in the middle of function cleanup and can't be erased nicely
	++Unsorted::SystemResponseMessages;
	const auto hwnd = IsWindow(Game::hWnd()) ? Game::hWnd() : nullptr;

	Debug::LogInfo("Reconnection error detected!");

	if (MessageBoxW(hwnd, L"Yuri's Revenge has detected a desynchronization!\n"
		L"Would you like to create a full error report for the developers?\n"
		L"Be advised that reports from at least two players are needed.", L"Reconnection Error!",
		MB_YESNO | MB_ICONERROR) == IDYES)
	{

		Debug::DumpStack(R, 8084);

		HCURSOR loadCursor = LoadCursor(nullptr, IDC_WAIT);
		if (hwnd)
			SetClassLong(hwnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));

		SetCursor(loadCursor);

		std::wstring path = Debug::PrepareSnapshotDirectory();

		Debug::LogInfo("Making a memory snapshot");
		Debug::FullDump(std::move(path));

		if (Debug::LogEnabled)
		{
			std::wstring _bar = Debug::LogFileMainName + Debug::LogFileExt;
			_bar += L" Copied. - Yuri's Revenge";

			MessageBoxW(hwnd, Debug::LogFileFullPath.c_str(), _bar.c_str(), MB_OK | MB_ICONERROR);
			CopyFileW(Debug::LogFileFullPath.c_str(), (path + L"\\" + Debug::LogFileMainName + Debug::LogFileExt).c_str(), FALSE);
		}

		loadCursor = LoadCursor(nullptr, IDC_ARROW);
		if (hwnd)
			SetClassLong(hwnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));

		SetCursor(loadCursor);
		Debug::FatalError("A desynchronization has occurred.\r\n"
			"%s"
			"A crash dump should have been created in your game's \\debug subfolder.\r\n"
			"Please submit that to the developers along with SYNC*.txt, debug.txt and syringe.log."
				, Phobos::Otamaa::ParserErrorDetected ? "(One or more parser errors have been detected that might be responsible. Check the debug logs.)\r" : ""
		);

		char logFilename[0x40];

		if (Game::EnableMPSyncDebug) {
			for (int i = 0; i < 256; i++) {
				sprintf_s(logFilename, std::size(logFilename) - 1, Debug::SyncFileFormat2.c_str(), HouseClass::CurrentPlayer->ArrayIndex, i);
				SyncLogger::WriteSyncLog(logFilename);
			}
		} else {
			sprintf_s(logFilename, std::size(logFilename) - 1, Debug::SyncFileFormat.c_str(), HouseClass::CurrentPlayer->ArrayIndex);
			SyncLogger::WriteSyncLog(logFilename);
		}

		Debug::ExitGame<true>(0);
	}
}

struct ExceptionHandler
{
public:
	// Named constants for crash addresses
	static constexpr DWORD CRASH_ADDRESS_1 = 0x7BC806;
	static constexpr DWORD CRASH_FIX_1 = 0x7BC80F;

	static constexpr DWORD CRASH_WAYPOINT_90 = 0x5D6C21;
	static constexpr DWORD CRASH_FIX_WAYPOINT_90 = 0x5D6C36;

	static constexpr DWORD CRASH_DSURFACE_GETPIXEL = 0x7BAEA1;
	static constexpr DWORD CRASH_FIX_DSURFACE_GETPIXEL = 0x7BAEA8;

	static constexpr DWORD CRASH_KEYBOARD_COMMAND = 0x535DBC;
	static constexpr DWORD CRASH_FIX_KEYBOARD_COMMAND = 0x535DCE;

	static constexpr DWORD CRASH_PATHFINDING_1 = 0x42C554;
	static constexpr DWORD CRASH_PATHFINDING_2 = 0x42C53E;
	static constexpr DWORD CRASH_PATHFINDING_3 = 0x42C507;

	static constexpr DWORD CRASH_SUBZONE_TRACKING = 0x584DF7;
	static constexpr DWORD CRASH_YURI_MIND_CONTROL = 0x55E018;

	static constexpr int EXCEPTION_STACK_DEPTH_MAX = 1024;
	static constexpr int EXCEPTION_STACK_COLUMNS = 8;
	static constexpr size_t MEMORY_DUMP_BYTES = 32;

	// Exception type mappings using fixed array for better performance
	struct ExceptionMapping
	{
		DWORD code;
		std::string_view name;
	};

	static constexpr std::array<ExceptionMapping, 23> ExceptionNames = { {
		{EXCEPTION_ACCESS_VIOLATION, "Access Violation"},                    // 1
		{EXCEPTION_ARRAY_BOUNDS_EXCEEDED, "Array Bounds Exceeded"},          // 2
		{EXCEPTION_BREAKPOINT, "Breakpoint"},                                // 3
		{EXCEPTION_DATATYPE_MISALIGNMENT, "Datatype Misalignment"},          // 4
		{EXCEPTION_FLT_DENORMAL_OPERAND, "Floating Point Denormal Operand"}, // 5
		{EXCEPTION_FLT_DIVIDE_BY_ZERO, "Floating Point Divide by Zero"},     // 6
		{EXCEPTION_FLT_INEXACT_RESULT, "Floating Point Inexact Result"},     // 7
		{EXCEPTION_FLT_INVALID_OPERATION, "Floating Point Invalid Operation"}, // 8
		{EXCEPTION_FLT_OVERFLOW, "Floating Point Overflow"},                 // 9
		{EXCEPTION_FLT_STACK_CHECK, "Floating Point Stack Check"},           // 10
		{EXCEPTION_FLT_UNDERFLOW, "Floating Point Underflow"},               // 11
		{EXCEPTION_ILLEGAL_INSTRUCTION, "Illegal Instruction"},              // 12
		{EXCEPTION_IN_PAGE_ERROR, "Page Fault"},                             // 13
		{EXCEPTION_INT_DIVIDE_BY_ZERO, "Integer Divide by Zero"},            // 14
		{EXCEPTION_INT_OVERFLOW, "Integer Overflow"},                        // 15
		{EXCEPTION_INVALID_DISPOSITION, "Invalid Disposition"},              // 16
		{EXCEPTION_NONCONTINUABLE_EXCEPTION, "Noncontinuable Exception"},    // 17
		{EXCEPTION_PRIV_INSTRUCTION, "Privileged Instruction"},              // 18
		{EXCEPTION_SINGLE_STEP, "Single Step"},                              // 19
		{EXCEPTION_STACK_OVERFLOW, "Stack Overflow"},                        // 20
		{0xE06D7363, "C++ Exception Thrown"},                                // 21
		{ERROR_MOD_NOT_FOUND, "Module Not Found"},                           // 22
		{ERROR_PROC_NOT_FOUND, "Procedure Not Found"}                        // 23
	} };

	// RAII file wrapper
	class FileHandle
	{
	private:
		FILE* file_;
	public:
		explicit FileHandle(const std::wstring& filename, const wchar_t* mode)
			: file_(nullptr)
		{
			_wfopen_s(&file_, filename.c_str(), mode);
		}

		~FileHandle()
		{
			if (file_)
			{
				fclose(file_);
			}
		}

		FILE* get() const { return file_; }
		operator bool() const { return file_ != nullptr; }

		// Non-copyable
		FileHandle(const FileHandle&) = delete;
		FileHandle& operator=(const FileHandle&) = delete;
	};

	// Helper function for safe window operations
	static HWND GetSafeWindowHandle()
	{
		const auto hwnd = Game::hWnd();
		return IsWindow(hwnd) ? hwnd : nullptr;
	}

	// Write formatted string to file using fmt
	template<typename... Args>
	static void WriteToFile(FILE* file, const std::string& format_str, Args&&... args)
	{
		if (file)
		{
			const auto formatted = fmt::format(fmt::runtime(format_str), std::forward<Args>(args)...);
			fputs(formatted.c_str(), file);
		}
	}

	// Safe memory reading
	static bool SafeReadMemory(const void* address, void* buffer, size_t size)
	{
		__try
		{
			memcpy(buffer, address, size);
			return true;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
	}

	// Handle specific crash addresses
	static bool HandleSpecificCrash(DWORD address, PEXCEPTION_POINTERS pExs, std::string& reason)
	{
		DWORD* eip_pointer = reinterpret_cast<DWORD*>(&pExs->ContextRecord->Eip);

		switch (address)
		{
		case CRASH_ADDRESS_1:
			*eip_pointer = CRASH_FIX_1;
			return true;

		case CRASH_WAYPOINT_90:
			// This bug most likely happens when a map doesn't have Waypoint 90
			*eip_pointer = CRASH_FIX_WAYPOINT_90;
			return true;

		case CRASH_DSURFACE_GETPIXEL:
			// A common crash in DSurface::GetPixel
			*eip_pointer = CRASH_FIX_DSURFACE_GETPIXEL;
			pExs->ContextRecord->Ebx = 0;
			return true;

		case CRASH_KEYBOARD_COMMAND:
			// Common crash in keyboard command class
			*eip_pointer = CRASH_FIX_KEYBOARD_COMMAND;
			pExs->ContextRecord->Esp += 12;
			return true;

		case CRASH_PATHFINDING_1:
		case CRASH_PATHFINDING_2:
		case CRASH_PATHFINDING_3:
			reason = "PathfindingCrash";
			return false;

		case CRASH_SUBZONE_TRACKING:
			reason = "SubzoneTrackingCrash";
			return false;

		case 0x000000:
			if (pExs->ContextRecord->Esp &&
				*(DWORD*)pExs->ContextRecord->Esp == CRASH_YURI_MIND_CONTROL)
			{
				// Common crash when Yuri Prime mind controls a building and dies while user presses hotkeys
				*eip_pointer = CRASH_YURI_MIND_CONTROL;
				pExs->ContextRecord->Esp += 8;
				return true;
			}
			break;

		default:
			return false;
		}

		return false;
	}

	// Write memory dump with better formatting
	static void WriteMemoryDump(FILE* file, PCONTEXT pCtxt)
	{
		WriteToFile(file, "Bytes at CS:EIP (0x{:08X}): ", pCtxt->Eip);

		std::array<uint8_t, MEMORY_DUMP_BYTES> memory_buffer {};
		const auto* eip_pointer = reinterpret_cast<const uint8_t*>(pCtxt->Eip);

		for (size_t i = 0; i < MEMORY_DUMP_BYTES; ++i)
		{
			if (SafeReadMemory(eip_pointer + i, &memory_buffer[i], sizeof(uint8_t)))
			{
				WriteToFile(file, "{:02X} ", memory_buffer[i]);
			}
			else
			{
				WriteToFile(file, "?? ");
			}
		}
		WriteToFile(file, "\n");
	}

	// Write register information
	static void WriteRegisterInfo(FILE* file, PCONTEXT pCtxt)
	{
		WriteToFile(file, "\nRegisters:\n");
		WriteToFile(file, "EIP: {:08X}\tESP: {:08X}\tEBP: {:08X}\n",
				   pCtxt->Eip, pCtxt->Esp, pCtxt->Ebp);
		WriteToFile(file, "EAX: {:08X}\tEBX: {:08X}\tECX: {:08X}\n",
				   pCtxt->Eax, pCtxt->Ebx, pCtxt->Ecx);
		WriteToFile(file, "EDX: {:08X}\tESI: {:08X}\tEDI: {:08X}\n",
				   pCtxt->Edx, pCtxt->Esi, pCtxt->Edi);
		WriteToFile(file, "CS:  {:04x}\tSS:  {:04x}\tDS:  {:04x}\n",
				   pCtxt->SegCs, pCtxt->SegSs, pCtxt->SegDs);
		WriteToFile(file, "ES:  {:04x}\tFS:  {:04x}\tGS:  {:04x}\n",
				   pCtxt->SegEs, pCtxt->SegFs, pCtxt->SegGs);
		WriteToFile(file, "EFlags: {:08X}\n\n", pCtxt->EFlags);
	}

	// Write floating point information
	static void WriteFloatingPointInfo(FILE* file, PCONTEXT pCtxt)
	{
		WriteToFile(file, "Floating point status:\n");
		WriteToFile(file, "Control word:\t{:08x}\n", pCtxt->FloatSave.ControlWord);
		WriteToFile(file, "Status word:\t{:08x}\n", pCtxt->FloatSave.StatusWord);
		WriteToFile(file, "Tag word:\t{:08x}\n", pCtxt->FloatSave.TagWord);
		WriteToFile(file, "Error Offset:\t{:08x}\n", pCtxt->FloatSave.ErrorOffset);
		WriteToFile(file, "Error Selector:\t{:08x}\n", pCtxt->FloatSave.ErrorSelector);
		WriteToFile(file, "Data Offset:\t{:08x}\n", pCtxt->FloatSave.DataOffset);
		WriteToFile(file, "Data Selector:\t{:08x}\n", pCtxt->FloatSave.DataSelector);
		WriteToFile(file, "Cr0NpxState:\t{:08x}\n\n", pCtxt->FloatSave.Spare0);

		WriteToFile(file, "Floating point Registers:\n");
		for (int d = 0; d < EXCEPTION_STACK_COLUMNS; ++d)
		{
			WriteToFile(file, "ST{}: ", d);

			for (int j = 0; j < 10; ++j)
			{
				WriteToFile(file, "{:02X}", pCtxt->FloatSave.RegisterArea[d * 10 + j]);
			}

			const double* fp_val = reinterpret_cast<const double*>(&pCtxt->FloatSave.RegisterArea[d * 10]);
			WriteToFile(file, "   {:+#.17e}\n", *fp_val);
		}
	}

	// Write stack dump
	static void WriteStackDump(FILE* file, PCONTEXT pCtxt)
	{
		WriteToFile(file, "\nStack dump (depth: {}):\n", EXCEPTION_STACK_DEPTH_MAX);

		const auto* ptr = reinterpret_cast<const DWORD*>(pCtxt->Esp);
		for (int c = 0; c < EXCEPTION_STACK_DEPTH_MAX; ++c)
		{
			DWORD value;
			if (!SafeReadMemory(ptr + c, &value, sizeof(DWORD)))
			{
				WriteToFile(file, "{:08p}: ???????? <unreadable>\n", (void*)(ptr + c));
				continue;
			}

			std::string_view suffix = "";
			if (value >= 0x401000 && value <= 0xB79BE4)
			{
				suffix = "GameMemory!";
			}
			else
			{
				for (auto it = Patch::ModuleDatas.begin() + 1; it != Patch::ModuleDatas.end(); ++it)
				{
					if (value >= it->BaseAddr && value <= (it->BaseAddr + it->Size))
					{
						suffix = fmt::format("{} Memory!", it->ModuleName);
						break;
					}
				}
			}

			WriteToFile(file, "{:08p}: {:08X} {}\n", (void*)(ptr + c), value, suffix);
		}
	}

	// Get exception name using binary search on sorted array
	static std::string_view GetExceptionName(DWORD code)
	{
		auto it = std::find_if(ExceptionNames.begin(), ExceptionNames.end(),
			[code](const ExceptionMapping& mapping)
 {
	 return mapping.code == code;
			});

		if (it != ExceptionNames.end())
		{
			return it->name;
		}
		return "Unknown Exception";
	}

public:
	[[noreturn]] static LONG __fastcall Handle(int code, PEXCEPTION_POINTERS pExs)
	{
		DWORD* eip_pointer = reinterpret_cast<DWORD*>(&pExs->ContextRecord->Eip);
		std::string reason = "Unknown";
		const auto hwnd = GetSafeWindowHandle();

		// Try to handle specific crashes that can be recovered
		if (HandleSpecificCrash(*eip_pointer, pExs, reason))
		{
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		Debug::FreeMouse();
		Debug::LogInfo("Exception handler fired reason {}!", reason);
		Debug::Log("Exception 0x{:x} at 0x{:x}\n",
				  pExs->ExceptionRecord->ExceptionCode,
				  pExs->ExceptionRecord->ExceptionAddress);
		Game::StreamerThreadFlush();

		if (hwnd)
		{
			SetWindowTextW(hwnd, L"Fatal Error - Yuri's Revenge");
		}

		const std::wstring path = Debug::PrepareSnapshotDirectory();
		const DWORD exception_code = pExs->ExceptionRecord->ExceptionCode;

		// Handle fatal exceptions
		if (IsFatalException(exception_code))
		{
			HandleFatalException(pExs, path, hwnd);
		}
		else
		{
			HandleNonFatalException(exception_code);
		}

		Debug::ExitGame<true>(exception_code);
		return 0u;
	}

private:
	static bool IsFatalException(DWORD code)
	{
		static constexpr std::array<DWORD, 21> fatal_exceptions = { {
			EXCEPTION_ACCESS_VIOLATION,           // 1
			EXCEPTION_ARRAY_BOUNDS_EXCEEDED,      // 2
			EXCEPTION_BREAKPOINT,                 // 3
			EXCEPTION_DATATYPE_MISALIGNMENT,      // 4
			EXCEPTION_FLT_DENORMAL_OPERAND,       // 5
			EXCEPTION_FLT_DIVIDE_BY_ZERO,         // 6
			EXCEPTION_FLT_INEXACT_RESULT,         // 7
			EXCEPTION_FLT_INVALID_OPERATION,      // 8
			EXCEPTION_FLT_OVERFLOW,               // 9
			EXCEPTION_FLT_STACK_CHECK,            // 10
			EXCEPTION_FLT_UNDERFLOW,              // 11
			EXCEPTION_ILLEGAL_INSTRUCTION,        // 12
			EXCEPTION_IN_PAGE_ERROR,              // 13
			EXCEPTION_INT_DIVIDE_BY_ZERO,         // 14
			EXCEPTION_INT_OVERFLOW,               // 15
			EXCEPTION_INVALID_DISPOSITION,        // 16
			EXCEPTION_NONCONTINUABLE_EXCEPTION,   // 17
			EXCEPTION_PRIV_INSTRUCTION,           // 18
			EXCEPTION_SINGLE_STEP,                // 19
			EXCEPTION_STACK_OVERFLOW,             // 20
			0xE06D7363 // C++ exception thrown and not caught  // 21
		} };

		return std::find(fatal_exceptions.begin(), fatal_exceptions.end(), code) != fatal_exceptions.end();
	}

	static void HandleFatalException(PEXCEPTION_POINTERS pExs, const std::wstring& path, HWND hwnd)
	{
		const std::wstring except_file = path + L"\\except.txt";

		// Copy debug log if enabled
		if (Debug::LogEnabled)
		{
			std::wstring bar = Debug::LogFileMainName + Debug::LogFileExt + L" Copied. - Yuri's Revenge";
			MessageBoxW(hwnd, Debug::LogFileFullPath.c_str(), bar.c_str(), MB_OK | MB_ICONERROR);
			CopyFileW(Debug::LogFileFullPath.c_str(),
					 (path + L"\\" + Debug::LogFileMainName + Debug::LogFileExt).c_str(), FALSE);
		}

		// Write exception report
		FileHandle except_handle(except_file, L"w");
		if (except_handle)
		{
			WriteExceptionReport(except_handle.get(), pExs);
			Debug::LogInfo("Exception data has been saved to file: {}",
						  PhobosCRT::WideStringToString(except_file));
		}

		// Ask user about crash dump
		if (MessageBoxW(hwnd,
			L"Yuri's Revenge has encountered a fatal error!\n"
			L"Would you like to create a full crash report for the developers?",
			L"Fatal Error!", MB_YESNO | MB_ICONERROR) == IDYES)
		{
			CreateCrashDump(pExs, path, hwnd);
		}
	}

	static void WriteExceptionReport(FILE* file, PEXCEPTION_POINTERS pExs)
	{
		constexpr auto delimiter = "------------------------------------------------------------------------------------\n";

		WriteToFile(file, "Internal Error encountered!\n");
		WriteToFile(file, delimiter);
		WriteToFile(file, "Ares version: 21.352.1218 With Phobos {}\n", PRODUCT_VERSION);
		WriteToFile(file, "Running on {}\n", Patch::WindowsVersion);
		WriteToFile(file, delimiter);
		WriteToFile(file, "\n");

		// Write module information
		int i = 0;
		for (const auto& data : Patch::ModuleDatas)
		{
			WriteToFile(file, "Module [({}) {}: Base address = {:x}]\n",
					   i++, data.ModuleName, data.BaseAddr);
		}

		WriteToFile(file, "\n");
		WriteExceptionDetails(file, pExs);
		WriteMemoryDump(file, pExs->ContextRecord);
		WriteRegisterInfo(file, pExs->ContextRecord);
		WriteFloatingPointInfo(file, pExs->ContextRecord);
		WriteMMXRegisters(file, pExs->ContextRecord);
		WriteDebugRegisters(file, pExs->ContextRecord);
		WriteStackDump(file, pExs->ContextRecord);
		WriteGameSpecificInfo(file);
	}

	// Additional helper methods...
	static void WriteExceptionDetails(FILE* file, PEXCEPTION_POINTERS pExs)
	{
		const DWORD code = pExs->ExceptionRecord->ExceptionCode;
		const auto name = GetExceptionName(code);

		WriteToFile(file, "Exception: {} (0x{:08X}) at {:08p}\n",
				   name, code, pExs->ExceptionRecord->ExceptionAddress);

		if (code == EXCEPTION_ACCESS_VIOLATION)
		{
			WriteAccessViolationDetails(file, pExs);
		}
	}

	static void WriteAccessViolationDetails(FILE* file, PEXCEPTION_POINTERS pExs)
	{
		const auto info = pExs->ExceptionRecord->ExceptionInformation[0];
		const auto address = pExs->ExceptionRecord->ExceptionInformation[1];

		std::string_view violation_type;
		switch (info)
		{
		case 0: violation_type = "read from"; break;
		case 1: violation_type = "written to"; break;
		case 2: violation_type = "executed from"; break;
		case 8: violation_type = "DEP violation at"; break;
		default: violation_type = "unknown violation at"; break;
		}

		WriteToFile(file, "Access address: 0x{:08X} was {}\n", address, violation_type);
	}

	static void HandleNonFatalException(DWORD code)
	{
		switch (code)
		{
		case ERROR_MOD_NOT_FOUND:
		case ERROR_PROC_NOT_FOUND:
			Debug::LogInfo("Massive failure: Procedure or module not found!");
			break;
		default:
			Debug::LogInfo("Massive failure: reason unknown, have fun figuring it out");
			break;
		}
	}

	static void CreateCrashDump(PEXCEPTION_POINTERS pExs, const std::wstring& path, HWND hwnd)
	{
		HCURSOR loadCursor = LoadCursor(nullptr, IDC_WAIT);

		if (hwnd)
		{
			SetClassLong(hwnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
		}

		SetCursor(loadCursor);
		Debug::LogInfo("Making a memory dump");

		MINIDUMP_EXCEPTION_INFORMATION expParam {};
		expParam.ThreadId = GetCurrentThreadId();
		expParam.ExceptionPointers = pExs;
		expParam.ClientPointers = FALSE;

		Debug::FullDump(path, &expParam);

		loadCursor = LoadCursor(nullptr, IDC_ARROW);
		if (hwnd)
		{
			SetClassLong(hwnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
		}

		SetCursor(loadCursor);
		Debug::FatalError("The cause of this error could not be determined.\r\n"
			"%s"
			"A crash dump should have been created in your game's \\debug subfolder.\r\n"
			"You can submit that to the developers (along with debug.txt and syringe.log).",
			Phobos::Otamaa::ParserErrorDetected ?
				"(One or more parser errors have been detected that might be responsible. Check the debug logs.)\r" : ""
		);
	}

	static void WriteDebugRegisters(FILE* file, PCONTEXT pCtxt)
	{
		WriteToFile(file, "\nDebug Registers:\n");
		WriteToFile(file, "Dr0: {:016llX}\tDr1: {:016llX}\tDr2: {:016llX}\tDr3: {:016llX}\n",
			pCtxt->Dr0, pCtxt->Dr1, pCtxt->Dr2, pCtxt->Dr3);
		WriteToFile(file, "Dr4: OBSOLETE\tDr5: OBSOLETE\tDr6: {:08X}\tDr7: {:08X}\n",
			pCtxt->Dr6, pCtxt->Dr7);
	}

	static void WriteMMXRegisters(FILE* file, PCONTEXT pCtxt)
	{
		if (IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE))
		{
			WriteToFile(file, "\nMMX Registers:\n");
			WriteToFile(file, "MMX0:\t{:016llX}\tMMX1:\t{:016llX}\tMMX2:\t{:016llX}\tMMX3:\t{:016llX}\n",
				pCtxt->ExtendedRegisters[0], pCtxt->ExtendedRegisters[1],
				pCtxt->ExtendedRegisters[2], pCtxt->ExtendedRegisters[3]);
			WriteToFile(file, "MMX4:\t{:016llX}\tMMX5:\t{:016llX}\tMMX6:\t{:016llX}\tMMX7:\t{:016llX}\n",
				pCtxt->ExtendedRegisters[4], pCtxt->ExtendedRegisters[5],
				pCtxt->ExtendedRegisters[6], pCtxt->ExtendedRegisters[7]);
		}
	}

	static void WriteGameSpecificInfo(FILE* file)
	{
		// Write last animation info
		const auto& last_anim = PhobosGlobal::Instance()->LastAnimName;
		if (!last_anim.empty())
		{
			WriteToFile(file, "\nLast Animation: {}\n", last_anim);
			Debug::LogInfo("LastAnim Calling CTOR ({})", last_anim);
		}

		// Write pathfinding info
		auto& pp = PhobosGlobal::Instance()->PathfindTechno;
		if (pp.IsValid())
		{
			std::string_view pTechnoID = GameStrings::NoneStr();
			std::string_view what = GameStrings::NoneStr();

			if (pp.Finder)
			{
				const auto vtable = VTable::Get(pp.Finder);
				if (vtable == UnitClass::vtable)
				{
					pTechnoID = pp.Finder->get_ID();
					what = "UnitClass";
				}
				else if (vtable == InfantryClass::vtable)
				{
					what = "InfantryClass";
					pTechnoID = pp.Finder->get_ID();
				}
			}

			WriteToFile(file, "Last Pathfind: ({:p})[{}] - [{}] from ({} - {}) to ({} - {})\n",
				static_cast<void*>(pp.Finder), what, pTechnoID,
				pp.From.X, pp.From.Y, pp.To.X, pp.To.Y);

			Debug::LogInfo("LastPathfind ({:p})[{}] - [{}] from ({} - {}) to ({} - {})",
				static_cast<void*>(pp.Finder), what, pTechnoID,
				pp.From.X, pp.From.Y, pp.To.X, pp.To.Y);
		}
		pp.Clear();
	}
};

DEFINE_HOOK(0x64CCBF, DoList_ReplaceReconMessage, 6)
{
	DoRecon(R);
	return 0x64CD11;
}

LONG __fastcall ExceptionHandler(int code, PEXCEPTION_POINTERS const pExs)
{

	DWORD* eip_pointer = reinterpret_cast<DWORD*>(&pExs->ContextRecord->Eip);
	std::string reason = "Unknown";
	const auto hwnd = IsWindow(Game::hWnd()) ? Game::hWnd() : nullptr;

	switch (*eip_pointer)
	{
	case 0x7BC806:
	{
		*eip_pointer = 0x7BC80F;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	case 0x5D6C21:
	{
		// This bug most likely happens when a map Doesn't have Waypoint 90
		*eip_pointer = 0x5D6C36;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	case 0x7BAEA1:
	{
		// A common crash in DSurface::GetPixel
		*eip_pointer = 0x7BAEA8;
		pExs->ContextRecord->Ebx = 0;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	case 0x535DBC:
	{
		// Common crash in keyboard command class
		*eip_pointer = 0x535DCE;
		pExs->ContextRecord->Esp += 12;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	case 0x42C554:
	case 0x42C53E:
	case 0x42C507:
	{
		//FootClass* pFoot = (FootClass*)(ExceptionInfo->ContextRecord->Ebp + 0x14);
		//CellStruct* pFrom = (CellStruct*)(ExceptionInfo->ContextRecord->Ebp + 0x8);
		//CellStruct* pTo = (CellStruct*)(ExceptionInfo->ContextRecord->Ebp + 0xC);
		//MovementZone movementZone = (MovementZone)(ExceptionInfo->ContextRecord->Ebp + 0x10);

		//AstarClass , broken ptr
		reason = ("PathfindingCrash");
		break;
	}
	case 0x584DF7:
		reason = ("SubzoneTrackingCrash");
		break;
		//case 0x755C7F:
		//{
		//	Debug::LogInfo("BounceAnimError ");
		//	return PrintException(exception_id, ExceptionInfo);
		//}
	case 0x000000:
		if (pExs->ContextRecord->Esp && *(DWORD*)pExs->ContextRecord->Esp == 0x55E018)
		{
			// A common crash that seems to happen when yuri prime mind controls a building and then dies while the user is pressing hotkeys
			*eip_pointer = 0x55E018;
			pExs->ContextRecord->Esp += 8;
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		break;
	default:
		break;
	}

	Debug::FreeMouse();
	Debug::LogInfo("Exception handler fired reason {} !", reason);
	Debug::Log("Exception 0x%x at 0x%x\n", pExs->ExceptionRecord->ExceptionCode, pExs->ExceptionRecord->ExceptionAddress);
	Game::StreamerThreadFlush();

	//the value of `reference<HWND> Game::hWnd` is stored on the stack instead of inlined as memory value, using `.get()` doesnot seems fixed it
	//so using these oogly
	if (hwnd)
		SetWindowTextW(hwnd, L"Fatal Error - Yuri's Revenge");

	std::wstring path = Debug::PrepareSnapshotDirectory();

	switch (pExs->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
	case EXCEPTION_BREAKPOINT:
	case EXCEPTION_DATATYPE_MISALIGNMENT:
	case EXCEPTION_FLT_DENORMAL_OPERAND:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_INEXACT_RESULT:
	case EXCEPTION_FLT_INVALID_OPERATION:
	case EXCEPTION_FLT_OVERFLOW:
	case EXCEPTION_FLT_STACK_CHECK:
	case EXCEPTION_FLT_UNDERFLOW:
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	case EXCEPTION_IN_PAGE_ERROR:
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_INT_OVERFLOW:
	case EXCEPTION_INVALID_DISPOSITION:
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
	case EXCEPTION_PRIV_INSTRUCTION:
	case EXCEPTION_SINGLE_STEP:
	case EXCEPTION_STACK_OVERFLOW:
	case 0xE06D7363: // exception thrown and not caught
	{
		const std::wstring except_file = path + L"\\except.txt";

		if (Debug::LogEnabled)
		{
			std::wstring _bar = Debug::LogFileMainName + Debug::LogFileExt;
			_bar += L" Copied. - Yuri's Revenge";

			MessageBoxW(hwnd, Debug::LogFileFullPath.c_str(), _bar.c_str(), MB_OK | MB_ICONERROR);
			CopyFileW(Debug::LogFileFullPath.c_str(), (path + L"\\" + Debug::LogFileMainName + Debug::LogFileExt).c_str(), FALSE);
		}

		if (FILE* except = _wfsopen(except_file.c_str(), L"w", _SH_DENYNO))
		{
			COMPILETIMEEVAL auto const pDelim = "------------------------------------------------------------------------------------\n";
			fprintf(except, "Internal Error encountered!\n");
			fprintf(except, pDelim);
			fprintf(except, "Ares version: 21.352.1218 With Phobos %s\n", PRODUCT_VERSION); //TODO
			fprintf(except, "Running on %s\n", Patch::WindowsVersion.c_str());
			fprintf(except, pDelim);

			fprintf(except, "\n");

			int i = 0;
			for (auto const& data : Patch::ModuleDatas)
			{
				fprintf(except, "Module [(%d) %s: Base address = %x]\n", i++, data.ModuleName.c_str(), data.BaseAddr);
			}

			fprintf(except, "\n");
			switch (pExs->ExceptionRecord->ExceptionCode)
			{
			case EXCEPTION_STACK_OVERFLOW:
				fprintf(except, "Exception is stack overflow! (0x%08X) at %08p\n", pExs->ExceptionRecord->ExceptionCode, pExs->ExceptionRecord->ExceptionAddress);
				break;
			case EXCEPTION_ACCESS_VIOLATION:
			{
				std::string VioType {};
				switch (pExs->ExceptionRecord->ExceptionInformation[0])
				{
				case 0: // Read violation
					VioType = ("Access address: 0x%08X was read from.\n");
					break;
				case 1: // Write violation
					VioType = ("Access address: 0x%08X was written to.\n");
					break;
				case 2: // Execute violation
					VioType = ("Access address: 0x%08X was written to.\n");
					break;
				case 8: // User-mode data execution prevention (DEP).
					VioType = ("Access address: 0x%08X DEP violation.\n");
					break;
				default: // Unknown
					VioType = ("Access address: 0x%08X Unknown violation.\n");
					break;
				};
				std::string type = "Exception is access violation (0x%08X) at %08p ";
				type += VioType;
				fprintf(except, type.c_str(), pExs->ExceptionRecord->ExceptionCode, pExs->ExceptionRecord->ExceptionAddress, pExs->ExceptionRecord->ExceptionInformation[1]);
			}
			break;
			case EXCEPTION_IN_PAGE_ERROR:
				fprintf(except, "Exception is page fault (0x%08X) at %08p\n", pExs->ExceptionRecord->ExceptionCode, pExs->ExceptionRecord->ExceptionAddress);
				break;
			default:
				fprintf(except, "Exception code is 0x%08X at %08p\n", pExs->ExceptionRecord->ExceptionCode, pExs->ExceptionRecord->ExceptionAddress);
				break;
			};

			PCONTEXT pCtxt = pExs->ContextRecord;
			fprintf(except, "Bytes at CS:EIP (0x%08X)  : ", pCtxt->Eip);
			uint8_t* _eip_pointer = reinterpret_cast<uint8_t*>(pCtxt->Eip);

			for (int e = 32; e > 0; --e)
			{
				if (IsBadReadPtr(_eip_pointer, sizeof(uint8_t)))
				{
					fprintf(except, "?? ");
				}
				else
				{
					fprintf(except, "%02X ", (uintptr_t)*_eip_pointer);
				}
				++_eip_pointer;
			}

			fprintf(except, "\n\nRegisters:\n");
			fprintf(except, "EIP: %08X\tESP: %08X\tEBP: %08X\t\n", pCtxt->Eip, pCtxt->Esp, pCtxt->Ebp);
			fprintf(except, "EAX: %08X\tEBX: %08X\tECX: %08X\n", pCtxt->Eax, pCtxt->Ebx, pCtxt->Ecx);
			fprintf(except, "EDX: %08X\tESI: %08X\tEDI: %08X\n", pCtxt->Edx, pCtxt->Esi, pCtxt->Edi);
			fprintf(except, "CS:  %04x\tSS:  %04x\tDS:  %04x\n", pCtxt->SegCs, pCtxt->SegSs, pCtxt->SegDs);
			fprintf(except, "ES:  %04x\tFS:  %04x\tGS:  %04x\n", pCtxt->SegEs, pCtxt->SegFs, pCtxt->SegGs);
			fprintf(except, "\n");

			fprintf(except, "EFlags: %08X\n", pCtxt->EFlags);

			fprintf(except, "\n");

			fprintf(except, "Floating point status:\n");
			fprintf(except, "Control word:\t%08x\n", pCtxt->FloatSave.ControlWord);
			fprintf(except, "Status word:\t%08x\n", pCtxt->FloatSave.StatusWord);
			fprintf(except, "Tag word:\t%08x\n", pCtxt->FloatSave.TagWord);
			fprintf(except, "Error Offset:\t%08x\n", pCtxt->FloatSave.ErrorOffset);
			fprintf(except, "Error Selector:\t%08x\n", pCtxt->FloatSave.ErrorSelector);
			fprintf(except, "Data Offset:\t%08x\n", pCtxt->FloatSave.DataOffset);
			fprintf(except, "Data Selector:\t%08x\n", pCtxt->FloatSave.DataSelector);
			fprintf(except, "Cr0NpxState:\t%08x\n", pCtxt->FloatSave.Spare0);

			fprintf(except, "\n");

			fprintf(except, "Floating point Registers:\n");

			for (int d = 0; d < ExceptionHandler::EXCEPTION_STACK_COLUMNS; ++d)
			{
				fprintf(except, "ST%d : ", d);

				for (int j = 0; j < 10; ++j)
				{
					fprintf(except, "%02X", pCtxt->FloatSave.RegisterArea[d * 10 + j]);
				}

				fprintf(except, "   %+#.17e\n", *reinterpret_cast<double*>(&pCtxt->FloatSave.RegisterArea[d * 10]));
			}

			if (IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE))
			{
				fprintf(except, "\n");
				fprintf(except, "MMX Registers:\n");

				fprintf(except, "MMX0:	%016llX\tMMX1:	%016llX\tMMX2:	%016llX\tMMX3:	%016llX\n",
					pCtxt->ExtendedRegisters[0],
					pCtxt->ExtendedRegisters[1],
					pCtxt->ExtendedRegisters[2],
					pCtxt->ExtendedRegisters[3]
				);

				fprintf(except, "MMX4:	%016llX\tMMX5:	%016llX\tMMX6:	%016llX\tMMX7:	%016llX\n",
					pCtxt->ExtendedRegisters[4],
					pCtxt->ExtendedRegisters[5],
					pCtxt->ExtendedRegisters[6],
					pCtxt->ExtendedRegisters[7]
				);
			}

			fprintf(except, "\n");

			fprintf(except, "Debug Registers:\n");
			fprintf(except, "Dr0: %016llX\tDr1: %016llX\tDr2: %016llX\tDr3: %016llX\n",
				pCtxt->Dr0,
				pCtxt->Dr1,
				pCtxt->Dr2,
				pCtxt->Dr3
			);

			fprintf(except, "Dr4: OBSOLETE\tDr5: OBSOLETE\tDr6: %08X\tDr7: %08X\n",
				pCtxt->Dr6,
				pCtxt->Dr7
			);

			{
				auto& last_anim = PhobosGlobal::Instance()->LastAnimName;

				if (!last_anim.empty())
				{
					Debug::LogInfo("LastAnim Calling CTOR ({})", last_anim);
				}
			}

			{
				auto& pp = PhobosGlobal::Instance()->PathfindTechno;
				if (pp.IsValid())
				{
					const char* pTechnoID = GameStrings::NoneStr();
					const char* what = GameStrings::NoneStr();
					if (pp.Finder)
					{
						const auto vtable = VTable::Get(pp.Finder);
						if (vtable == UnitClass::vtable)
						{
							pTechnoID = pp.Finder->get_ID();
							what = "UnitClass";
						}
						else if (vtable == InfantryClass::vtable)
						{
							what = "InfantryClass";
							pTechnoID = pp.Finder->get_ID();
						}
					}

					Debug::LogInfo("LastPathfind ({})[{}] - [{}] from ({} - {}) to ({} - {})", (void*)pp.Finder, what, pTechnoID,
						pp.From.X, pp.From.Y,
						pp.To.X, pp.To.Y
					);
				}
				pp.Clear();
			}

			fprintf(except, "\nStack dump (depth : %d):\n", ExceptionHandler::EXCEPTION_STACK_DEPTH_MAX);
			DWORD* ptr = reinterpret_cast<DWORD*>(pCtxt->Esp);
			for (int c = 0; c < ExceptionHandler::EXCEPTION_STACK_DEPTH_MAX; ++c)
			{
				const char* suffix = "";
				if (*ptr >= 0x401000 && *ptr <= 0xB79BE4)
					suffix = "GameMemory!";
				else
				{
					for (auto begin = Patch::ModuleDatas.begin() + 1; begin < Patch::ModuleDatas.end(); ++begin)
					{
						if (*ptr >= begin->BaseAddr && *ptr <= (begin->BaseAddr + begin->Size))
						{
							suffix = (begin->ModuleName + " Memory!").c_str();
							break;
						}
					}
				}

				fprintf(except, "%08p: %08X %s\n", ptr, *ptr, suffix);
				++ptr;
			}

			fclose(except);
			Debug::LogInfo("Exception data has been saved to file: {}", PhobosCRT::WideStringToString(except_file));
		}

		//the value of `reference<HWND> Game::hWnd` is stored on the stack instead of inlined as memory value, using `.get()` doesnot seems fixed it
		//so using these oogly
		if (MessageBoxW(hwnd, L"Yuri's Revenge has encountered a fatal error!\nWould you like to create a full crash report for the developers?", L"Fatal Error!", MB_YESNO | MB_ICONERROR) == IDYES)
		{
			HCURSOR loadCursor = LoadCursor(nullptr, IDC_WAIT);
			//the value of `reference<HWND> Game::hWnd` is stored on the stack instead of inlined as memory value, using `.get()` doesnot seems fixed it
			//so using these oogly
			if (hwnd)
				SetClassLong(hwnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));

			SetCursor(loadCursor);
			Debug::LogInfo("Making a memory dump");

			MINIDUMP_EXCEPTION_INFORMATION expParam {};
			expParam.ThreadId = GetCurrentThreadId();
			expParam.ExceptionPointers = pExs;
			expParam.ClientPointers = FALSE;

			Debug::FullDump(std::move(path), &expParam);

			loadCursor = LoadCursor(nullptr, IDC_ARROW);
			//the value of `reference<HWND> Game::hWnd` is stored on the stack instead of inlined as memory value, using `.get()` doesnot seems fixed it
			//so using these oogly
			if (hwnd)
				SetClassLong(hwnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));

			SetCursor(loadCursor);
			Debug::FatalError("The cause of this error could not be determined.\r\n"
				"%s"
				"A crash dump should have been created in your game's \\debug subfolder.\r\n"
				"You can submit that to the developers (along with debug.txt and syringe.log)."
				, Phobos::Otamaa::ParserErrorDetected ? "(One or more parser errors have been detected that might be responsible. Check the debug logs.)\r" : ""
			);
		}
		break;
	}
	case ERROR_MOD_NOT_FOUND:
	case ERROR_PROC_NOT_FOUND:
		Debug::LogInfo("Massive failure: Procedure or module not found!");
		break;
	default:
		Debug::LogInfo("Massive failure: reason unknown, have fun figuring it out");
		//Debug::DumpObj(reinterpret_cast<byte*>(pExs->ExceptionRecord), sizeof(*(pExs->ExceptionRecord)));
		//return EXCEPTION_CONTINUE_SEARCH;
		break;
	}

	Debug::ExitGame<true>(pExs->ExceptionRecord->ExceptionCode);

	return 0u;
};

DEFINE_FUNCTION_JUMP(LJMP, 0x4C8FE0, ExceptionHandler)

struct Logger
{
private:
	// Helper function to write formatted string to FILE*
	template<typename... Args>
	static void write_to_file(FILE* file, fmt::format_string<Args...> format_str, Args&&... args) {
		const auto formatted = fmt::format(format_str, std::forward<Args>(args)...);
		fputs(formatted.c_str(), file);
	}

public:

	template<typename T>
	static void WriteLog(const T* it, int idx, DWORD checksum, FILE* F) {
		write_to_file(F, "#{:05d}:\t{:08X}", idx, checksum);
	}

	template<>
	static void WriteLog(const AbstractClass* it, int idx, DWORD checksum, FILE* F) {
		WriteLog<void>(it, idx, checksum, F);
		auto abs = it->WhatAmI();
		write_to_file(F, "; Abs: {} ({})", abs, AbstractClass::GetAbstractClassName(abs));
	}

	template<>
	static void WriteLog(const ObjectClass* it, int idx, DWORD checksum, FILE* F) {
		WriteLog<AbstractClass>(it, idx, checksum, F);

		std::string_view typeID = GameStrings::NoneStr();
		int typeIndex = -1;
		if (auto pType = it->GetType()) {
			typeID = pType->ID;
			typeIndex = pType->GetArrayIndex();
		}

		CoordStruct crd = it->GetCoords();
		CellStruct cell = CellClass::Coord2Cell(crd);

		write_to_file(F, "; Type: {} ({}); Coords: {},{},{} ({},{}); Health: {} ; InLimbo: {}",
			typeIndex, typeID, crd.X, crd.Y, crd.Z, cell.X, cell.Y, it->Health, it->InLimbo);
	}

	template<>
	static void WriteLog(const MissionClass* it, int idx, DWORD checksum, FILE* F) {
		WriteLog<ObjectClass>(it, idx, checksum, F);
		const auto Cur = it->GetCurrentMission();
		write_to_file(F, "; Mission: {} ({}); StartTime: {}",
			MissionClass::MissionToString(Cur), Cur, it->CurrentMissionStartTime);
	}

	template<>
	static void WriteLog(const TechnoClass* it, int idx, DWORD checksum, FILE* F) {
		WriteLog<MissionClass>(it, idx, checksum, F);

		std::string_view targetID = GameStrings::NoneStr();
		int targetIndex = -1;
		CoordStruct targetCrd = { -1, -1, -1 };
		if (auto pTarget = it->Target) {
			targetID = AbstractClass::GetAbstractClassName(pTarget->WhatAmI());
			targetIndex = pTarget->GetArrayIndex();
			targetCrd = pTarget->GetCoords();
		}

		write_to_file(F, "; Facing: {}; Facing2: {}; Target: {} ({}; {},{})",
			it->PrimaryFacing.Current().Getvalue8(), it->SecondaryFacing.Current().Getvalue8(),
			targetID, targetIndex, targetCrd.X, targetCrd.Y);
	}

	template<>
	static void WriteLog(const FootClass* it, int idx, DWORD checksum, FILE* F) {
		WriteLog<TechnoClass>(it, idx, checksum, F);

		std::string_view destID = GameStrings::NoneStr();

		int destIndex = -1;
		CoordStruct destCrd = { -1, -1, -1 };
		if (auto pDest = it->Destination) {
			destID = AbstractClass::GetAbstractClassName(pDest->WhatAmI());
			destIndex = pDest->GetArrayIndex();
			destCrd = pDest->GetCoords();
		}

		write_to_file(F, "; Destination: {} ({}; {},{}); SpeedPercentage {} ; Height {}",
			destID, destIndex, destCrd.X, destCrd.Y,
			static_cast<int>(it->SpeedPercentage * 256),
			it->GetHeight());
	}

	template<>
	static void WriteLog(const UnitClass* it, int idx, DWORD checksum, FILE* F) {
		WriteLog<FootClass>(it, idx, checksum, F);

		const auto& Loco = it->Locomotor;
		auto accum = Loco->Get_Speed_Accum();
		auto index = Loco->Get_Track_Index();
		auto number = Loco->Get_Track_Number();

		write_to_file(F, "; SpeedAccum {}; TrackNumber: {}; TrackIndex: {}", accum, number, index);
	}

	template<>
	static void WriteLog(const InfantryClass* it, int idx, DWORD checksum, FILE* F) {
		WriteLog<FootClass>(it, idx, checksum, F);
	}

	template<>
	static void WriteLog(const AircraftClass* it, int idx, DWORD checksum, FILE* F) {
		WriteLog<FootClass>(it, idx, checksum, F);
	}

	template<>
	static void WriteLog(const BuildingClass* it, int idx, DWORD checksum, FILE* F) {
		WriteLog<TechnoClass>(it, idx, checksum, F);
	}

	template<>
	static void WriteLog(const AbstractTypeClass* it, int idx, DWORD checksum, FILE* F) {
		WriteLog<AbstractClass>(it, idx, checksum, F);
		write_to_file(F, "; ID: {}; Name: {}", it->ID, it->Name);
	}

	template<>
	static void WriteLog(const HouseClass* it, int idx, DWORD checksum, FILE* F) {
		WriteLog<void>(it, idx, checksum, F);

		write_to_file(F, "; Player Name : {} ({} - {}); IsHumanPlayer: {}; ColorScheme: {} ({}); Edge: {}; StartingAllies: {}; Startspot: {},{}; Visionary: {}; MapIsClear: {}; Money: {}",
			it->PlainName ? it->PlainName : GameStrings::NoneStr(),
			it->ArrayIndex, HouseTypeClass::Array->Items[it->Type->ArrayIndex]->Name,
			it->IsHumanPlayer, ColorScheme::Array->Items[it->ColorSchemeIndex]->ID, it->ColorSchemeIndex,
			static_cast<int>(it->Edge), it->StartingAllies.data, it->StartingCell.X, it->StartingCell.Y,
			it->Visionary, it->MapIsClear, it->Available_Money());

		if (!it->IsNeutral() && !it->IsControlledByHuman()) {
			write_to_file(F, "\nLogging AI BaseNodes : \n");

			const auto& b = it->Base.BaseNodes;

			for (int j = 0; j < b.Count; ++j) {
				const auto& n = b[j];
				if (n.BuildingTypeIndex >= 0) {
					auto lbl = BuildingTypeClass::Array->Items[n.BuildingTypeIndex]->ID;
					write_to_file(F, "\tNode #{:03d}: {} @ ({:05d}, {:05d}), Attempts so far: {}, Placed: {}\n",
						j, lbl, n.MapCoords.X, n.MapCoords.Y, n.Attempts, n.Placed);
				} else {
					write_to_file(F, "\tNode #{:03d}: Special {} @ ({:05d}, {:05d}), Attempts so far: {}, Placed: {}\n",
						j, n.BuildingTypeIndex, n.MapCoords.X, n.MapCoords.Y, n.Attempts, n.Placed);
				}
			}
			write_to_file(F, "\n");
		}
	}

	// calls WriteLog and appends a newline
	template<typename T>
	static void WriteLogLine(const T* it, int idx, DWORD checksum, FILE* F) {
		WriteLog(it, idx, checksum, F);
		write_to_file(F, "\n");
	}

	template<typename T>
	static void LogItem(const T* it, int idx, FILE* F) {
		if (it->WhatAmI() != AnimClass::AbsID || it->Fetch_ID() != -2) {
			SafeChecksummer Ch {};
			it->ComputeCRC(Ch);
			WriteLogLine(it, idx, Ch.operator unsigned int(), F);
		}
	}

	template<typename T>
	static void VectorLogger(const DynamicVectorClass<T>* Array, FILE* F, const char* Label = nullptr) {
		if (Label) {
			write_to_file(F, "Checksums for [{}] ({}) :\n", Label, Array ? Array->Count : -1);
		}

		if (Array) {
			for (auto i = 0; i < Array->Count; ++i) {
				auto it = Array->Items[i];
				LogItem(it, i, F);
			}
		} else {
			write_to_file(F, "Array not initialized yet...\n");
		}
	}

	template<typename T>
	static void HouseLogger(const DynamicVectorClass<T>* Array, FILE* F, const char* Label = nullptr) {
		if (Array) {
			for (auto j = 0; j < HouseClass::Array->Count; ++j) {
				auto pHouse = HouseClass::Array->Items[j];
				write_to_file(F, "-------------------- {} ({}) {} -------------------\n",
					pHouse->Type->Name, j, Label ? Label : "");

				for (auto i = 0; i < Array->Count; ++i) {
					auto it = Array->Items[i]; if (it->Owner == pHouse)
					{
						LogItem(it, i, F);
					}
				}
			}
		} else {
			write_to_file(F, "Array not initialized yet...\n");
		}
	}

	static bool LogFrame(const std::string& LogFilename, EventClass* OffendingEvent = nullptr) {
		FILE* LogFile = nullptr;

		if (!fopen_s(&LogFile, LogFilename.c_str(), "wt") && LogFile) {
			std::setvbuf(LogFile, nullptr, _IOFBF, 1024 * 1024); // 1024 kb buffer - should be sufficient for whole log

			write_to_file(LogFile, "YR synchronization log\n");
			write_to_file(LogFile, "With Ares [21.352.1218] and Phobos {}\n", PRODUCT_VERSION);
			write_to_file(LogFile, "\n");

			int i = 0;
			for (const auto& data : Patch::ModuleDatas) {
				write_to_file(LogFile, "Module [({}) {}: Base address = {:08X}]\n",
					i++, data.ModuleName, data.BaseAddr);
			}

			write_to_file(LogFile, "\n");

			for (size_t ixF = 0; ixF < EventClass::LatestFramesCRC.c_size(); ++ixF) {
				write_to_file(LogFile, "LastFrame CRC[{:02d}] = {:08X}\n", ixF, EventClass::LatestFramesCRC[ixF]);
			}

			write_to_file(LogFile, "My Random Number: {:08X}\n", ScenarioClass::Instance->Random.Random());
			write_to_file(LogFile, "My Frame: {:08X}\n", Unsorted::CurrentFrame());
			write_to_file(LogFile, "Average FPS: {}\n", static_cast<int>(FPSCounter::GetAverageFrameRate()));
			write_to_file(LogFile, "Max MaxAhead: {}\n", Game::Network::MaxMaxAhead());
			write_to_file(LogFile, "Latency setting: {}\n", Game::Network::LatencyFudge());
			write_to_file(LogFile, "Game speed setting: {}\n", GameOptionsClass::Instance->GameSpeed);
			write_to_file(LogFile, "FrameSendRate: {}\n", Game::Network::FrameSendRate());
			write_to_file(LogFile, "Mod is {} ({}) with {:X}\n",
				AresGlobalData::ModName, AresGlobalData::ModVersion,
				static_cast<unsigned>(AresGlobalData::ModIdentifier));

			if (HouseClass::CurrentPlayer()) {
				write_to_file(LogFile, "Player Name: {}\n", HouseClass::CurrentPlayer->PlainName);
			}

			const auto nHashes = HashData::GetINIChecksums();

			write_to_file(LogFile, "Rules checksum: {:08X}\n", nHashes.Rules);
			write_to_file(LogFile, "Art checksum: {:08X}\n", nHashes.Art);
			write_to_file(LogFile, "AI checksum: {:08X}\n", nHashes.AI);

			if (OffendingEvent)
			{
				write_to_file(LogFile, "\nOffending event:\n");
				write_to_file(LogFile, "Type:         {:X}\n", (unsigned)OffendingEvent->Type);
				write_to_file(LogFile, "Frame:        {:X}\n", OffendingEvent->Frame);
				write_to_file(LogFile, "ID:           {:X}\n", OffendingEvent->HouseIndex);
				write_to_file(LogFile, "CRC:          {:X}({})\n",
					OffendingEvent->Data.FrameInfo.CRC, OffendingEvent->Data.FrameInfo.CRC);
				write_to_file(LogFile, "CommandCount: {}\n", OffendingEvent->Data.FrameInfo.CommandCount);
				write_to_file(LogFile, "Delay:        {}\n", OffendingEvent->Data.FrameInfo.Delay);
				write_to_file(LogFile, "\n\n");
			}

			write_to_file(LogFile, "\n**Types**\n");
			HouseLogger(InfantryClass::Array(), LogFile, "Infantry");
			HouseLogger(UnitClass::Array(), LogFile, "Units");
			HouseLogger(AircraftClass::Array(), LogFile, "Aircraft");
			HouseLogger(BuildingClass::Array(), LogFile, "Buildings");

			write_to_file(LogFile, "\n**Checksums**\n");
			VectorLogger(HouseClass::Array(), LogFile, "Houses");
			VectorLogger(InfantryClass::Array(), LogFile, "Infantry");
			VectorLogger(UnitClass::Array(), LogFile, "Units");
			VectorLogger(AircraftClass::Array(), LogFile, "Aircraft");
			VectorLogger(BuildingClass::Array(), LogFile, "Buildings");

			write_to_file(LogFile, "\n");
			VectorLogger(&ObjectClass::CurrentObjects(), LogFile, "Current Objects");
			VectorLogger(&LogicClass::Instance(), LogFile, "Logics");

			write_to_file(LogFile, "\n**Checksums for Map Layers**\n");
			for (size_t ixL = 0; ixL < MapClass::ObjectsInLayers.c_size(); ++ixL) {
				write_to_file(LogFile, "Checksums for Layer {}\n", ixL);
				VectorLogger(&(MapClass::ObjectsInLayers[ixL]), LogFile);
			}

			write_to_file(LogFile, "\n**Checksums for Logics**\n");
			VectorLogger(&LogicClass::Instance(), LogFile);

			write_to_file(LogFile, "\n**Checksums for Abstracts**\n");
			VectorLogger(AbstractClass::Array(), LogFile, "Abstracts");
			VectorLogger(AbstractTypeClass::Array(), LogFile, "AbstractTypes");

			fclose(LogFile);
			return true;
		} else {
			Debug::FatalError("Failed to open file for sync log. Error code {}.\n", errno);
			return false;
		}
	}
};

ASMJIT_PATCH(0x647344, Multiplay_LogToSync, 0x5)
{
	GET(int, frame, ECX);

	char logFilename[0x40];
	if (Game::EnableMPSyncDebug())
	{
		sprintf_s(logFilename, std::size(logFilename) - 1, Debug::SyncFileFormat2.c_str(), HouseClass::CurrentPlayer->ArrayIndex, frame % 256);
	}
	else
	{
		sprintf_s(logFilename, std::size(logFilename) - 1, Debug::SyncFileFormat.c_str(), HouseClass::CurrentPlayer->ArrayIndex);
	}

	Logger::LogFrame(logFilename);
	SyncLogger::WriteSyncLog(logFilename);
	return 0x64736D;
}

ASMJIT_PATCH(0x64DEA0, Multiplay_LogToSYNC_NOMPDEBUG, 6)
{
	GET(EventClass*, OffendingEvent, ECX);

	char logFilename[0x40];
	sprintf_s(logFilename, std::size(logFilename) - 1, Debug::SyncFileFormat.c_str(), HouseClass::CurrentPlayer->ArrayIndex);
	Logger::LogFrame(logFilename, OffendingEvent);

	return 0x64DF3D;
}

ASMJIT_PATCH(0x6516F0, Multiplay_LogToSync_MPDEBUG, 6)
{
	GET(int, SlotNumber, ECX);
	GET(EventClass*, OffendingEvent, EDX);

	char logFilename[0x40];
	sprintf_s(logFilename, std::size(logFilename) - 1, Debug::SyncFileFormat2.c_str(), HouseClass::CurrentPlayer->ArrayIndex, SlotNumber);
	Logger::LogFrame(logFilename, OffendingEvent);

	return 0x651781;
}

#pragma warning(pop)
