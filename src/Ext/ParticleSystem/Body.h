#pragma once

#include <Utilities/PooledContainer.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/VectorHelper.h>

#include <ParticleSystemClass.h>

#include <Ext/Object/Body.h>

class ParticleSystemClass;
class ParticleTypeClass;
class ParticleSystemExtData final : public ObjectExtData
{
public:

	using base_type = ParticleSystemClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:
#pragma region ClassMembers
	enum class Behave : int
	{
		None = 0,
		Spark = 1,
		Railgun = 2,
		Smoke = 3
	};
	Behave What;
	ParticleTypeClass* HeldType;
	//everything else use this
	struct Movement
	{
		Vector3D<float> vel;
		Vector3D<float> velB;
		float A;
		float ColorFactor;
		int C; //counter for the color change update
		int RemainingEC;
		BYTE Empty; //state counter
		ColorStruct Colors;
		bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return Serialize(Stm);
		}
		bool Save(PhobosStreamWriter& Stm) const
		{
			return const_cast<Movement*>(this)->Serialize(Stm);
		}
	private:
		template <typename T>
		bool Serialize(T& Stm)
		{
			return Stm
				.Process(vel)
				.Process(velB)
				.Process(A)
				.Process(ColorFactor)
				.Process(C)
				.Process(RemainingEC)
				.Process(Empty)
				.Process(Colors)
				.Success()
				//&& Stm.RegisterChange(this)
				;
		}
	};
	static_assert(sizeof(Movement) == 0x2C, "Invalid Size");
	HelperedVector<Movement> OtherParticleData;
	//used for smoke state
	struct Draw
	{
		CoordStruct vel; //0 4 8
		Vector3D<float> velB; //C 10 14
		int StateAdvance;//18
		int ImageFrame; //1C
		int RemainingEC; //20
		ParticleTypeClass* LinkedParticleType; //24
		BYTE Translucency; //28
		BYTE DeleteOnStateLimit; //state counter //29
		BYTE byte30; //2A
		BYTE byte31;//2B
		bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
		{
			return Serialize(Stm);
		}
		bool Save(PhobosStreamWriter& Stm) const
		{
			return const_cast<Draw*>(this)->Serialize(Stm);
		}
	private:
		template <typename T>
		bool Serialize(T& Stm)
		{
			return Stm
				.Process(vel)
				.Process(velB)
				.Process(StateAdvance)
				.Process(ImageFrame)
				.Process(RemainingEC)
				.Process(LinkedParticleType)
				.Process(Translucency)
				.Process(DeleteOnStateLimit)
				.Process(byte30)
				.Process(byte31)
				.Success()
				//&& Stm.RegisterChange(this)
				;
		}
	};
	static_assert(sizeof(Draw) == 0x2C, "Invalid Size");
	HelperedVector<Draw> SmokeData;
	bool AlphaIsLightFlash;
#pragma endregion

public:

	ParticleSystemExtData(ParticleSystemClass* pObj);
	ParticleSystemExtData(ParticleSystemClass* pObj, noinit_t nn) : ObjectExtData(pObj, nn) { }

	virtual ~ParticleSystemExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<ParticleSystemExtData*>(this)->ObjectExtData::SaveToStream(Stm);
		const_cast<ParticleSystemExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectExtData::CalculateCRC(crc);
	}

	virtual ParticleSystemClass* This() const override { return reinterpret_cast<ParticleSystemClass*>(this->ObjectExtData::This()); }
	virtual const ParticleSystemClass* This_Const() const override { return reinterpret_cast<const ParticleSystemClass*>(this->ObjectExtData::This_Const()); }

public:

	void UpdateLocations();
	void UpdateState();
	void UpdateColor();
	void UpdateSpark();
	void UpdateRailgun();
	void UpdateWindDirection();
	void UpdateSmoke();
	bool UpdateHandled();
	void UpdateInAir_Main(bool allowDraw);

public:

	static void UpdateInAir();

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParticleSystemExtContainer final : public Container<ParticleSystemExtData>
{
public:
	static ParticleSystemExtContainer Instance;
	static void Clear()
	{
		Array.clear();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}
};

class ParticleSystemTypeExtData;
class NOVTABLE FakeParticleSystemClass : public ParticleSystemClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	ParticleSystemExtData* _GetExtData() {
		return *reinterpret_cast<ParticleSystemExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	ParticleSystemTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<ParticleSystemTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeParticleSystemClass) == sizeof(ParticleSystemClass), "Invalid Size !");