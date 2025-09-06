#pragma once

#include <CRT.h>
#include <ArrayClasses.h>
#include <GeneralDefinitions.h>

#include <Timers.h>

#include <Helpers/CompileTime.h>
#include <unknwn.h>

struct TacticalSelectableStruct;
class SideClass;
class HouseClass;
class WeaponTypeClass;
class BulletTypeClass;
class ObjectTypeClass;
class ObjectClass;
struct RectangleStruct;
struct CoordStruct;
class CCFileClass;
class CCINIClass;
class CellClass;
class BuildingClass;
class BuildingTypeClass;

struct PacketField {
	char str[4];
	char pad[4];
};

// things that I can't put into nice meaningful classes
struct Game
{
	static COMPILETIMEEVAL reference<PacketField, 0x841F40, 29u> const PacketFields {};
	static COMPILETIMEEVAL reference<CoordStruct, 0xB05488u> const CoordStruct_Empty {};

	// the magic checksum for version validation - linked in StaticInits
	static COMPILETIMEEVAL reference<DWORD, 0x83D560u> const Savegame_Magic {};
	static COMPILETIMEEVAL reference<double, 0xB0CD48u> const GameMagicValue_ {};

	static COMPILETIMEEVAL reference<DynamicVectorClass<ULONG>, 0xB0BC88u> const ClassFactories {};

	static COMPILETIMEEVAL reference<unsigned, 0x8B3A84u> const ExceptionReturnBase {};
	static COMPILETIMEEVAL reference<unsigned, 0x8B3A7Cu> const ExceptionReturnStack {};
	static COMPILETIMEEVAL reference<unsigned, 0x8B3A80u> const ExceptionReturnAddress {};
	static COMPILETIMEEVAL reference<HWND, 0xB73550u> const hWnd {};
	static COMPILETIMEEVAL reference<HINSTANCE, 0xB732F0u> const hInstance {};

	//"29e3bb2a-2f36-11d3-a72c-0090272fa661"
	static COMPILETIMEEVAL reference<HANDLE, 0xB0BCE4u> const Mutex {};

	//"01AF9993-3492-11d3-8F6F-0060089C05B1"
	static COMPILETIMEEVAL reference<HANDLE, 0xB0BCE8u> const AutoPlayMutex {};

	static COMPILETIMEEVAL reference<SystemTimerClass, 0x887348> const FrameTimer {};
	static COMPILETIMEEVAL reference<SystemTimerClass, 0x887328> const NFTTimer {};
	static COMPILETIMEEVAL reference<int, 0x8A00A8u> const ScreenHeight {};
	static COMPILETIMEEVAL reference<int, 0x8A00A4u> const ScreenWidth {};
	static COMPILETIMEEVAL reference<bool, 0xA8EDDCu> const SpeedControl {};
	static COMPILETIMEEVAL reference<unsigned short, 0x841F30u> const PlanetWestwoodPortNumber {};
	static COMPILETIMEEVAL reference<int, 0xB0BD90u> const SendStatistic_PacketSize {};
	static COMPILETIMEEVAL reference<bool, 0xA8F900u> const SendStatistic_Sended {};
	static COMPILETIMEEVAL reference<bool, 0xB0FBB8u> const ScoreStuffLoad {};
	static COMPILETIMEEVAL reference<bool, 0xA8D110u> const LANTaunts {};
	static COMPILETIMEEVAL reference<bool, 0xA8D111> const WOLTaunts {};
	static COMPILETIMEEVAL reference<bool, 0xA8D108u, 8u> const ChatMask {};

#define GAMEMD_CLSID(_addrs ,_name) \
	static COMPILETIMEEVAL reference<CLSID const, _addrs> const _name {};

	GAMEMD_CLSID(0x7E9520u, CLSID_);
	GAMEMD_CLSID(0x7E9540u, CLSID_CompressStream);
	GAMEMD_CLSID(0x7E9720u, CLSID_WaveClass);
	GAMEMD_CLSID(0x7E95E0u, CLSID_TerrainTypeClass);
	GAMEMD_CLSID(0x7E9740u, CLSID_TerrainClass);
	GAMEMD_CLSID(0x7E9570u, CLSID_SuperWeaponTypeClass);
	GAMEMD_CLSID(0x7E9580u, CLSID_SuperWeaponClass);
	GAMEMD_CLSID(0x7E9950u, CLSID_TacticalMapClass);
	GAMEMD_CLSID(0x7E9930u, CLSID_CellClass);
	GAMEMD_CLSID(0x7E9940u, CLSID_EMPulseClass);
	GAMEMD_CLSID(0x7E98F0u, CLSID_LightSource);
	GAMEMD_CLSID(0x7E9910u, CLSID_SideClass);
	GAMEMD_CLSID(0x7E9920u, CLSID_TiberiumClass);
	GAMEMD_CLSID(0x7E9750u, CLSID_TubeClass);
	GAMEMD_CLSID(0x7E9900u, CLSID_CampaignClass);
	GAMEMD_CLSID(0x7E9730u, CLSID_BuildingLightClass);
	GAMEMD_CLSID(0x7E9850u, CLSID_WaypointPath);
	GAMEMD_CLSID(0x7E98D0u, CLSID_TemporalClass);
	//etc,..

	GAMEMD_CLSID(0x7F7D50u, CLSID_IDirectDraw2);
	GAMEMD_CLSID(0x7F8070u, IID_IDirect3D2);

#undef GAMEMD_CLSID
	static COMPILETIMEEVAL reference<Matrix3D, 0xB44318> VoxelDefaultMatrix {};
	static COMPILETIMEEVAL reference<Matrix3D, 0xB45188, 21> VoxelRampMatrix {};
	static COMPILETIMEEVAL reference<double, 0xB1D008> const Pixel_Per_Lepton {};

	static COMPILETIMEEVAL reference<bool, 0x887418u> const bVPLRead {};
	static COMPILETIMEEVAL reference<bool, 0x840A6Cu> const bVideoBackBuffer {};
	static COMPILETIMEEVAL reference<bool, 0xA8EB96u> const bAllowVRAMSidebar {};

	static COMPILETIMEEVAL reference<RecordFlag, 0xA8D5F8u> const RecordingFlag {};
	static COMPILETIMEEVAL reference<CCFileClass, 0xA8D58Cu> const RecordFile {};
	//static COMPILETIMEEVAL reference<DynamicVectorClass<DWORD>, 0xB0BC88u> const COMClasses {};

	static COMPILETIMEEVAL reference<bool, 0xA8EDD8u> const ReadyToExit {};
	static COMPILETIMEEVAL reference<bool, 0x822CF1u> const bDrawShadow {};
	static COMPILETIMEEVAL reference<bool, 0x8A0DEFu> const bAllowDirect3D {};
	static COMPILETIMEEVAL reference<bool, 0x8A0DF0u> const bDirect3DIsUseable {};

	static COMPILETIMEEVAL reference<bool, 0xA8E9A0u> const IsActive {};
	static COMPILETIMEEVAL reference<bool, 0xA8ED80u> const IsFocused {};
	static COMPILETIMEEVAL reference<int, 0xA8EDA0u> const SpecialDialog {};
	static COMPILETIMEEVAL reference<bool, 0xAC48D4> const PCXInitialized {};

