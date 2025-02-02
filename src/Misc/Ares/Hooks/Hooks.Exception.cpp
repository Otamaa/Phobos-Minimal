
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

DEFINE_STRONG_HOOK(0x64CCBF, DoList_ReplaceReconMessage, 6)
{
	// mimic an increment because decrement happens in the middle of function cleanup and can't be erased nicely
	++Unsorted::SystemResponseMessages;

	Debug::Log("Reconnection error detected!\n");
	if (MessageBoxW(Game::hWnd, L"Yuri's Revenge has detected a desynchronization!\n"
		L"Would you like to create a full error report for the developers?\n"
		L"Be advised that reports from at least two players are needed.", L"Reconnection Error!", MB_YESNO | MB_ICONERROR) == IDYES)
	{
		Debug::DumpStack(R, 8084);

		HCURSOR loadCursor = LoadCursor(nullptr, IDC_WAIT);
		SetClassLong(Game::hWnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
		SetCursor(loadCursor);

		std::wstring path = Debug::PrepareSnapshotDirectory();

		if (Debug::LogEnabled)
		{
			Debug::Log("Copying debug log\n");
			const std::wstring logCopy = path + Debug::LogFileMainName + Debug::LogFileExt;
			CopyFileW(Debug::LogFileTempName.c_str(), logCopy.c_str(), FALSE);
		}

		Debug::Log("Making a memory snapshot\n");
		Debug::FullDump(std::move(path));

		loadCursor = LoadCursor(nullptr, IDC_ARROW);
		SetClassLong(Game::hWnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
		SetCursor(loadCursor);
		Debug::FatalError("A desynchronization has occurred.\r\n"
			"%s"
			"A crash dump should have been created in your game's \\debug subfolder.\r\n"
			"Please submit that to the developers along with SYNC*.txt, debug.txt and syringe.log."
				, Phobos::Otamaa::ParserErrorDetected ? "(One or more parser errors have been detected that might be responsible. Check the debug logs.)\r\n" : ""
		);
	}

	return 0x64CD11;
}

#pragma warning(push)
#pragma warning(disable: 4646) // this function does not return, though it isn't declared VOID
#pragma warning(disable: 4477)
#pragma warning(disable: 4715)
#define EXCEPTION_STACK_COLUMNS 8 // Number of columns in stack dump.
#define EXCEPTION_STACK_DEPTH_MAX 1024

LONG __fastcall ExceptionHandler(int code , PEXCEPTION_POINTERS const pExs) {

	DWORD* eip_pointer = reinterpret_cast<DWORD*>(&pExs->ContextRecord->Eip);

	switch (*eip_pointer)
	{
	case 0x7BC806: {
		*eip_pointer = 0x7BC80F;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	case 0x5D6C21: {
	// This bug most likely happens when a map Doesn't have Waypoint 90
		*eip_pointer = 0x5D6C36;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	case 0x7BAEA1: {
		// A common crash in DSurface::GetPixel
		*eip_pointer = 0x7BAEA8;
		pExs->ContextRecord->Ebx = 0;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	case 0x535DBC: {
		// Common crash in keyboard command class
		*eip_pointer = 0x535DCE;
		pExs->ContextRecord->Esp += 12;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	case 0x42C554:
	case 0x42C53E:
	case 0x42C507: {
		//FootClass* pFoot = (FootClass*)(ExceptionInfo->ContextRecord->Ebp + 0x14);
		//CellStruct* pFrom = (CellStruct*)(ExceptionInfo->ContextRecord->Ebp + 0x8);
		//CellStruct* pTo = (CellStruct*)(ExceptionInfo->ContextRecord->Ebp + 0xC);
		//MovementZone movementZone = (MovementZone)(ExceptionInfo->ContextRecord->Ebp + 0x10);

		//AstarClass , broken ptr
		Debug::Log("PathfindingCrash\n");
		break;
	}
	case 0x584DF7:
			Debug::Log("SubzoneTrackingCrash\n");
	break;
	//case 0x755C7F:
	//{
	//	Debug::Log("BounceAnimError \n");
	//	return PrintException(exception_id, ExceptionInfo);
	//}
	case 0x000000:
		if (pExs->ContextRecord->Esp && *(DWORD*)pExs->ContextRecord->Esp == 0x55E018) {
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
	Debug::Log("Exception handler fired!\n");
	Debug::Log("Exception %X at %p\n", pExs->ExceptionRecord->ExceptionCode, pExs->ExceptionRecord->ExceptionAddress);
	Game::StreamerThreadFlush();

	//the value of `reference<HWND> Game::hWnd` is stored on the stack instead of inlined as memory value, using `.get()` doesnot seems fixed it
	//so using these oogly
	SetWindowTextW(*reinterpret_cast<HWND*>(0xB73550), L"Fatal Error - Yuri's Revenge");

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
		std::wstring path = Debug::PrepareSnapshotDirectory();

		if (Debug::LogEnabled)
		{
			const std::wstring logCopy = path + Debug::LogFileMainName + Debug::LogFileExt;
			CopyFileW(Debug::LogFileTempName.c_str(), logCopy.c_str(), FALSE);
		}

		const std::wstring except_file = path + L"\\except.txt";

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
			for (auto const& data : Patch::ModuleDatas) {
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

			for (int d = 0; d < EXCEPTION_STACK_COLUMNS; ++d)
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

				if (!last_anim.empty()) {
					Debug::Log("LastAnim Calling CTOR (%s)\n", last_anim.c_str());
				}
			}

			{
				auto& pp = PhobosGlobal::Instance()->PathfindTechno;
				if (pp.IsValid()) {
					const char* pTechnoID = GameStrings::NoneStr();
					const char* what = GameStrings::NoneStr();
					if(pp.Finder) {
						const auto vtable = VTable::Get(pp.Finder);
						if(vtable == UnitClass::vtable) {
							pTechnoID = pp.Finder->get_ID();
							what = "UnitClass";
						} else if (vtable == InfantryClass::vtable) {
							what = "InfantryClass";
							pTechnoID = pp.Finder->get_ID();
						}
					}

					Debug::Log("LastPathfind (%x)[%s] - [%s] from (%d - %d) to (%d - %d)\n", pp.Finder , what , pTechnoID,
						pp.From.X , pp.From.Y ,
						pp.To.X , pp.To.Y
					);
				}
			}

			fprintf(except, "\nStack dump (depth : %d):\n", EXCEPTION_STACK_DEPTH_MAX);
			DWORD* ptr = reinterpret_cast<DWORD*>(pCtxt->Esp);
			for (int c = 0; c < EXCEPTION_STACK_DEPTH_MAX; ++c)
			{
				const char* suffix = "";
				if (*ptr >= 0x401000 && *ptr <= 0xB79BE4)
					suffix = "GameMemory!";
				else {
					for(auto begin = Patch::ModuleDatas.begin() + 1; begin != Patch::ModuleDatas.end(); ++begin) {
						if(*ptr >= begin->BaseAddr && *ptr <= (begin->BaseAddr + begin->Size)) {
							suffix = (begin->ModuleName +" Memory!").c_str();
							break;
						}
					}
				}

				fprintf(except, "%08p: %08X %s\n", ptr, *ptr, suffix);
				++ptr;
			}

			fclose(except);
			Debug::Log("Exception data has been saved to file:\n%ls\n", except_file.c_str());
		}

		//the value of `reference<HWND> Game::hWnd` is stored on the stack instead of inlined as memory value, using `.get()` doesnot seems fixed it
		//so using these oogly
		if (MessageBoxW(*reinterpret_cast<HWND*>(0xB73550), L"Yuri's Revenge has encountered a fatal error!\nWould you like to create a full crash report for the developers?", L"Fatal Error!", MB_YESNO | MB_ICONERROR) == IDYES)
		{
			HCURSOR loadCursor = LoadCursor(nullptr, IDC_WAIT);
			//the value of `reference<HWND> Game::hWnd` is stored on the stack instead of inlined as memory value, using `.get()` doesnot seems fixed it
			//so using these oogly
			SetClassLong(*reinterpret_cast<HWND*>(0xB73550), GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
			SetCursor(loadCursor);
			Debug::Log("Making a memory dump\n");

			MINIDUMP_EXCEPTION_INFORMATION expParam {};
			expParam.ThreadId = GetCurrentThreadId();
			expParam.ExceptionPointers = pExs;
			expParam.ClientPointers = FALSE;

			Debug::FullDump(std::move(path), &expParam);

			loadCursor = LoadCursor(nullptr, IDC_ARROW);
			//the value of `reference<HWND> Game::hWnd` is stored on the stack instead of inlined as memory value, using `.get()` doesnot seems fixed it
			//so using these oogly
			SetClassLong(*reinterpret_cast<HWND*>(0xB73550), GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
			SetCursor(loadCursor);
			Debug::FatalError("The cause of this error could not be determined.\r\n"
				"%s"
				"A crash dump should have been created in your game's \\debug subfolder.\r\n"
				"You can submit that to the developers (along with debug.txt and syringe.log)."
				, Phobos::Otamaa::ParserErrorDetected ? "(One or more parser errors have been detected that might be responsible. Check the debug logs.)\r\n" : ""
			);
		}
		break;
	}
	case ERROR_MOD_NOT_FOUND:
	case ERROR_PROC_NOT_FOUND:
		Debug::Log("Massive failure: Procedure or module not found!\n");
		break;
	default:
		Debug::Log("Massive failure: reason unknown, have fun figuring it out\n");
		Debug::DumpObj(reinterpret_cast<byte*>(pExs->ExceptionRecord), sizeof(*(pExs->ExceptionRecord)));
		//return EXCEPTION_CONTINUE_SEARCH;
		break;
	}

	Debug::Log("Exiting...\n");
	Debug::ExitGame(pExs->ExceptionRecord->ExceptionCode);
	return 0u;
};

DEFINE_JUMP(LJMP, 0x4C8FE0, MiscTools::to_DWORD(&ExceptionHandler))

//DEFINE_STRONG_HOOK(0x4C8FE0, Exception_Handler, 9)
//{
//	//GET(int, code, ECX);
//	GET(LPEXCEPTION_POINTERS, pExs, EDX);
//	if (!Phobos::Otamaa::ExeTerminated)
//	{
//		//dont fire exception multiple times ,..
//	   //i dont know how handle recursive exception
//		ExceptionHandler(pExs);
//		__debugbreak();
//	}
//}

#pragma warning(pop)
template<typename T>
void WriteLog(const T* it, int idx, DWORD checksum, FILE* F)
{
	fprintf(F, "#%05d:\t%08X", idx, checksum);
}

template<>
void WriteLog(const AbstractClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<void>(it, idx, checksum, F);
	auto abs = it->WhatAmI();
	fprintf(F, "; Abs: %u (%s)", abs, AbstractClass::GetAbstractClassName(abs));
}

template<>
void WriteLog(const ObjectClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<AbstractClass>(it, idx, checksum, F);

	const char* typeID = GameStrings::NoneStr();
	int typeIndex = -1;
	if (auto pType = it->GetType())
	{
		typeID = pType->ID;
		typeIndex = pType->GetArrayIndex();
	}

	CoordStruct crd = it->GetCoords();
	CellStruct cell = CellClass::Coord2Cell(crd);

	fprintf(F, "; Type: %d (%s); Coords: %d,%d,%d (%d,%d); Health: %d ; InLimbo: %u",
		typeIndex, typeID, crd.X, crd.Y, crd.Z, cell.X, cell.Y, it->Health, it->InLimbo);
}

template<>
void WriteLog(const MissionClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<ObjectClass>(it, idx, checksum, F);
	const auto Cur = it->GetCurrentMission();
	fprintf(F, "; Mission: %s (%d); StartTime: %d",
		MissionClass::MissionToString(Cur), Cur, it->CurrentMissionStartTime);
}

template<>
void WriteLog(const TechnoClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<MissionClass>(it, idx, checksum, F);

	const char* targetID = GameStrings::NoneStr();
	int targetIndex = -1;
	CoordStruct targetCrd = { -1, -1, -1 };
	if (auto pTarget = it->Target)
	{
		targetID = AbstractClass::GetAbstractClassName(pTarget->WhatAmI());
		targetIndex = pTarget->GetArrayIndex();
		targetCrd = pTarget->GetCoords();
	}

	fprintf(F, "; Facing: %d; Facing2: %d; Target: %s (%d; %d,%d)",
		it->PrimaryFacing.Current().Getvalue8(), it->SecondaryFacing.Current().Getvalue8(), targetID, targetIndex, targetCrd.X, targetCrd.Y);
}

template<>
void WriteLog(const FootClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<TechnoClass>(it, idx, checksum, F);

	const char* destID = GameStrings::NoneStr();
	int destIndex = -1;
	CoordStruct destCrd = { -1, -1, -1 };
	if (auto pDest = it->Destination)
	{
		destID = AbstractClass::GetAbstractClassName(pDest->WhatAmI());
		destIndex = pDest->GetArrayIndex();
		destCrd = pDest->GetCoords();
	}

	fprintf(F, "; Destination: %s (%d; %d,%d); SpeedPercentage %d ; Height %d",
		destID, destIndex, destCrd.X, destCrd.Y
		, int(it->SpeedPercentage * 256)
		, it->GetHeight()
	);
}

template<>
void WriteLog(const UnitClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<FootClass>(it, idx, checksum, F);

	const auto& Loco = it->Locomotor;
	auto accum = Loco->Get_Speed_Accum();
	auto index = Loco->Get_Track_Index();
	auto number = Loco->Get_Track_Number();

	fprintf(F, "; SpeedAccum %d; TrackNumber: %d; TrackIndex: %d", accum, number, index);
}

template<>
void WriteLog(const InfantryClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<FootClass>(it, idx, checksum, F);
}

template<>
void WriteLog(const AircraftClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<FootClass>(it, idx, checksum, F);
}

template<>
void WriteLog(const BuildingClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<TechnoClass>(it, idx, checksum, F);
}

template<>
void WriteLog(const AbstractTypeClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<AbstractClass>(it, idx, checksum, F);
	fprintf(F, "; ID: %s; Name: %s", it->ID, it->Name);
}

template<>
void WriteLog(const HouseClass* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog<void>(it, idx, checksum, F);

	fprintf(F, "; Player Name : %s (%d - %s); IsHumanPlayer: %u; ColorScheme: %s (%d); Edge: %d; StartingAllies: %u; Startspot: %d,%d; Visionary: %d; MapIsClear: %u; Money: %d",
		it->PlainName ? it->PlainName : GameStrings::NoneStr(),
		it->ArrayIndex, HouseTypeClass::Array->Items[it->Type->ArrayIndex]->Name,
		it->IsHumanPlayer, ColorScheme::Array->Items[it->ColorSchemeIndex]->ID, it->ColorSchemeIndex,
		(int)it->Edge, it->StartingAllies.data, it->StartingCell.X, it->StartingCell.Y, it->Visionary,
		it->MapIsClear, it->Available_Money());

	if (!it->IsNeutral() && !it->IsControlledByHuman())
	{
		fprintf(F, "\nLogging AI BaseNodes : \n");

		const auto& b = it->Base.BaseNodes;
		for (int j = 0; j < b.Count; ++j)
		{
			const auto& n = b[j];
			if (n.BuildingTypeIndex >= 0)
			{
				auto lbl = BuildingTypeClass::Array->Items[n.BuildingTypeIndex]->ID;
				fprintf(F, "\tNode #%03d: %s @ (%05d, %05d), Attempts so far: %d, Placed: %d\n"
					, j, lbl, n.MapCoords.X, n.MapCoords.Y, n.Attempts, n.Placed);
			}
			else
			{
				fprintf(F, "\tNode #%03d: Special %d @ (%05d, %05d), Attempts so far: %d, Placed: %d\n"
					, j, n.BuildingTypeIndex, n.MapCoords.X, n.MapCoords.Y, n.Attempts, n.Placed);
			}
		}
		fprintf(F, "\n");
	}

}

// calls WriteLog and appends a newline
template<typename T>
void WriteLogLine(const T* it, int idx, DWORD checksum, FILE* F)
{
	WriteLog(it, idx, checksum, F);
	fprintf(F, "\n");
}

template<typename T>
void LogItem(const T* it, int idx, FILE* F)
{
	if (it->WhatAmI() != AnimClass::AbsID || it->Fetch_ID() != -2)
	{
		DWORD Checksum(0);
		SafeChecksummer Ch;
		it->ComputeCRC(Ch);
		Checksum = Ch.CRC;
		WriteLogLine(it, idx, Checksum, F);
	}
}

template<typename T>
void VectorLogger(const DynamicVectorClass<T>* Array, FILE* F, const char* Label = nullptr)
{
	if (Label)
	{
		fprintf(F, "Checksums for [%s] (%d) :\n", Label, Array ? Array->Count : -1);
	}
	if (Array)
	{
		for (auto i = 0; i < Array->Count; ++i)
		{
			auto it = Array->Items[i];
			LogItem(it, i, F);
		}
	}
	else
	{
		fprintf(F, "Array not initialized yet...\n");
	}
}

template<typename T>
void HouseLogger(const DynamicVectorClass<T>* Array, FILE* F, const char* Label = nullptr)
{
	if (Array)
	{
		for (auto j = 0; j < HouseClass::Array->Count; ++j)
		{
			auto pHouse = HouseClass::Array->Items[j];
			fprintf(F, "-------------------- %s (%d) %s -------------------\n", pHouse->Type->Name, j, Label ? Label : "");

			for (auto i = 0; i < Array->Count; ++i)
			{
				auto it = Array->Items[i];

				if (it->Owner == pHouse)
				{
					LogItem(it, i, F);
				}
			}
		}
	}
	else
	{
		fprintf(F, "Array not initialized yet...\n");
	}
}

#include <Phobos.version.h>
static COMPILETIMEEVAL reference<DynamicVectorClass<ObjectClass*>*, 0x87F778u> const Logics {};

bool LogFrame(const char* LogFilename, EventClass* OffendingEvent = nullptr)
{
	FILE* LogFile = nullptr;
	if (!fopen_s(&LogFile, LogFilename, "wt") && LogFile)
	{
		std::setvbuf(LogFile, nullptr, _IOFBF, 1024 * 1024); // 1024 kb buffer - should be sufficient for whole log

		fprintf(LogFile, "YR synchronization log\n");
		fprintf(LogFile, "With Ares [21.352.1218] and Phobos %s\n", PRODUCT_VERSION);

		fprintf(LogFile, "\n");

		int i = 0;
		for (auto const& data : Patch::ModuleDatas)
		{
			fprintf(LogFile, "Module [(%d) %s: Base address = %08X]\n", i++, data.ModuleName.c_str(), data.BaseAddr);
		}

		fprintf(LogFile, "\n");

		for (size_t ixF = 0; ixF < EventClass::LatestFramesCRC.c_size(); ++ixF)
		{
			fprintf(LogFile, "LastFrame CRC[%02d] = %08X\n", ixF, EventClass::LatestFramesCRC[ixF]);
		}

		fprintf(LogFile, "My Random Number: %08X\n", ScenarioClass::Instance->Random.Random());
		fprintf(LogFile, "My Frame: %08X\n", Unsorted::CurrentFrame());
		fprintf(LogFile, "Average FPS: %d\n", int(FPSCounter::GetAverageFrameRate()));
		fprintf(LogFile, "Max MaxAhead: %d\n", Game::Network::MaxMaxAhead());
		fprintf(LogFile, "Latency setting: %d\n", Game::Network::LatencyFudge());
		fprintf(LogFile, "Game speed setting: %d\n", GameOptionsClass::Instance->GameSpeed);
		fprintf(LogFile, "FrameSendRate: %d\n", Game::Network::FrameSendRate());
		fprintf(LogFile, "Mod is %s (%s) with %X\n", AresGlobalData::ModName, AresGlobalData::ModVersion, AresGlobalData::ModIdentifier);

		if (HouseClass::CurrentPlayer())
			fprintf(LogFile, "Player Name: %s\n", HouseClass::CurrentPlayer->PlainName);

		const auto nHashes = HashData::GetINIChecksums();

		fprintf(LogFile, "Rules checksum: %08X\n", nHashes.Rules);
		fprintf(LogFile, "Art checksum: %08X\n", nHashes.Art);
		fprintf(LogFile, "AI checksum: %08X\n", nHashes.AI);

		if (OffendingEvent)
		{
			fprintf(LogFile, "\nOffending event:\n");
			fprintf(LogFile, "Type:         %X\n", OffendingEvent->Type);
			fprintf(LogFile, "Frame:        %X\n", OffendingEvent->Frame);
			fprintf(LogFile, "ID:           %X\n", OffendingEvent->HouseIndex);
			fprintf(LogFile, "CRC:          %X(%d)\n", OffendingEvent->Data.FrameInfo.CRC, OffendingEvent->Data.FrameInfo.CRC);
			fprintf(LogFile, "CommandCount: %hu\n", OffendingEvent->Data.FrameInfo.CommandCount);
			fprintf(LogFile, "Delay:        %hhu\n", OffendingEvent->Data.FrameInfo.Delay);
			fprintf(LogFile, "\n\n");
		}

		fprintf(LogFile, "\n**Types**\n");
		HouseLogger(InfantryClass::Array(), LogFile, "Infantry");
		HouseLogger(UnitClass::Array(), LogFile, "Units");
		HouseLogger(AircraftClass::Array(), LogFile, "Aircraft");
		HouseLogger(BuildingClass::Array(), LogFile, "Buildings");

		fprintf(LogFile, "\n**Checksums**\n");
		VectorLogger(HouseClass::Array(), LogFile, "Houses");
		VectorLogger(InfantryClass::Array(), LogFile, "Infantry");
		VectorLogger(UnitClass::Array(), LogFile, "Units");
		VectorLogger(AircraftClass::Array(), LogFile, "Aircraft");
		VectorLogger(BuildingClass::Array(), LogFile, "Buildings");

		fprintf(LogFile, "\n");
		VectorLogger(&ObjectClass::CurrentObjects(), LogFile, "Current Objects");
		VectorLogger(&LogicClass::Instance(), LogFile, "Logics");

		fprintf(LogFile, "\n**Checksums for Map Layers**\n");
		for (size_t ixL = 0; ixL < MapClass::ObjectsInLayers.c_size(); ++ixL)
		{
			fprintf(LogFile, "Checksums for Layer %d\n", ixL);
			VectorLogger(&(MapClass::ObjectsInLayers[ixL]), LogFile);
		}

		fprintf(LogFile, "\n**Checksums for Logics**\n");
		VectorLogger(&LogicClass::Instance(), LogFile);

		fprintf(LogFile, "\n**Checksums for Abstracts**\n");
		VectorLogger(AbstractClass::Array(), LogFile, "Abstracts");
		VectorLogger(AbstractTypeClass::Array(), LogFile, "AbstractTypes");

		fclose(LogFile);
		return true;
	}
	else
	{
		Debug::Log("Failed to open file for sync log. Error code %X.\n", errno);
		return false;
	}
}

DEFINE_STRONG_HOOK(0x64DEA0, Multiplay_LogToSYNC_NOMPDEBUG, 6)
{
	GET(EventClass*, OffendingEvent, ECX);

	char LogFilename[0x40];
	IMPL_SNPRNINTF(LogFilename, sizeof(LogFilename), GameStrings::SYNCNAME_TXT(), HouseClass::CurrentPlayer->ArrayIndex);

	LogFrame(LogFilename, OffendingEvent);
	SyncLogger::WriteSyncLog(LogFilename);

	return 0x64DF3D;
}

DEFINE_STRONG_HOOK(0x6516F0, Multiplay_LogToSync_MPDEBUG, 6)
{
	GET(int, SlotNumber, ECX);
	GET(EventClass*, OffendingEvent, EDX);

	char LogFilename[0x40];
	IMPL_SNPRNINTF(LogFilename, sizeof(LogFilename), GameStrings::SYNCNAME2_TXT(), HouseClass::CurrentPlayer->ArrayIndex, SlotNumber);

	LogFrame(LogFilename, OffendingEvent);
	SyncLogger::WriteSyncLog(LogFilename);

	return 0x651781;
}

