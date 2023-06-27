#pragma once

#include <Interface/ILocomotion.h>
#include <Interface/IPiggyback.h>
#include <FootClass.h>
#include <Unsorted.h>
#include <YRCom.h>
#include <Helpers/ComPtr.h>
#include <Matrix3D.h>
#include "CLSIDs.h"

struct LocoIdent
{
	wchar_t* w_name;
	const char* s_name;
	wchar_t* w_CLSID;
	const char* s_CLSID;
};

class NOVTABLE LocomotionClass : public IPersistStream, public ILocomotion
{
public:

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, LPVOID* ppvObject) { JMP_STD(0x55A9B0); }
	virtual ULONG __stdcall AddRef() { JMP_STD(0x55A950); }
	virtual ULONG __stdcall Release() { JMP_STD(0x55A970); }

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) = 0;

	//IPersistStream
	virtual HRESULT __stdcall IsDirty() { JMP_STD(0x4B4C30); }
	virtual HRESULT __stdcall Load(IStream* pStm) = 0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) = 0;

	static HRESULT __stdcall Internal_Load(LocomotionClass* pThis, IStream* pStm) { JMP_STD(0x55AAC0) };
	static HRESULT __stdcall Internal_Save(LocomotionClass* pThis, IStream* pStm, BOOL fClearDirty) { JMP_STD(0x55AA60); }

	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) { JMP_STD(0x55AB40); }

	virtual ~LocomotionClass() { JMP_THIS(0x55A6F0); } // should be SDDTOR in fact
	virtual int Size() = 0;

	// ILocomotion
	virtual HRESULT __stdcall Link_To_Object(void* pointer) { JMP_STD(0x55A710); }
	virtual bool __stdcall Is_Moving() { JMP_STD(0x55ACD0); }
	virtual CoordStruct __stdcall Destination() { JMP_STD(0x55AC70); }
	virtual CoordStruct __stdcall Head_To_Coord() { JMP_STD(0x55ACA0); }
	virtual Move __stdcall Can_Enter_Cell(CellStruct cell) { JMP_STD(0x55ABF0); }
	virtual bool __stdcall Is_To_Have_Shadow() { JMP_STD(0x55ABE0); }
	virtual Matrix3D* __stdcall Draw_Matrix(Matrix3D* pMatrix, VoxelIndexKey* key) { JMP_STD(0x55A730); }
	virtual Matrix3D* __stdcall Shadow_Matrix(Matrix3D* pMatrix, VoxelIndexKey* key) { JMP_STD(0x55A7D0); }
	virtual Point2D __stdcall Draw_Point() { JMP_STD(0x55ABD0); }
	virtual Point2D __stdcall Shadow_Point() { JMP_STD(0x55A8C0); }
	virtual VisualType __stdcall Visual_Character(VARIANT_BOOL unused) { JMP_STD(0x55ABC0); }
	virtual int __stdcall Z_Adjust() { JMP_STD(0x55ABA0); }
	virtual ZGradient __stdcall Z_Gradient() { JMP_STD(0x55ABB0); }
	virtual bool __stdcall Process() { JMP_STD(0x55AC60); }
	virtual void __stdcall Move_To(CoordStruct to) { JMP_STD(0x55AC50); }
	virtual void __stdcall Stop_Moving()  { JMP_STD(0x55AC40); }
	virtual void __stdcall Do_Turn(DirStruct coord) { JMP_STD(0x55AC30); }
	virtual void __stdcall Unlimbo() { JMP_STD(0x55AC20); }
	virtual void __stdcall Tilt_Pitch_AI() { JMP_STD(0x55AB90); }
	virtual bool __stdcall Power_On() { JMP_STD(0x55A8F0); }
	virtual bool __stdcall Power_Off() { JMP_STD(0x55A910); }
	virtual bool __stdcall Is_Powered() { JMP_STD(0x55A930); }
	virtual bool __stdcall Is_Ion_Sensitive() { JMP_STD(0x55A940); }
	virtual bool __stdcall Push(DirStruct dir) { JMP_STD(0x55AB70); }
	virtual bool __stdcall Shove(DirStruct dir) { JMP_STD(0x55AB80); }
	virtual void __stdcall Force_Track(int track, CoordStruct coord) { JMP_STD(0x55AC10); }
	virtual Layer __stdcall In_Which_Layer() = 0;
	virtual void __stdcall Force_Immediate_Destination(CoordStruct coord) { JMP_STD(0x55AC00); }
	virtual void __stdcall Force_New_Slope(int ramp) { JMP_STD(0x55ACE0); }
	virtual bool __stdcall Is_Moving_Now() { JMP_STD(0x4B6610); }
	virtual int __stdcall Apparent_Speed() { JMP_STD(0x55AD10); }
	virtual int __stdcall Drawing_Code() { JMP_STD(0x55ACF0); }
	virtual FireError __stdcall Can_Fire() { JMP_STD(0x55AD00); }
	virtual int __stdcall Get_Status() { JMP_STD(0x4B4C60); }
	virtual void __stdcall Acquire_Hunter_Seeker_Target() { JMP_STD(0x4B4C70); }
	virtual bool __stdcall Is_Surfacing() { JMP_STD(0x4B4C80); }
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) { JMP_STD(0x4B6620); }
	virtual bool __stdcall Is_Moving_Here(CoordStruct to) { JMP_STD(0x4B6630); }
	virtual bool __stdcall Will_Jump_Tracks() { JMP_STD(0x4B6640); }
	virtual bool __stdcall Is_Really_Moving_Now() { JMP_STD(0x4B4C50); }
	virtual void __stdcall Stop_Movement_Animation() { JMP_STD(0x4B4C90); }
	virtual void __stdcall Clear_Coords() { JMP_STD(0x4B4CA0); }
	virtual void __stdcall Lock() { JMP_STD(0x4B6650); }
	virtual void __stdcall Unlock() { JMP_STD(0x4B6660); }
	virtual int __stdcall Get_Track_Number() { JMP_STD(0x4B6670); }
	virtual int __stdcall Get_Track_Index() { JMP_STD(0x4B6680); }
	virtual int __stdcall Get_Speed_Accum() { JMP_STD(0x4B6690); }

	// Non virtuals
	static HRESULT TryPiggyback(IPiggyback** Piggy, ILocomotion** Loco)
	{ PUSH_VAR32(Loco); SET_REG32(ECX, Piggy); CALL(0x45AF20); }

	static HRESULT CreateInstance(ILocomotion** ppv, const CLSID* rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext)
	{ PUSH_VAR32(dwClsContext); PUSH_VAR32(pUnkOuter); PUSH_VAR32(rclsid); SET_REG32(ECX, ppv); CALL(0x41C250); }

	// these two are identical, why do they both exist...
	static void AddRef1(LocomotionClass** Loco)
	{ SET_REG32(ECX, Loco); CALL(0x45A170); }

	static void AddRef2(LocomotionClass** Loco)
	{ SET_REG32(ECX, Loco); CALL(0x6CE270); }

	static void ChangeLocomotorTo(FootClass *Object, const CLSID &clsid);

	// creates a new instance by class ID. returns a pointer to ILocomotion
	static ILocomotionPtr CreateInstance(const CLSID& rclsid)
	{
		return ILocomotionPtr(rclsid, nullptr,
			CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER);
	}

	// finds out whether a locomotor is currently piggybacking and restores
	// the original locomotor. this function ignores Is_Ok_To_End().
	static bool End_Piggyback(ILocomotionPtr &pLoco);

protected:

	//Constructors
	LocomotionClass() { JMP_THIS(0x55A6C0); }
	//LocomotionClass(LocomotionClass* pAnother) { JMP_THIS(0x55A6C0); }

	explicit __forceinline LocomotionClass(noinit_t)  noexcept { }

	//Properties
public:

	FootClass* Owner;
	FootClass* LinkedTo;
	bool Powered;
	bool Dirty;
	int RefCount;

private:
	LocomotionClass(const LocomotionClass&) = delete;
	LocomotionClass& operator=(const LocomotionClass&) = delete;
};

static_assert(sizeof(LocomotionClass) == 0x18);