	static COMPILETIMEEVAL reference<int, 0xA8ED94u> const Seed {};
	static COMPILETIMEEVAL reference<int, 0x822CF4u> const TechLevel {};
	static COMPILETIMEEVAL reference<int, 0xA8B54Cu> const PlayerCount {};
	static COMPILETIMEEVAL reference<int, 0xA8B394u> const PlayerColor {};
	static COMPILETIMEEVAL reference<bool, 0xAC10C8u> const ObserverMode {};
	static COMPILETIMEEVAL reference<char, 0xA8B8E0u> const ScenarioName {};

	static COMPILETIMEEVAL reference<int, 0xB0BD90u> const PacketSize {};
	static COMPILETIMEEVAL reference<bool, 0xA8F900u> const StaticPacketSent {};

	static COMPILETIMEEVAL reference<bool, 0xA8E378u> const InScenario1 {};
	static COMPILETIMEEVAL reference<bool, 0xA8ED5Cu> const InScenario2 {};

	static COMPILETIMEEVAL reference<CoordStruct, 0x89C870u> const RelativeCoordCenter {};

	static COMPILETIMEEVAL reference<wchar_t, 0xB730ECu, 256u> const IMEBuffer {};
	static COMPILETIMEEVAL reference<HIMC, 0xB7355Cu> const IMEContext {};
	static COMPILETIMEEVAL reference<wchar_t, 0xB73318u, 257u> const IMECompositionString {};

	static COMPILETIMEEVAL reference<bool, 0xA8F7ACu> const DontSetExceptionHandler{};
	static COMPILETIMEEVAL reference<bool, 0xA8B8B4u> const EnableMPDebug{};
	static COMPILETIMEEVAL reference<bool, 0xA8B8B5u> const DrawMPDebugStats{};
	static COMPILETIMEEVAL reference<bool, 0xB04880u> const EnableMPSyncDebug{};

	static COMPILETIMEEVAL reference<Vector3D<float>, 0x887470> const VoxelLightSource { };
	static COMPILETIMEEVAL reference<Vector3D<float>, 0x887420> const VoxelShadowLightSource { };

	struct Network
	{

		static COMPILETIMEEVAL reference<u_short, 0x841F30u> const ListenPort {};
		static COMPILETIMEEVAL reference<int, 0xB779C4u> const Tournament {};
		static COMPILETIMEEVAL reference<DWORD, 0xB779D4u> const WOLGameID {};
		static COMPILETIMEEVAL reference<time_t, 0xB77788u> const PlanetWestwoodStartTime {};
		static COMPILETIMEEVAL reference<int, 0xB73814u> const GameStockKeepingUnit {};
		static COMPILETIMEEVAL reference<int, 0xA8B24Cu> const ProtocolVersion {};
		static COMPILETIMEEVAL reference<int, 0xA8B554u> const FrameSendRate {};
		static COMPILETIMEEVAL reference<int, 0xA8B570u> const PreCalcFrameRate {};
		static COMPILETIMEEVAL reference<int, 0x83737Cu> const ReconnectTimeout {};
		static COMPILETIMEEVAL reference<int, 0xA8B550u> const MaxAhead {};
		static COMPILETIMEEVAL reference<int, 0xA8B56Cu> const PreCalcMaxAhead {};
		static COMPILETIMEEVAL reference<int, 0xA8B568u> const MaxMaxAhead {};
		static COMPILETIMEEVAL reference<int, 0xA8DB9Cu> const LatencyFudge {};
		static COMPILETIMEEVAL reference<int, 0xA8B558u> const RequestedFPS {};

		static bool __fastcall Init()
		{ JMP_FAST(0x5DA6C0); }
	};

	static void __fastcall GetKeyboardKeyString(unsigned short key, wchar_t* buffer)
		{ JMP_FAST(0x61EF70); }

	static bool __fastcall File_Finder_Start(char* filename) {
		JMP_FAST(0x47AF70);
	}

	static BuildingTypeClass* __fastcall Set_Sidebar_Tab_Object(BuildingClass* pThis) {
		JMP_FAST(0x734250);
	}

	static bool __fastcall File_Finder_Next_Name(char* filename) {
		JMP_FAST(0x47B0C0);
	}

	static void __cdecl File_Finder_End() {
		JMP_STD(0x47B130);
	}

	static bool __cdecl MainLoop()
		{ JMP_STD(0x55D360); }

	static int __fastcall Get_Volume_Index(int timeout) {
		JMP_FAST(0x4790A0);
	}

	static void __fastcall DestroyVoxelCaches() {
		JMP_FAST(0x755C50);
	}

	static bool __fastcall Prep_For_Side(int house) {
		JMP_FAST(0x534FA0);
	}

	static RectangleStruct* __fastcall Get_Sidebar_Clip_Bounds(RectangleStruct* ree) {
		JMP_FAST(0x72AD20);
	}

	static HRESULT __fastcall Save_Misc_Values(LPSTREAM a1) {
		JMP_FAST(0x67F7E0);
	}
	static HRESULT __fastcall Load_Misc_Values(LPSTREAM a1) {
		JMP_FAST(0x67F9C0);
	}
#ifdef deprecated

	// the game's own rounding function
	// infamous for true'ing (F2I(-5.00) == -4.00)
	static int F2I(double val)
	{
		return int(F2I64(val));
	}

	// the game's own rounding function
	// infamous for true'ing (F2I(-5.00) == -4.00)
	static uint64_t F2I64(double val)
	{
		double something = val;
		__asm { fld something };
		ASM_CALL(0x7C5F00);
	}

#endif // deprecated

	static COMPILETIMEEVAL int FORCEDINLINE AdjustHeight(int height)  {
		return int((double)height * Unsorted::GameMagicNumbr_ + ((double)(height >= Unsorted::HeightMax)) + 0.5);
	}

	[[noreturn]] static void __stdcall RaiseError(HRESULT err)
	{
		JMP_STD(0x7DC720);
	}

	static void ClearScenario() {
		ASM_CALL(0x6851F0);
	}

	// // actually is SessionClass::Callback
	// static void SetProgress(int progress)
	// { SET_REG32(ECX, 0xA8B238); JMP_STD(0x69AE90); }

	static void __cdecl CallBack()
	{ JMP(0x48D080); }

	static int __fastcall GetResource(int ID, int Type)
	{ JMP_FAST(0x4A3B40); }

	static void __fastcall CenterWindowIn(HWND Child, HWND Parent)
	{ JMP_FAST(0x777080); }

	static void __fastcall sub_53E420(HWND hWindow)
	{ JMP_FAST(0x53E420); }

	static void __fastcall sub_53E3C0(HWND hWindow)
	{ JMP_FAST(0x53E3C0); }

	static void __stdcall OnWindowMoving(tagRECT* Rect)
	{ JMP_FAST(0x776D80); }

	static void __stdcall PlanningManager_WM_RBUTTONUP_63AB00(Point2D XY)
	{ JMP_FAST(0x63AB00); }

	static HRESULT __fastcall Save_Sides(LPSTREAM pStm, DynamicVectorClass<SideClass*>* pVector)
	{ JMP_FAST(0x6805F0); }

	static void __fastcall StreamerThreadFlush()
	{ JMP_FAST(0x53E6B0); }

	static void __fastcall UICommands_TypeSelect_7327D0(const char* iniName)
	{ JMP_FAST(0x7327D0); }

