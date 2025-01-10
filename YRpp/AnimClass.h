/*
	Animations
*/

#pragma once
#include <ObjectClass.h>
#include <AnimTypeClass.h>
#include <BounceClass.h>
#include <ProgressTimer.h>

enum class AnimFlag : unsigned int
{
	AnimFlag_1 = 0x1,
	AnimFlag_2 = 0x2 ,
	AnimFlag_4 = 0x4,
	AnimFlag_8 = 0x8,
	AnimFlag_10 = 0x10,
	AnimFlag_20 = 0x20,
	AnimFlag_40 = 0x40,
	AnimFlag_80 = 0x80,
	AnimFlag_100 = 0x100,
	AnimFlag_200 = 0x200, // always ?
	AnimFlag_400 = 0x400, // always ?
	AnimFlag_800 = 0x800,
	AnimFlag_1000 = 0x1000, // building anim
	AnimFlag_2000 = 0x2000, // IvanDamage nuke
	AnimFlag_4000 = 0x4000,
	AnimFlag_8000 = 0x8000,
};

MAKE_ENUM_FLAGS(AnimFlag);

//forward declarations
class AnimTypeClass;
class BulletClass;
class HouseClass;
class LightConvertClass;

class DECLSPEC_UUID("0E272DC3-9C0F-11D1-B709-00A024DDAFD1")
	  NOVTABLE AnimClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::Anim;
	static constexpr inline DWORD vtable = 0x7E3354;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<AnimClass*>, 0xA8E9A8u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x426540);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x425280);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x4253B0);

	//Destructor
	virtual ~AnimClass();

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) override JMP_THIS(0x425150);
	virtual AbstractType WhatAmI() const override { return AbstractType::Anim; }
	virtual int	Size() const override { return 0x1C8; }

	//ObjectClass
	//AnimClass
	virtual int AnimExtras() R0; // tumbling for IsMeteor and Bouncer anims
	virtual int GetEnd() const R0; //End tag from the AnimType

	void SetOwnerObject(ObjectClass *pOwner) { JMP_THIS(0x424B50); }
	void SetHouse(HouseClass* pOwner) { this->Owner = pOwner; }
	void SetBullet(BulletClass* pBullet) { this->AttachedBullet = pBullet; }

	void DetachFromObject(ObjectClass* pTarget, bool detachFromAll)
		{ JMP_THIS(0x425150); }

	void Pause() {
		this->Paused = true;
		this->Unpaused = false;
		this->PausedAnimFrame = this->Animation.Value;
	}

	AnimTypeClass* GetAnimType() const
		{ return Type; }

	AnimTypeClass* GetEndAnimType() const
		{ JMP_THIS(0x425510); }

	void Unpause() {
		this->Paused = false;
		this->Unpaused = true;
	}

	BounceClass::Status BounceAI() const { JMP_THIS(0x423930); }

	void FlamingGuy_AI() const { JMP_THIS(0x425670); }

	CoordStruct* FlamingGuy_Coords(CoordStruct* pbuffer) const
	{ JMP_THIS(0x425D10); }

	bool FlamingGuy_Allowed(CellStruct* pCell) const
	{ JMP_THIS(0x4260F0);}

	void Make_Invisible() { Invisible = true; }
	void Make_Visible() { Invisible = false; }

	void Func_426300(CoordStruct* pCoord) const
		{ JMP_THIS(0x426300); }

	void Func_426270(CoordStruct* pCoord) const
		{ JMP_THIS(0x426270); }

	void RemoveThis() const
	{ JMP_THIS(0x4255B0); }

	void DestroyPointer() const { JMP_THIS(0x4228E0); }

	// Anim start logic: sound event handling, tiberium chain reaction etc.
	void Start() const
	{ JMP_THIS(0x424CE0); }

	// Anim midpoint logic: particle spawning, smudges etc.
	bool Middle() const
	{ JMP_THIS(0x424F00); }

	//Constructor
	AnimClass(AnimTypeClass* pAnimType, const CoordStruct& Location, int LoopDelay = 0,
		int LoopCount = 1, AnimFlag flags = AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, int ForceZAdjust = 0, bool reverse = false);

	AnimClass(AnimTypeClass* pAnimType, const CoordStruct& Location, int LoopDelay, int LoopCount, DWORD flags, int ForceZAdjust = 0, bool reverse = false);

	//Coord were refence , just to save time crated this
	AnimClass(AnimTypeClass* pAnimType, CoordStruct* pLocation, int LoopDelay,
	int LoopCount, AnimFlag flags, int ForceZAdjust, bool reverse);

	AnimClass();
public:

	DECLARE_PROPERTY(StageClass, Animation);
	AnimTypeClass* Type; //The AnimType.
	ObjectClass* OwnerObject; // set by AnimClass::SetOwnerObject (0x424B50)
	DWORD unknown_D0;
	LightConvertClass* LightConvert;	 //Palette?
	int LightConvertIndex; // assert( (*ColorScheme::Array)[this->LightConvertIndex] == this->LightConvert ;
	char PaletteName[0x20]; // filename set for destroy anims
	int TintColor;
	int ZAdjust;
	int YSortAdjust; // same as YSortAdjust from Type
	DECLARE_PROPERTY(CoordStruct, FlamingGuyCoords); // the destination the anim tries to reach
	int FlamingGuyRetries; // number of failed attemts to reach water. the random destination generator stops if >= 7
	bool IsBuildingAnim; // whether this anim will invalidate on buildings, and whether it's tintable
	bool UnderTemporal; // temporal'd building's active anims
	bool Paused; // if paused, does not advance anim, does not deliver damage
	bool Unpaused; // set when unpaused
	int PausedAnimFrame; // the animation value when paused
	bool Reverse; // anim is forced to be played from end to start
	DWORD unknown_124;
	DECLARE_PROPERTY(BounceClass, Bounce);
	BYTE TranslucencyLevel; // on a scale of 1 - 100
	bool TimeToDie; // or something to that effect, set just before UnInit
	BulletClass* AttachedBullet;
	HouseClass* Owner; //Used for remap (AltPalette).
	int LoopDelay; // randomized value, depending on RandomLoopDelay
	double Accum; // Stores accumulated fractional animation damage and gets added to Type->Damage if at least 1.0 or above. Defaults to 1.0.
	BlitterFlags AnimFlags; // argument that's 0x600 most of the time
	bool HasExtras; // enables IsMeteor and Bouncer special behavior (AnimExtras)
	BYTE RemainingIterations; // defaulted to deleteAfterIterations, when reaches zero, UnInit() is called
	BYTE __lighting__celldraw_196;
	BYTE __ToDelete_197;
	bool IsPlaying;
	bool IsFogged;
	bool FlamingGuyExpire; // finish animation and remove
	bool UnableToContinue; // set when something prevents the anim from going on: cell occupied, veins destoyed or unit gone, ...
	bool SkipProcessOnce; // set in constructor, cleared during Update. skips damage, veins, tiberium chain reaction and animation progress
	bool Invisible; // don't draw, but Update state anyway
	bool PowerOff; // powered animation has no power
	PROTECTED_PROPERTY(BYTE, unused_19F);
	DECLARE_PROPERTY(AudioController, Audio3);
	DECLARE_PROPERTY(AudioController, Audio4);
};
static_assert(sizeof(AnimClass) == (0x1C8), "Invalid size.");

struct UninitAnim
{
	void operator() (AnimClass* pAnim) const
	{
		if (pAnim && pAnim->Type)
		{
			pAnim->TimeToDie = true;
			pAnim->UnInit();
		}
		pAnim = nullptr;
	}
};