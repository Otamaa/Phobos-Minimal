#pragma once

#include <unknwn.h>
#include <GeneralStructures.h>
#include <CoordStruct.h>
#include <Matrix3D.h>
#include <VoxelIndex.h>

DECLARE_INTERFACE_IID_(ILocomotion, IUnknown, "070F3290-9841-11D1-B709-00A024DDAFD1")
{
	virtual HRESULT __stdcall Link_To_Object(void* pointer) PURE; //Links object to locomotor.
	virtual bool __stdcall Is_Moving() PURE;	//Sees if object is moving.
	virtual CoordStruct __stdcall Destination() PURE;	//Fetches destination coordinate.
	virtual CoordStruct __stdcall Head_To_Coord() PURE; // Fetches immediate (next cell) destination coordinate.
	virtual Move __stdcall Can_Enter_Cell(CellStruct cell) PURE; //Determine if specific cell can be entered.
	virtual bool __stdcall Is_To_Have_Shadow() PURE;	//Should object cast a shadow?
	virtual Matrix3D* __stdcall Draw_Matrix(Matrix3D* pMatrix, VoxelIndexKey * key) PURE; //Fetch voxel draw matrix.
	virtual Matrix3D* __stdcall Shadow_Matrix(Matrix3D * pMatrix, VoxelIndexKey * key) PURE;	//Fetch shadow draw matrix.
	virtual Point2D __stdcall Draw_Point() PURE;	//Draw point center location.
	virtual Point2D __stdcall Shadow_Point() PURE;	//Shadow draw point center location.
	virtual VisualType __stdcall Visual_Character(VARIANT_BOOL unused) PURE;	//Visual character for drawing.
	virtual int __stdcall Z_Adjust() PURE;	//Z adjust control value.
	virtual ZGradient __stdcall Z_Gradient() PURE;	//Z gradient control value.
	virtual bool __stdcall Process() PURE;	//Process movement of object.]
	virtual void __stdcall Move_To(CoordStruct to) PURE;	//Instruct to move to location specified.
	virtual void __stdcall Stop_Moving() PURE;	//Stop moving at first opportunity.
	virtual void __stdcall Do_Turn(DirStruct coord) PURE;	//Try to face direction specified.
	virtual void __stdcall Unlimbo() PURE;	//Object is appearing in the world.
	virtual void __stdcall Tilt_Pitch_AI() PURE;	//Special tilting AI function.
	virtual bool __stdcall Power_On() PURE;	//Locomotor becomes powered.
	virtual bool __stdcall Power_Off() PURE;	//Locomotor loses power.
	virtual bool __stdcall Is_Powered() PURE;	//Is locomotor powered?
	virtual bool __stdcall Is_Ion_Sensitive() PURE;	//Is locomotor sensitive to ion storms?
	virtual bool __stdcall Push(DirStruct dir) PURE;	//Push object in direction specified.
	virtual bool __stdcall Shove(DirStruct dir) PURE;	//Shove object (with spin) in direction specified.
	virtual void __stdcall Force_Track(int track, CoordStruct coord) PURE;	//Force drive track -- special case only.
	virtual Layer __stdcall In_Which_Layer() PURE;	//What display layer is it located in.
	virtual void __stdcall Force_Immediate_Destination(CoordStruct coord) PURE;	//Don't use this function.
	virtual void __stdcall Force_New_Slope(int ramp) PURE;	//Force a voxel unit to a given slope. Used in cratering.
	virtual bool __stdcall Is_Moving_Now() PURE;	//Is it actually moving across the ground this very second?
	virtual int __stdcall Apparent_Speed() PURE;	//Actual current speed of object expressed as leptons per game frame.
	virtual int __stdcall Drawing_Code() PURE;	//Special drawing feedback code (locomotor specific meaning)
	virtual FireError __stdcall Can_Fire() PURE;	//Queries if any locomotor specific state prevents the object from firing.
	virtual int __stdcall Get_Status() PURE;	//Queries the general state of the locomotor.
	virtual void __stdcall Acquire_Hunter_Seeker_Target() PURE;	//Forces a hunter seeker droid to find a target.
	virtual bool __stdcall Is_Surfacing() PURE;	//Is this object surfacing?
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) PURE;	//Lifts all occupation bits associated with the object off the map
	virtual bool __stdcall Is_Moving_Here(CoordStruct to) PURE;	//Is this object in the process of moving into this coord.
	virtual bool __stdcall Will_Jump_Tracks() PURE;	//Will this object jump tracks?
	virtual bool __stdcall Is_Really_Moving_Now() PURE;	//Infantry moving query function
	virtual void __stdcall Stop_Movement_Animation() PURE;	//Falsifies the IsReallyMoving flag in WalkLocomotionClass
	virtual void __stdcall Clear_Coords() PURE;	//Unknown, must have been added after LOCOS.TLB was generated. -pd
	virtual void __stdcall Lock() PURE;	//Locks the locomotor from being deleted
	virtual void __stdcall Unlock() PURE;	//Unlocks the locomotor from being deleted
	virtual int __stdcall Get_Track_Number() PURE;	//Queries internal variables
	virtual int __stdcall Get_Track_Index() PURE;	//Queries internal variables
	virtual int __stdcall Get_Speed_Accum() PURE;	//Queries internal variables
};