	static bool __fastcall IsTypeSelecting()
	{ JMP_FAST(0x732D00); }

	static double __cdecl GetFloaterGravity()
	{ JMP(0x48ACF0); }

	static void __fastcall KeyboardProcess(DWORD& input)
	{ JMP_FAST(0x55DEE0); }

	static LARGE_INTEGER __fastcall AudioGetTime()
	{ JMP_FAST(0x4093B0); }

	static void InitRandom()
	{ ASM_CALL(0x52FC20); }

	static void ShowSpecialDialog()
	{ ASM_CALL(0x48C8B0); }

	static bool __fastcall InitNetwork()
	{ JMP_FAST(0x5DA6C0); }

	static void InitUIStuff()
	{
		/* InitCommonDialogStuff() */ ASM_CALL(0x600560);

		if (!PCXInitialized)
		{
			/* InitUIColorShifts() */ ASM_CALL(0x61F190);
			/* LoadPCXFiles() */      ASM_CALL(0x61F210);
			PCXInitialized = true;
		}
	}

	static void __fastcall PlayMovie(
		const char* movieName,
		int queue_theme = -1,
		char use_hidden_surface1 = -1,
		char stretch_movie = -1,
		char use_hidden_surface2 = -1,
		char set_state_1 = -1
	)
	{
		JMP_FAST(0x5BED40);
	}

	static AbstractType __fastcall WhichTab(AbstractType Type, int heapId, int a3 = 0)
	{ JMP_FAST(0x6ABC60); }

	// convert xyz height to xy height?
	//static int __fastcall AdjustForZ(int Height) //ZDepth_Adjust_For_Height
	//{ JMP_FAST(0x6D20E0); }

	static void __fastcall WriteMapFiles(const char* pFilename, int bArgs = false)
	{ JMP_FAST(0x687CE0); }

	static CoordStruct* __fastcall Coord_Move(CoordStruct* revtal, CoordStruct* start, DirStruct* dir, int distance) {
		JMP_FAST(0x50E3A0);
	}

	static int __fastcall Spot_Index(CoordStruct* coord) {
		JMP_FAST(0x4810A0);
	}

	static bool __fastcall func_007BBE20(RectangleStruct* torect, const RectangleStruct* toarea, RectangleStruct* fromrect, const RectangleStruct* fromarea)
	{ JMP_FAST(0x7BBE20); }

	static int __fastcall Point2DToDir8(Point2D* pFrom, Point2D* pTo)
	{ JMP_FAST(0x75F230); }

	static AbilityType __fastcall GetAbility(const char* pString)
	{ JMP_FAST(0x74FEF0); }

	static int __fastcall CellStructToIdx(CellStruct* pCell)
	{ JMP_FAST(0x42B1C0); }

	//static int __fastcall ZDepthAdjust(int nZ)
	//{ JMP_FAST(0x6D20E0); }

	static bool __fastcall Clip_Line(Point2D* point1, Point2D* point2, const RectangleStruct* rect)
	{ JMP_FAST(0x7BC2B0) }

	static void Unselect_All_Except(AbstractType rtti);
	static void Unselect_All_Except(ObjectTypeClass* objecttype);

	static void Unselect_All_Except(ObjectClass* object);

	static void __fastcall Compute_All_CRC()
	{
		JMP_FAST(0x64DAB0);
	}

	static bool __fastcall WillItReachTargetInTime(int speed, int distance, int heignt, double gravity)
	{
		JMP_FAST(0x48ABC0);
	}

	static CellClass* __fastcall GetCellOfFirstObstacle(CoordStruct* start, CoordStruct* end, BulletTypeClass* bullet, HouseClass* house)
	{
		JMP_FAST(0x4CC100);
	}

	static CellClass* __fastcall Get_Cell_Of_First_Obstacle_0(CoordStruct* start, CoordStruct* end, WeaponTypeClass* weapon, HouseClass* house)
	{
		JMP_FAST(0x4CC310);
	}

	static void __fastcall Init_Voxel_Light(float theta) { JMP_FAST(0x754C00); }
	static void __fastcall Init_Voxel_Projections() { JMP_FAST(0x754CB0); }

	static Matrix3D* __fastcall GetRampMtx(Matrix3D* pRet, int nRampIdx)
	{
		JMP_FAST(0x7559B0);
	}

	// fastcall 48AB90
	static int __fastcall AdjustRangeWithGravity(int nRange, double Gravity)
	{
		JMP_FAST(0x48AB90);
	}

	// fastcall 48A8D0
	static bool __fastcall func_48A8D0_Legal(bool InRange, int x , int y , int z , double something, DirStruct* pDir)
	{
		JMP_FAST(0x48A8D0);
	}

	static void __fastcall ComputeFrameCRC()
	{ JMP_FAST(0x64DAB0); }

	static void __fastcall LogFrameCRC(int frameIndex)
	{ JMP_FAST(0x650A90); }
};

// this fake class contains the IIDs used by the game
// no comments because the IIDs suck
struct IIDs
{
	static COMPILETIMEEVAL reference<IID const, 0x7F7C90u> const IUnknown {};
	static COMPILETIMEEVAL reference<IID const, 0x7F7C80u> const IPersistStream {};
	static COMPILETIMEEVAL reference<IID const, 0x7F7C70u> const IPersist {};
	static COMPILETIMEEVAL reference<IID const, 0x7E9AE0u> const IRTTITypeInfo {};
	static COMPILETIMEEVAL reference<IID const, 0x7EA768u> const IHouse {};
	static COMPILETIMEEVAL reference<IID const, 0x7E9B00u> const IPublicHouse {};
	static COMPILETIMEEVAL reference<IID const, 0x7F7CB0u> const IEnumConnections {};
	static COMPILETIMEEVAL reference<IID const, 0x7F7CC0u> const IConnectionPoint {};
	static COMPILETIMEEVAL reference<IID const, 0x7F7CD0u> const IConnectionPointContainer {};
	static COMPILETIMEEVAL reference<IID const, 0x7F7CE0u> const IEnumConnectionPoints {};
	static COMPILETIMEEVAL reference<IID const, 0x7E36C0u> const IApplication {};
	static COMPILETIMEEVAL reference<IID const, 0x7EA6E8u> const IGameMap {};
	static COMPILETIMEEVAL reference<IID const, 0x7ED358u> const ILocomotion {};
	static COMPILETIMEEVAL reference<IID const, 0x7E9B10u> const IPiggyback {};
	static COMPILETIMEEVAL reference<IID const, 0x7E9B40u> const IFlyControl {};
	static COMPILETIMEEVAL reference<IID const, 0x7E9B20u> const ISwizzle {};
};

// this class links to functions gamemd imports
// to avoid having to link to their DLLs ourselves
struct Imports
{

	// OleLoadFromStream
	typedef HRESULT(__stdcall* FP_OleSaveToStream)(LPPERSISTSTREAM pPStm, LPSTREAM pStm);
	static COMPILETIMEEVAL referencefunc<FP_OleSaveToStream, 0x7E15F4> const OleSaveToStream {};

	typedef HRESULT(__stdcall* FP_OleLoadFromStream)(LPSTREAM pStm, const IID* const iidInterface, LPVOID* ppvObj);
	static COMPILETIMEEVAL referencefunc<FP_OleLoadFromStream, 0x7E15F8> const OleLoadFromStream {};

