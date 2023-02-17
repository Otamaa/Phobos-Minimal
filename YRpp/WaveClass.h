#pragma once

#include <GeneralDefinitions.h>
#include <FacingClass.h>
#include <ObjectClass.h>

//thse mostlikely some kind of array / multi dimentional array
struct SingleArray_WavePoint2D
{
	int Count;
	Point2D* Points;
};
static_assert(sizeof(SingleArray_WavePoint2D) == 8, "Invalid Size !");

struct MultiDimentionalArray_WavePoint2D
{
	Point2D Pos;
	Point2D* Points;
};
static_assert(sizeof(MultiDimentionalArray_WavePoint2D) == 12, "Invalid Size !");

class CellClass;
class TechnoClass;
class DECLSPEC_UUID("0E272DCD-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE WaveClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::Wave;

	static constexpr constant_ptr<DynamicVectorClass<WaveClass*>, 0xA8EC38u> const Array{};
	static constexpr reference<bool, 0xB725CCu> const IsLUTs_Calculated{};
	static constexpr referencemult<short, 0xB4669Cu, 300, 300> const LUT_Pythagoras{};
	static constexpr reference<short, 0xB45E68u, 496u> const MagneticBeamSineTable{};
	static constexpr reference<short, 0xB46254u, 500u> const SonicBeamSineTable{};
	static constexpr reference<int, 0xB46648u, 14u> const LUT_Linear1{};
	static constexpr reference<Matrix3D, 0xB45DA8u, 4u> const Matrix{};
	static constexpr reference<Matrix3D, 0xB45CA0u, 4u> const MagneticMatrix{};

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x75F7D0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x75F650);

	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//Destructor
	virtual ~WaveClass() RX;

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x75F610);
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;
	virtual void Update() override JMP_THIS(0x760F50);

	//ObjectClass
	virtual Layer InWhichLayer() const override JMP_THIS(0x75F890); // dont ask me ,.. -Otamaa

	// remove object from the map
	virtual bool Limbo() override JMP_THIS(0x75F980);

	// place the object on the map
	virtual bool Unlimbo(const CoordStruct& Crd, DirType dFaceDir) override JMP_THIS(0x75F8B0);
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const  override JMP_THIS(0x7F6D08);

	static void Generate_Tables()
		{ JMP_STD(0x75F020); }

	//=================================================================================
	void CalculateSonicBeam (int nA1, int nA2, int nA3, ColorStruct* pBuffer) const
		{ JMP_THIS(0x75EDF0); }

	void CalculateMagBeam(int nA1, int nA2, int nA3, ColorStruct* pBuffer) const
		{ JMP_THIS(0x760190); }

	void CalculateColorOverTo(ColorStruct* pBuffer ,int nDir) const
		{ JMP_THIS(0x75EF30); }

	void GenerateSurfacePitch() const
		{ JMP_THIS(0x75F140); }

	void DamageArea(const CoordStruct& location) const
		{ JMP_THIS(0x75F330); }

	void DamageArea(const CoordStruct* pLocation) const
		{ JMP_THIS(0x75F330); }

	void AddCell(const CoordStruct& location) const
		{ JMP_THIS(0x75F4C0); }

	void AddCell(const CoordStruct* pLocation) const
		{ JMP_THIS(0x75F4C0); }

	void DrawSonic(Point2D* From, RectangleStruct* To) const
		{ JMP_THIS(0x75FA90); }

	void DrawMagnetic(Point2D* From, RectangleStruct* To) const
		{ JMP_THIS(0x7602E0); }

	void DrawLaser(Point2D* From, RectangleStruct* To) const
		{ JMP_THIS(0x7609E0); }

	void AIAll() const
		{ JMP_THIS(0x760F50); }

	void WaveAI()const
		{ JMP_THIS(0x762AF0); }

	void CellAI() const
		{ JMP_THIS(0x7610F0); }

	// matrix ? or something else ?
	Matrix3D* DrawNonMag(const CoordStruct& xyzFrom, const CoordStruct& xyzTo)const
		{ JMP_THIS(0x761640); }

	Matrix3D* DrawMag(const CoordStruct& xyzFrom, const CoordStruct& xyzTo) const
		{ JMP_THIS(0x762070); }

	//Constructor
	WaveClass(
		const CoordStruct& From, const CoordStruct& To, TechnoClass *Owner,
		WaveType mode, AbstractClass *Target) : WaveClass(noinit_t())
	{ JMP_THIS(0x75E950); }

protected:
	explicit __forceinline WaveClass(noinit_t)
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	TechnoClass* Target;
	WaveType Type;
	DECLARE_PROPERTY(CoordStruct, LimboCoords);
	DECLARE_PROPERTY(CoordStruct, Pos0);
	DECLARE_PROPERTY(Point2D, InitialWavePixels_0);
	DECLARE_PROPERTY(Point2D, InitialWavePixels_1);
	DECLARE_PROPERTY(Point2D, InitialWavePixels_2);
	DECLARE_PROPERTY(Point2D, InitialWavePixels_3);
	DECLARE_PROPERTY(Point2D, InitialWavePixels_4);
	DECLARE_PROPERTY(Point2D, InitialWavePixels_5);
	DECLARE_PROPERTY(CoordStruct, Pos7);
	DECLARE_PROPERTY(CoordStruct, Pos8);
	DECLARE_PROPERTY(CoordStruct, Pos9);
	DECLARE_PROPERTY(CoordStruct, Pos10);
	bool bool_12C;
	bool bool_12D;
	int WaveIntensity;
	int WaveCount;
	double MatrixScale1;
	double MatrixScale2;
	DECLARE_PROPERTY(SingleArray_WavePoint2D, FirstPointData);
	DECLARE_PROPERTY(Point2D, ActiveWavePixels_0);
	DECLARE_PROPERTY(Point2D, ActiveWavePixels_1);
	DECLARE_PROPERTY(Point2D, ActiveWavePixels_2);
	DECLARE_PROPERTY(Point2D, ActiveWavePixels_3);
	DECLARE_PROPERTY(Point2D, ActiveWavePixels_4);
	DECLARE_PROPERTY(Point2D, ActiveWavePixels_5);
	DECLARE_PROPERTY(Point2D, UnusedWavePixels__0);
	DECLARE_PROPERTY(Point2D, UnusedWavePixels__1);
	DECLARE_PROPERTY(Point2D, UnusedWavePixels__2);
	DECLARE_PROPERTY(Point2D, UnusedWavePixels__3);
	DECLARE_PROPERTY(MultiDimentionalArray_WavePoint2D, SecondPointData);
	int EachFacing[8];
	FacingType FacingType;
	int LaserIntensity;
	TechnoClass* Owner;
	DECLARE_PROPERTY(FacingClass, Facing);
	DECLARE_PROPERTY(DynamicVectorClass<CellClass *>, Cells);
	int intensity_tables[14];
};

static_assert(sizeof(WaveClass) == 0x240, "Invalid size.");
