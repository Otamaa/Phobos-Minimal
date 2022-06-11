#pragma once

#include <Interface/ILocomotion.h>
#include <Interface/IPiggyback.h>
#include <FootClass.h>
#include <Unsorted.h>
#include <YRCom.h>
#include <Helpers/ComPtr.h>
#include <Helpers/CompileTime.h>
#include <Matrix3D.h>

class LocomotionClass : public IPersistStream, public ILocomotion
{
public:
	class CLSIDs
	{
	public:
		LOCO_CLSID(Drive, 0x7E9A30u)
		LOCO_CLSID(Jumpjet, 0x7E9AC0u)
		LOCO_CLSID(Hover, 0x7E9A40u)
		LOCO_CLSID(Rocket, 0x7E9AD0u)
		LOCO_CLSID(Tunnel, 0x7E9A50u)
		LOCO_CLSID(Walk, 0x7E9A60u)
		LOCO_CLSID(Droppod, 0x7E9A70u)
		LOCO_CLSID(Fly, 0x7E9A80u)
		LOCO_CLSID(Teleport, 0x7E9A90u)
		LOCO_CLSID(Mech, 0x7E9AA0u)
		LOCO_CLSID(Ship, 0x7E9AB0u)

	};
	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, LPVOID* ppvObject) { JMP_STD(0x55A9B0); }
	virtual ULONG __stdcall AddRef() { JMP_STD(0x55A950); }
	virtual ULONG __stdcall Release() { JMP_STD(0x55A970); }

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall IsDirty() { return Dirty ? S_OK : S_FALSE; }
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) { JMP_STD(0x55AA60); }

	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) { JMP_STD(0x55AB40); }

	virtual ~LocomotionClass() { JMP_THIS(0x55A6F0); } // should be SDDTOR in fact
	virtual int Size() = 0;

	// ILocomotion
	// virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) R0;
	// virtual ULONG __stdcall AddRef() R0;
	// virtual ULONG __stdcall Release() R0;
	virtual HRESULT __stdcall Link_To_Object(void* pointer) { JMP_STD(0x55A710); }
	virtual bool __stdcall Is_Moving() { return false; };
	virtual CoordStruct __stdcall Destination()
	{ return CoordStruct { 0,0,0 }; }
	virtual CoordStruct __stdcall Head_To_Coord()
	{ return LinkedTo->Location; }
	virtual Move __stdcall Can_Enter_Cell(CellStruct cell) { return Move::OK; }
	virtual bool __stdcall Is_To_Have_Shadow() { return true; }
	virtual Matrix3D __stdcall Draw_Matrix(int* facing) { JMP_STD(0x55A730); }
	virtual Matrix3D __stdcall Shadow_Matrix(int* facing) { JMP_STD(0x55A7D0); }
	virtual Point2D __stdcall Draw_Point()
	{ return Point2D{ 0,0 }; }
	virtual Point2D __stdcall Shadow_Point() { JMP_STD(0x55A8C0); }
	virtual VisualType __stdcall Visual_Character(VARIANT_BOOL unused) { return VisualType::Normal; }
	virtual int __stdcall Z_Adjust() { return 0; }
	virtual ZGradient __stdcall Z_Gradient() { return ZGradient::Deg90; }
	virtual bool __stdcall Process() { return true; }
	virtual void __stdcall Move_To(CoordStruct to) RX;
	virtual void __stdcall Stop_Moving() RX;
	virtual void __stdcall Do_Turn(DirStruct coord) RX;
	virtual void __stdcall Unlimbo() RX;
	virtual void __stdcall Tilt_Pitch_AI() RX;
	virtual bool __stdcall Power_On() { Powered = true; return Is_Powered(); }
	virtual bool __stdcall Power_Off() { Powered = false; return Is_Powered(); }
	virtual bool __stdcall Is_Powered() { return Powered; }
	virtual bool __stdcall Is_Ion_Sensitive() { return false; }
	virtual bool __stdcall Push(DirStruct dir) { return false; }
	virtual bool __stdcall Shove(DirStruct dir) { return false; }
	virtual void __stdcall Force_Track(int track, CoordStruct coord) RX;
	//virtual Layer __stdcall In_Which_Layer() RT(Layer);
	virtual void __stdcall Force_Immediate_Destination(CoordStruct coord) RX;
	virtual void __stdcall Force_New_Slope(int ramp) RX;
	virtual bool __stdcall Is_Moving_Now() { return Is_Moving(); }
	virtual int __stdcall Apparent_Speed() { return LinkedTo->GetCurrentSpeed(); }
	virtual int __stdcall Drawing_Code() { return 0; }
	virtual FireError __stdcall Can_Fire() { return FireError::OK; }
	virtual int __stdcall Get_Status() { return 0; }
	virtual void __stdcall Acquire_Hunter_Seeker_Target() RX;
	virtual bool __stdcall Is_Surfacing() { return false; }
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) RX;
	virtual bool __stdcall Is_Moving_Here(CoordStruct to) { return false; }
	virtual bool __stdcall Will_Jump_Tracks() { return false; }
	virtual bool __stdcall Is_Really_Moving_Now() { return Is_Moving_Now(); }
	virtual void __stdcall Stop_Movement_Animation() RX;
	virtual void __stdcall Clear_Coords() RX;
	virtual void __stdcall Lock() RX;
	virtual void __stdcall Unlock() RX;
	virtual int __stdcall Get_Track_Number() { return -1; }
	virtual int __stdcall Get_Track_Index() { return -1; }
	virtual int __stdcall Get_Speed_Accum() { return -1; }

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
	static YRComPtr<ILocomotion> CreateInstance(const CLSID& rclsid)
	{
		return YRComPtr<ILocomotion>(rclsid, nullptr,
			CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER);
	}

	// finds out whether a locomotor is currently piggybacking and restores
	// the original locomotor. this function ignores Is_Ok_To_End().
	static bool End_Piggyback(YRComPtr<ILocomotion> &pLoco);

	//Constructors
	LocomotionClass()
		: LocomotionClass(noinit_t())
	{ JMP_THIS(0x55A6C0); }
protected:
	explicit __forceinline LocomotionClass(noinit_t)  noexcept
	{ }

	//Properties
public:

	FootClass* Owner;
	FootClass* LinkedTo;
	bool Powered;
	bool Dirty;
	LONG RefCount;

private:
	LocomotionClass(const LocomotionClass&) = delete;
	LocomotionClass& operator=(const LocomotionClass&) = delete;
};

//static_assert(sizeof(LocomotionClass) == 0x18);