	typedef HRESULT(__stdcall* FP_CoRegisterClassObject)(const IID& rclsid, LPUNKNOWN pUnk, DWORD dwClsContext, DWORD flags, LPDWORD lpdwRegister);
	static COMPILETIMEEVAL referencefunc<FP_CoRegisterClassObject, 0x7E15D8> const CoRegisterClassObject {};

	typedef HRESULT(__stdcall* FP_CoRevokeClassObject)(DWORD dwRegister);
	static COMPILETIMEEVAL referencefunc<FP_CoRevokeClassObject, 0x7E15CC> const CoRevokeClassObject {};

	typedef HRESULT(__stdcall* FP_OleCoCreateInstance)(const IID& rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, const IID& riid, LPVOID* ppv);
	static COMPILETIMEEVAL referencefunc<FP_OleCoCreateInstance, 0x7E15FC > const CoCreateInstance {};

	typedef HRESULT(__stdcall* FP_OleRun)(LPUNKNOWN pUnknown);
	static COMPILETIMEEVAL referencefunc<FP_OleRun, 0x7E1600 > const OleRun {};

	typedef DWORD(*FP_TimeGetTime)();
	static COMPILETIMEEVAL referencefunc<FP_TimeGetTime, 0x7E1530> const TimeGetTime {};

	/* user32.dll */
	typedef LRESULT(__stdcall* FP_DefWindowProcA)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static COMPILETIMEEVAL referencefunc<FP_DefWindowProcA, 0x7E1394> const DefWindowProcA {};

	typedef BOOL(__stdcall* FP_MoveWindow)(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
	static COMPILETIMEEVAL referencefunc<FP_MoveWindow, 0x7E1398> const MoveWindow {};

	typedef BOOL(__stdcall* FP_GetUpdateRect)(HWND hWnd, LPRECT lpRect, BOOL bErase);
	static COMPILETIMEEVAL referencefunc<FP_GetUpdateRect, 0x7E139C> const GetUpdateRect {};

	typedef HWND(*FP_GetFocus)(void);
	static COMPILETIMEEVAL referencefunc<FP_GetFocus, 0x7E13A0> const GetFocus {};

	typedef HDC(__stdcall* FP_GetDC)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_GetDC, 0x7E13A4> const GetDC {};

	typedef SHORT(__stdcall* FP_GetKeyState)(int nVirtKey);
	static COMPILETIMEEVAL referencefunc<FP_GetKeyState, 0x7E13A8> const GetKeyState {};

	typedef HWND(*FP_GetActiveWindow)(void);
	static COMPILETIMEEVAL referencefunc<FP_GetActiveWindow, 0x7E13AC> const GetActiveWindow {};

	typedef HWND(*FP_GetCapture)(void);
	static COMPILETIMEEVAL referencefunc<FP_GetCapture, 0x7E13B0> const GetCapture {};

	typedef int(__stdcall* FP_GetDlgCtrlID)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_GetDlgCtrlID, 0x7E13B4> const GetDlgCtrlID {};

	typedef HWND(__stdcall* FP_ChildWindowFromPointEx)(HWND, POINT, UINT);
	static COMPILETIMEEVAL referencefunc<FP_ChildWindowFromPointEx, 0x7E13B8> const ChildWindowFromPointEx {};

	typedef BOOL(__stdcall* FP_GetWindowRect)(HWND hWnd, LPRECT lpRect);
	static COMPILETIMEEVAL referencefunc<FP_GetWindowRect, 0x7E13BC> const GetWindowRect {};

	typedef BOOL(__stdcall* FP_GetCursorPos)(LPPOINT lpPoint);
	static COMPILETIMEEVAL referencefunc<FP_GetCursorPos, 0x7E13C0> const GetCursorPos {};

	typedef BOOL(__stdcall* FP_CloseWindow)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_CloseWindow, 0x7E13C4> const CloseWindow {};

	typedef BOOL(__stdcall* FP_EndDialog)(HWND hDlg, int nResult);
	static COMPILETIMEEVAL referencefunc<FP_EndDialog, 0x7E13C8> const EndDialog {};

	typedef HWND(__stdcall* FP_SetFocus)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_SetFocus, 0x7E13CC> const SetArchiveTarget {};

	typedef BOOL(__stdcall* FP_SetDlgItemTextA)(HWND hDlg, int nIDDlgItem, LPCSTR lpString);
	static COMPILETIMEEVAL referencefunc<FP_SetDlgItemTextA, 0x7E13D0> const SetDlgItemTextA {};

	typedef int(__stdcall* FP_DialogBoxParamA)(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
	static COMPILETIMEEVAL referencefunc<FP_DialogBoxParamA, 0x7E13D4> const DialogBoxParamA {};

	typedef int(__stdcall* FP_DialogBoxIndirectParamA)(HINSTANCE hInstance, LPCDLGTEMPLATEA hDialogTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
	static COMPILETIMEEVAL referencefunc<FP_DialogBoxIndirectParamA, 0x7E13D8> const DialogBoxIndirectParamA {};

	typedef int(__stdcall* FP_ShowCursor)(BOOL bShow);
	static COMPILETIMEEVAL referencefunc<FP_ShowCursor, 0x7E13DC> const ShowCursor {};

	typedef SHORT(__stdcall* FP_GetAsyncKeyState)(int vKey);
	static COMPILETIMEEVAL referencefunc<FP_GetAsyncKeyState, 0x7E13E0> const GetAsyncKeyState {};

	typedef int(__stdcall* FP_ToAscii)(UINT uVirtKey, UINT uScanCode, PBYTE lpKeyState, LPWORD lpChar, UINT uFlags);
	static COMPILETIMEEVAL referencefunc<FP_ToAscii, 0x7E13E4> const ToAscii {};

	typedef UINT(__stdcall* FP_MapVirtualKeyA)(UINT uCode, UINT uMapType);
	static COMPILETIMEEVAL referencefunc<FP_MapVirtualKeyA, 0x7E13E8> const MapVirtualKeyA {};

	typedef int(__stdcall* FP_GetSystemMetrics)(int nIndex);
	static COMPILETIMEEVAL referencefunc<FP_GetSystemMetrics, 0x7E13EC> const GetSystemMetrics {};

	typedef BOOL(__stdcall* FP_SetWindowPos)(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
	static COMPILETIMEEVAL referencefunc<FP_SetWindowPos, 0x7E13F0> const SetWindowPos {};

	typedef BOOL(__stdcall* FP_DestroyWindow)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_DestroyWindow, 0x7E13F4> const DestroyWindow {};

	typedef BOOL(*FP_ReleaseCapture)(void);
	static COMPILETIMEEVAL referencefunc<FP_ReleaseCapture, 0x7E13F8> const ReleaseCapture {};

	typedef HWND(__stdcall* FP_SetCapture)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_SetCapture, 0x7E13FC> const SetCapture {};

	typedef BOOL(__stdcall* FP_AdjustWindowRectEx)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle);
	static COMPILETIMEEVAL referencefunc<FP_AdjustWindowRectEx, 0x7E1400> const AdjustWindowRectEx {};

	typedef HMENU(__stdcall* FP_GetMenu)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_GetMenu, 0x7E1404> const GetMenu {};

	typedef BOOL(__stdcall* FP_AdjustWindowRect)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu);
	static COMPILETIMEEVAL referencefunc<FP_AdjustWindowRect, 0x7E1408> const AdjustWindowRect {};

	typedef DWORD(__stdcall* FP_GetSysColor)(int nIndex);
	static COMPILETIMEEVAL referencefunc<FP_GetSysColor, 0x7E140C> const GetSysColor {};

	typedef UINT(__stdcall* FP_IsDlgButtonChecked)(HWND hDlg, int nIDButton);
	static COMPILETIMEEVAL referencefunc<FP_IsDlgButtonChecked, 0x7E1410> const IsDlgButtonChecked {};

	typedef BOOL(__stdcall* FP_CheckDlgButton)(HWND hDlg, int nIDButton, UINT uCheck);
	static COMPILETIMEEVAL referencefunc<FP_CheckDlgButton, 0x7E1414 > const CheckDlgButton {};

	typedef DWORD(__stdcall* FP_WaitForInputIdle)(HANDLE hProcess, DWORD dwMilliseconds);
	static COMPILETIMEEVAL referencefunc<FP_WaitForInputIdle, 0x7E1418 > const WaitForInputIdle {};

	typedef HWND(__stdcall* FP_GetTopWindow)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_GetTopWindow, 0x7E141C > const GetTopWindow {};

	typedef HWND(*FP_GetForegroundWindow)(void);
	static COMPILETIMEEVAL referencefunc<FP_GetForegroundWindow, 0x7E1420 > const GetForegroundWindow {};

	typedef HICON(__stdcall* FP_LoadIconA)(HINSTANCE hInstance, LPCSTR lpIconName);
	static COMPILETIMEEVAL referencefunc<FP_LoadIconA, 0x7E1424 > const LoadIconA {};

	typedef HWND(__stdcall* FP_SetActiveWindow)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_SetActiveWindow, 0x7E1428> const SetActiveWindow {};

	typedef BOOL(__stdcall* FP_RedrawWindow)(HWND hWnd, const RECT* lprcUpdate, HRGN hrgnUpdate, UINT flags);
	static COMPILETIMEEVAL referencefunc<FP_RedrawWindow, 0x7E142C> const RedrawWindow {};

	typedef DWORD(__stdcall* FP_GetWindowContextHelpId)(HWND);
	static COMPILETIMEEVAL referencefunc<FP_GetWindowContextHelpId, 0x7E1430  > const GetWindowContextHelpId {};

	typedef BOOL(__stdcall* FP_WinHelpA)(HWND hWndMain, LPCSTR lpszHelp, UINT uCommand, DWORD dwData);
	static COMPILETIMEEVAL referencefunc<FP_WinHelpA, 0x7E1434> const WinHelpA {};

	typedef HWND(__stdcall* FP_ChildWindowFromPoint)(HWND hWndParent, POINT Point);
	static COMPILETIMEEVAL referencefunc<FP_ChildWindowFromPoint, 0x7E1438 > const ChildWindowFromPoint {};

	typedef HCURSOR(__stdcall* FP_LoadCursorA)(HINSTANCE hInstance, LPCSTR lpCursorName);
	static COMPILETIMEEVAL referencefunc<FP_LoadCursorA, 0x7E143C > const LoadCursorA {};

	typedef HCURSOR(__stdcall* FP_SetCursor)(HCURSOR hCursor);
	static COMPILETIMEEVAL referencefunc<FP_SetCursor, 0x7E1440 > const SetCursor {};

	typedef void(__stdcall* FP_PostQuitMessage)(int nExitCode);
	static COMPILETIMEEVAL referencefunc<FP_PostQuitMessage, 0x7E1444 > const PostQuitMessage {};

	typedef HWND(__stdcall* FP_FindWindowA)(LPCSTR lpClassName, LPCSTR lpWindowName);
	static COMPILETIMEEVAL referencefunc<FP_FindWindowA, 0x7E1448 > const FindWindowA {};

	typedef BOOL(__stdcall* FP_SetCursorPos)(int X, int Y);
	static COMPILETIMEEVAL referencefunc<FP_SetCursorPos, 0x7E144C> const SetCursorPos {};

	typedef HWND(__stdcall* FP_CreateDialogIndirectParamA)(HINSTANCE hInstance, LPCDLGTEMPLATEA lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
	static COMPILETIMEEVAL referencefunc<FP_CreateDialogIndirectParamA, 0x7E1450> const CreateDialogIndirectParamA {};

	typedef int(__stdcall* FP_GetKeyNameTextA)(LONG lParam, LPSTR lpString, int nSize);
	static COMPILETIMEEVAL referencefunc<FP_GetKeyNameTextA, 0x7E1454 > const GetKeyNameTextA {};

	typedef BOOL(__stdcall* FP_ScreenToClient)(HWND hWnd, LPPOINT lpPoint);
	static COMPILETIMEEVAL referencefunc<FP_ScreenToClient, 0x7E1458 > const ScreenToClient {};

	typedef BOOL(__stdcall* FP_LockWindowUpdate)(HWND hWndLock);
	static COMPILETIMEEVAL referencefunc<FP_LockWindowUpdate, 0x7E145C> const LockWindowUpdate {};

	typedef int(__stdcall* FP_MessageBoxA)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
	static COMPILETIMEEVAL referencefunc<FP_MessageBoxA, 0x7E1460> const MessageBoxA {};

	typedef int(__stdcall* FP_ReleaseDC)(HWND hWnd, HDC hDC);
	static COMPILETIMEEVAL referencefunc<FP_ReleaseDC, 0x7E1464> const ReleaseDC {};

	typedef HWND(__stdcall* FP_WindowFromPoint)(POINT Point);
	static COMPILETIMEEVAL referencefunc<FP_WindowFromPoint, 0x7E1468> const WindowFromPoint {};

	typedef BOOL(__stdcall* FP_UpdateWindow)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_UpdateWindow, 0x7E146C > const UpdateWindow {};

	typedef LONG(__stdcall* FP_SetWindowLongA)(HWND hWnd, int nIndex, LONG dwNewLong);
	static COMPILETIMEEVAL referencefunc<FP_SetWindowLongA, 0x7E1470 > const SetWindowLongA {};

	typedef LONG(__stdcall* FP_GetWindowLongA)(HWND hWnd, int nIndex);
	static COMPILETIMEEVAL referencefunc<FP_GetWindowLongA, 0x7E1474> const GetWindowLongA {};

	typedef BOOL(__stdcall* FP_ValidateRect)(HWND hWnd, const RECT* lpRect);
	static COMPILETIMEEVAL referencefunc<FP_ValidateRect, 0x7E1478 > const ValidateRect {};

	typedef BOOL(__stdcall* FP_IntersectRect)(LPRECT lprcDst, const RECT* lprcSrc1, const RECT* lprcSrc2);
	static COMPILETIMEEVAL referencefunc<FP_IntersectRect, 0x7E147C > const IntersectRect {};

	typedef int(__stdcall* FP_MessageBoxIndirectA)(LPMSGBOXPARAMSA);
	static COMPILETIMEEVAL referencefunc<FP_MessageBoxIndirectA, 0x7E1480 > const MessageBoxIndirectA {};

	typedef BOOL(__stdcall* FP_PeekMessageA)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
	static COMPILETIMEEVAL referencefunc<FP_PeekMessageA, 0x7E1484> const PeekMessageA {};

	typedef LRESULT(__stdcall* FP_CallWindowProcA)(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static COMPILETIMEEVAL referencefunc<FP_CallWindowProcA, 0x7E1488> const CallWindowProcA {};

	typedef BOOL(__stdcall* FP_KillTimer)(HWND hWnd, UINT uIDEvent);
	static COMPILETIMEEVAL referencefunc<FP_KillTimer, 0x7E148C> const KillTimer {};

	typedef LONG(__stdcall* FP_SendDlgItemMessageA)(HWND hDlg, int nIDDlgItem, UINT Msg, WPARAM wParam, LPARAM lParam);
	static COMPILETIMEEVAL referencefunc<FP_SendDlgItemMessageA, 0x7E1490> const SendDlgItemMessageA {};

	typedef UINT(__stdcall* FP_SetTimer)(HWND hWnd, UINT nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc);
	static COMPILETIMEEVAL referencefunc<FP_SetTimer, 0x7E1494> const SetTimer {};

	typedef BOOL(__stdcall* FP_ShowWindow)(HWND hWnd, int nCmdShow);
	static COMPILETIMEEVAL referencefunc<FP_ShowWindow, 0x7E1498 > const ShowWindow {};

	typedef BOOL(__stdcall* FP_InvalidateRect)(HWND hWnd, const RECT* lpRect, BOOL bErase);
	static COMPILETIMEEVAL referencefunc<FP_InvalidateRect, 0x7E149C > const InvalidateRect {};

	typedef BOOL(__stdcall* FP_EnableWindow)(HWND hWnd, BOOL bEnable);
	static COMPILETIMEEVAL referencefunc<FP_EnableWindow, 0x7E14A0> const EnableWindow {};

	typedef LRESULT(__stdcall* FP_SendMessageA)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static COMPILETIMEEVAL referencefunc<FP_SendMessageA, 0x7E14A4> const SendMessageA {};

	typedef HWND(__stdcall* FP_GetDlgItem)(HWND hDlg, int nIDDlgItem);
	static COMPILETIMEEVAL referencefunc<FP_GetDlgItem, 0x7E14A8 > const GetDlgItem {};

	typedef BOOL(__stdcall* FP_PostMessageA)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static COMPILETIMEEVAL referencefunc<FP_PostMessageA, 0x7E14AC > const PostMessageA {};

	typedef int (*FP_wsprintfA)(LPSTR, LPCSTR, ...);
	static COMPILETIMEEVAL referencefunc<FP_wsprintfA, 0x7E14B0> const wsprintfA {};

	typedef BOOL(__stdcall* FP_SetRect)(LPRECT lprc, int xLeft, int yTop, int xRight, int yBottom);
	static COMPILETIMEEVAL referencefunc<FP_SetRect, 0x7E14B4 > const SetRect {};

	typedef BOOL(__stdcall* FP_ClientToScreen)(HWND hWnd, LPPOINT lpPoint);
	static COMPILETIMEEVAL referencefunc<FP_ClientToScreen, 0x7E14B8> const ClientToScreen {};

	typedef BOOL(__stdcall* FP_TranslateMessage)(const MSG* lpMsg);
	static COMPILETIMEEVAL referencefunc<FP_TranslateMessage, 0x7E14BC> const TranslateMessage {};

	typedef LONG(__stdcall* FP_DispatchMessageA)(const MSG* lpMsg);
	static COMPILETIMEEVAL referencefunc<FP_DispatchMessageA, 0x7E14C0> const DispatchMessageA {};

	typedef BOOL(__stdcall* FP_GetClientRect)(HWND hWnd, LPRECT lpRect);
	static COMPILETIMEEVAL referencefunc<FP_GetClientRect, 0x7E14C4 > const GetClientRect {};

	typedef HWND(__stdcall* FP_GetWindow)(HWND hWnd, UINT uCmd);
	static COMPILETIMEEVAL referencefunc<FP_GetWindow, 0x7E14C8> const GetWindow {};

	typedef BOOL(__stdcall* FP_BringWindowToTop)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_BringWindowToTop, 0x7E14CC> const BringWindowToTop {};

	typedef BOOL(__stdcall* FP_SetForegroundWindow)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_SetForegroundWindow, 0x7E14D0 > const SetForegroundWindow {};

	typedef HWND(__stdcall* FP_CreateWindowExA)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
	static COMPILETIMEEVAL referencefunc<FP_CreateWindowExA, 0x7E14D4> const CreateWindowExA {};

	typedef ATOM(__stdcall* FP_RegisterClassA)(const WNDCLASSA* lpWndClass);
	static COMPILETIMEEVAL referencefunc<FP_RegisterClassA, 0x7E14D8> const RegisterClassA {};

	typedef int(__stdcall* FP_GetClassNameA)(HWND hWnd, LPSTR lpClassName, int nMaxCount);
	static COMPILETIMEEVAL referencefunc<FP_GetClassNameA, 0x7E14DC> const GetClassNameA {};

	typedef BOOL(__stdcall* FP_IsWindowVisible)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_IsWindowVisible, 0x7E14E0> const IsWindowVisible {};

	typedef BOOL(__stdcall* FP_EnumChildWindows)(HWND hWndParent, WNDENUMPROC lpEnumFunc, LPARAM lParam);
	static COMPILETIMEEVAL referencefunc<FP_EnumChildWindows, 0x7E14E4> const EnumChildWindows {};

	typedef BOOL(__stdcall* FP_IsWindowEnabled)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_IsWindowEnabled, 0x7E14E8> const IsWindowEnabled {};

	typedef HWND(__stdcall* FP_GetParent)(HWND hWnd);
	static COMPILETIMEEVAL referencefunc<FP_GetParent, 0x7E14EC > const GetParent {};

	typedef HWND(__stdcall* FP_GetNextDlgTabItem)(HWND hDlg, HWND hCtl, BOOL bPrevious);
	static COMPILETIMEEVAL referencefunc<FP_GetNextDlgTabItem, 0x7E14F0 > const GetNextDlgTabItem {};

	typedef BOOL(__stdcall* FP_IsDialogMessageA)(HWND hDlg, LPMSG lpMsg);
	static COMPILETIMEEVAL referencefunc<FP_IsDialogMessageA, 0x7E14F4> const IsDialogMessageA {};

	typedef int(__stdcall* FP_TranslateAcceleratorA)(HWND hWnd, HACCEL hAccTable, LPMSG lpMsg);
	static COMPILETIMEEVAL referencefunc<FP_TranslateAcceleratorA, 0x7E14F8> const TranslateAcceleratorA {};

	typedef BOOL(__stdcall* FP_CharToOemBuffA)(LPCSTR lpszSrc, LPSTR lpszDst, DWORD cchDstLength);
	static COMPILETIMEEVAL referencefunc<FP_CharToOemBuffA, 0x7E14FC > const CharToOemBuffA {};

	typedef HDC(__stdcall* FP_BeginPaint)(HWND hWnd, LPPAINTSTRUCT lpPaint);
	static COMPILETIMEEVAL referencefunc<FP_BeginPaint, 0x7E1500 > const BeginPaint {};

	typedef BOOL(__stdcall* FP_EndPaint)(HWND hWnd, const PAINTSTRUCT* lpPaint);
	static COMPILETIMEEVAL referencefunc<FP_EndPaint, 0x7E1504 > const EndPaint {};

	typedef HWND(__stdcall* FP_CreateDialogParamA)(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
	static COMPILETIMEEVAL referencefunc<FP_CreateDialogParamA, 0x7E1508 > const CreateDialogParamA {};

	typedef int(__stdcall* FP_GetWindowTextA)(HWND hWnd, LPSTR lpString, int nMaxCount);
	static COMPILETIMEEVAL referencefunc<FP_GetWindowTextA, 0x7E150C> const GetWindowTextA {};

	typedef BOOL(__stdcall* FP_RegisterHotKey)(HWND hWnd, int id, UINT fsModifiers, UINT vk);
	static COMPILETIMEEVAL referencefunc<FP_RegisterHotKey, 0x7E1510 > const RegisterHotKey {};

	typedef LONG(__stdcall* FP_InterlockedIncrement)(void* lpAddend);
	static COMPILETIMEEVAL referencefunc<FP_InterlockedIncrement, 0x7E11C8> const  InterlockedIncrementFunc {};

	typedef LONG(__stdcall* FP_InterlockedDecrement)(void* lpAddend);
	static COMPILETIMEEVAL referencefunc<FP_InterlockedDecrement, 0x7E11CC> const InterlockedDecrementFunc {};

	typedef void(__stdcall* FP_DeleteCriticalSection)(LPCRITICAL_SECTION lpCriticalSection);
	static COMPILETIMEEVAL referencefunc<FP_DeleteCriticalSection, 0x7E11E4> const DeleteCriticalSection {};

	typedef void(__stdcall* FP_EnterCriticalSection)(LPCRITICAL_SECTION lpCriticalSection);
	static COMPILETIMEEVAL referencefunc<FP_EnterCriticalSection, 0x7E11E8 > const EnterCriticalSection {};

	typedef void(__stdcall* FP_LeaveCriticalSection)(LPCRITICAL_SECTION lpCriticalSection);
	static COMPILETIMEEVAL referencefunc<FP_LeaveCriticalSection, 0x7E11EC> const LeaveCriticalSection {};

	typedef void(__stdcall* FP_InitializeCriticalSection)(LPCRITICAL_SECTION lpCriticalSection);
	static COMPILETIMEEVAL referencefunc<FP_InitializeCriticalSection, 0x7E11F4> const InitializeCriticalSection {};

	typedef void(__stdcall* FP_Sleep)(DWORD dwMilliseconds);
	static COMPILETIMEEVAL referencefunc<FP_Sleep, 0x7E11F0> const Sleep {};

	//Kernel32.dll
	typedef BOOL(__stdcall* FP_FindClose)(HANDLE hFindFile);
	static COMPILETIMEEVAL referencefunc<FP_FindClose, 0x7E1300> const FindClose {};

	typedef BOOL(__stdcall* FP_CloseHandle)(HANDLE hObject);
	static COMPILETIMEEVAL referencefunc<FP_CloseHandle, 0x7E11E0> const CloseHandle {};

	typedef DWORD(__stdcall* FP_WaitForSingleObject)(HANDLE hHandle, DWORD dwMilliseconds);
	static COMPILETIMEEVAL referencefunc<FP_WaitForSingleObject, 0x7E11DC> const WaitForSingleObject {};

	typedef DWORD(__stdcall* FP_FormatMessageA)(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments);
	static COMPILETIMEEVAL referencefunc<FP_FormatMessageA, 0x7E1204> const FormatMessageA {};

	typedef DWORD(__stdcall* FP_GetLastError)();
	static COMPILETIMEEVAL referencefunc<FP_GetLastError, 0x7E1200> const GetLastError {};

	typedef BOOL(__stdcall* FP_SetThreadPriority)(HANDLE hThread, int nPriority);
	static COMPILETIMEEVAL referencefunc<FP_SetThreadPriority, 0x7E11FC> const SetThreadPriority {};

	typedef HANDLE(__stdcall* FP_CreateThread)(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
	static COMPILETIMEEVAL referencefunc<FP_CreateThread, 0x7E11F8> const CreateThread {};

	typedef BOOL(__stdcall* FP_ReleaseMutex)(HANDLE hMutex);
	static COMPILETIMEEVAL referencefunc<FP_ReleaseMutex, 0x7E1198> const ReleaseMutex {};

	typedef HANDLE(__stdcall* FP_CreateMutexA)(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName);
	static COMPILETIMEEVAL referencefunc<FP_CreateMutexA, 0x7E1248 > const CreateMutexA {};

	//
	typedef HMODULE(__stdcall* FP_LoadLibraryA)(LPCSTR lpLibFileName);
	static COMPILETIMEEVAL referencefunc<FP_LoadLibraryA, 0x7E1220> const LoadLibraryA {};

	typedef BOOL(__stdcall* FP_FreeLibrary)(HMODULE hLibModule);
	static COMPILETIMEEVAL referencefunc<FP_FreeLibrary, 0x7E1224> const FreeLibrary {};

	typedef int(__stdcall* FP_GetLocaleInfoA)(LCID Locale, LCTYPE LCType, LPSTR lpLCData, int cchData);
	static COMPILETIMEEVAL referencefunc<FP_GetLocaleInfoA, 0x7E130C> const GetLocaleInfoA {};

	typedef int(__stdcall* FP_MultiByteToWideChar)(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
	static COMPILETIMEEVAL referencefunc<FP_MultiByteToWideChar, 0x7E11C4> const MultiByteToWideChar {};

	typedef HANDLE(__stdcall* FP_CreateFileA)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
	static COMPILETIMEEVAL referencefunc<FP_CreateFileA, 0x7E11BC> const CreateFileA {};

	typedef BOOL(__stdcall* FP_ReadFile)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
	static COMPILETIMEEVAL referencefunc<FP_ReadFile, 0x7E111C> const ReadFile {};

	typedef void(__stdcall* FP_GetLocalTime)(LPSYSTEMTIME lpSystemTime);
	static COMPILETIMEEVAL referencefunc<FP_GetLocalTime, 0x7E1284> const GetLocalTime {};

	typedef DWORD(__stdcall* FP_GetFileSize)(HANDLE hFile, LPDWORD lpFileSizeHigh);
	static COMPILETIMEEVAL referencefunc<FP_GetFileSize, 0x7E1120> const GetFileSize {};

	typedef DWORD(__stdcall* FP_SetFilePointer)(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
	static COMPILETIMEEVAL referencefunc<FP_SetFilePointer, 0x7E11C0> const SetFilePointer {};

	typedef DWORD(__stdcall* FP_SetFileTime)(HANDLE hFile, const FILETIME* lpCreationTime, const FILETIME* lpLastAccessTime, const FILETIME* lpLastWriteTime);
	static COMPILETIMEEVAL referencefunc<FP_SetFileTime, 0x7E112C> const SetFileTime {};

};

//DeleterType is required to identify
template<typename Deleter>
requires requires(Deleter) { Deleter::DeleterType; }
class MovieInfo
{
public:
	// technically, this is a DVC<const char*>
	// and string management is done manually
	static COMPILETIMEEVAL reference<DynamicVectorClass<MovieInfo<GameDeleter>>, 0xABF390u> const Array {};

	bool operator== (MovieInfo const& rhs) const
	{
		if COMPILETIMEEVAL (Deleter::DeleterType == DeleterType::GameDeleter || Deleter::DeleterType == DeleterType::GameDTORCaller)
			return !CRT::strcmpi(this->Name, rhs.Name);
		else
			return !_strcmpi(this->Name, rhs.Name);
	}

	bool operator!= (MovieInfo const& rhs) const
	{ return !((*this) == rhs); }

	explicit MovieInfo(const char* fname)
		: Name(nullptr)
	{
		if(fname) {
			if COMPILETIMEEVAL (Deleter::DeleterType == DeleterType::GameDeleter || Deleter::DeleterType == DeleterType::GameDTORCaller)
				Name = CRT::strdup(fname);
			else
				Name = _strdup(fname);
		}
	}

	MovieInfo() : Name(nullptr) { }

	~MovieInfo() {
		if (this->Name) {
			Deleter()(const_cast<char*>(this->Name));
		}
	}

	const char* Name; // yes, only that
};

struct MovieUnlockableInfo
{
	static COMPILETIMEEVAL reference<MovieUnlockableInfo, 0x832C20u, 1u> const Common {};
	static COMPILETIMEEVAL reference<MovieUnlockableInfo, 0x832C30u, 8u> const Allied {};
	static COMPILETIMEEVAL reference<MovieUnlockableInfo, 0x832CA0u, 8u> const Soviet {};

	COMPILETIMEEVAL MovieUnlockableInfo() = default;

	COMPILETIMEEVAL explicit MovieUnlockableInfo(const char* pFilename, const char* pDescription = nullptr, int disk = 2)
		: Filename(pFilename), Description(pDescription), DiskRequired(disk)
	{
	}

	// the destructor doesnt delete the string data
	// please manage them
	~MovieUnlockableInfo() = default;

	const char* Filename { nullptr };
	const char* Description { nullptr };
	int DiskRequired { 2 };
};

struct ColorPacker
{
	int _R_SHL;
	int _R_SHR;
	int _B_SHL;
	int _B_SHR;
	int _G_SHL;
	int _G_SHR;
};

class BuildingTypeClass;
namespace Unsorted
{
	static COMPILETIMEEVAL reference<Point2D, 0x89F6D8, 8u> const AdjacentCoord {};
	COMPILETIMEEVAL reference<const char* const, 0x7E5210u, 11u> const ArmorNameArray {};

	COMPILETIMEEVAL reference<int, 0xA8ED84u> const CurrentFrame {};
	COMPILETIMEEVAL reference<long, 0xA8ED84u> const l_CurrentFrame {};
	COMPILETIMEEVAL reference<int, 0xA8B568u> const MaxAhead {};
	COMPILETIMEEVAL reference<int, 0xA8DB9Cu> const NetworkFudge {};
	COMPILETIMEEVAL reference<int, 0xA8B554u> const FrameSendRate {};

	// if != 0, EVA_SWxxxActivated is skipped
	COMPILETIMEEVAL reference<int, 0xA8B538> const MuteSWLaunches {};

	// skip unit selection and move command voices?
	COMPILETIMEEVAL reference<bool, 0x822CF2> const MoveFeedback {};

	COMPILETIMEEVAL reference<bool, 0xA8ED6B> const ArmageddonMode {};
	COMPILETIMEEVAL reference<bool, 0xA8E9A0> const WTFMode {};
	COMPILETIMEEVAL constant_ptr<DynamicVectorClass<ObjectClass*>, 0x8A0360> const ObjectsInLayers {};

	// checkbox states, afaik
	COMPILETIMEEVAL reference<bool, 0xA8B258> const Bases {};
	COMPILETIMEEVAL reference<bool, 0xA8B260> const BridgeDestruction {};
	COMPILETIMEEVAL reference<bool, 0xA8B261> const Crates {};
	COMPILETIMEEVAL reference<bool, 0xA8B262> const ShortGame {};
	COMPILETIMEEVAL reference<bool, 0xA8B263> const SWAllowed {};
	COMPILETIMEEVAL reference<bool, 0xA8B26C> const MultiEngineer {};
	COMPILETIMEEVAL reference<bool, 0xA8B31C> const AlliesAllowed {};
	COMPILETIMEEVAL reference<bool, 0xA8B31D> const HarvesterTruce {};
	COMPILETIMEEVAL reference<bool, 0xA8B31E> const CTF {};
	COMPILETIMEEVAL reference<bool, 0xA8B31F> const FOW {};
	COMPILETIMEEVAL reference<bool, 0xA8B320> const MCVRedeploy {};

	COMPILETIMEEVAL reference<TacticalSelectableStruct, 0xB0CEC8, 500> TacticalSelectables {};
	COMPILETIMEEVAL reference<bool, 0xB0FE65> const TypeSelecting {};
	COMPILETIMEEVAL constant_ptr<HWND, 0xB73550u> const Game_hWnd_ptr {};

	COMPILETIMEEVAL constant_ptr<ColorPacker, 0x8A0DD0> const ColorPackData {};
	COMPILETIMEEVAL reference<int, 0x8809A0> const CurrentSWType {};
	COMPILETIMEEVAL reference<bool, 0x88099A> const PowerToggleMode {};
	COMPILETIMEEVAL reference<bool, 0x88099B> const PlanningMode {};
	COMPILETIMEEVAL reference<bool, 0x88099C> const PlaceBeaconMode {};
	COMPILETIMEEVAL reference<DWORD , 0x880994> const unknown_11AC {};
	COMPILETIMEEVAL reference<void*, 0x880990> const CurrentBuildingType {};
	COMPILETIMEEVAL reference<void*, 0x88098C> const CurrentBuilding {};

	COMPILETIMEEVAL const int except_txt_length = 0xFFFF;
	COMPILETIMEEVAL constant_ptr<char, 0x8A3A08> const except_txt_content {};

	// Note: SomeMutex has been renamed to this because it reflects the usage better
	COMPILETIMEEVAL reference<int, 0xA8E7AC> const ScenarioInit {}; // h2ik
	COMPILETIMEEVAL reference<int , 0xB1D480> const ScenarioInit_Audio {};
	COMPILETIMEEVAL reference<int, 0xA8DAB4> const SystemResponseMessages {};

	COMPILETIMEEVAL reference<CellStruct*, 0x880964u> const CursorSize {};
	COMPILETIMEEVAL reference<CellStruct*, 0x880974u> const CursorSizeSecond {};
	COMPILETIMEEVAL reference<CellStruct, 0x88095Cu> const Display_ZoneCell {};
	COMPILETIMEEVAL reference<CellStruct, 0x88096Au> const Display_ZoneCell2 {};
	COMPILETIMEEVAL reference<CellStruct, 0x880960u> const Display_ZoneOffset {};
	COMPILETIMEEVAL reference<CellStruct, 0x88096Eu> const Display_ZoneOffset2 {};
	COMPILETIMEEVAL reference<bool, 0x880968u> const Display_PassedProximityCheck{};
	COMPILETIMEEVAL reference<BuildingTypeClass*, 0x880990u> const Display_PendingObject{};
	COMPILETIMEEVAL reference<int, 0x880994u> const Display_PendingHouse{};
};

struct CheatData
{
	bool* Destination;
	const char* TriggerString;
	DWORD unknown1;
	DWORD unknown2;

	// this holds four original cheats, keep that limit in mind
	static COMPILETIMEEVAL reference<CheatData, 0x825C28 , 4u> OriginalCheats {};
};

