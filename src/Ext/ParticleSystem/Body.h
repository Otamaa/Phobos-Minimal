#pragma once

#include <Utilities/PooledContainer.h>
#include <Utilities/TemplateDefB.h>

#include <ParticleSystemClass.h>

class ParticleClass;
class ParticleTypeClass;
class ParticleSystemExtData
{
public:
	static COMPILETIMEEVAL size_t Canary = 0xAAA2BBBB;
	using base_type = ParticleSystemClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	enum class Behave : int
	{
		None = 0,
		Spark = 1,
		Railgun = 2,
		Smoke = 3
	};

	Behave What { Behave::None };
	ParticleTypeClass* HeldType { nullptr };

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
	HelperedVector<Movement> OtherParticleData { };

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
	HelperedVector<Draw> SmokeData { };

	bool AlphaIsLightFlash { true };

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	void InitializeConstant();
	void UpdateLocations();
	void UpdateState();
	void UpdateColor();
	void UpdateSpark();
	void UpdateRailgun();
	void UpdateWindDirection();
	void UpdateSmoke();
	bool UpdateHandled();
	void UpdateInAir_Main(bool allowDraw);

	static void UpdateInAir();

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(ParticleSystemExtData) -
			(4u //AttachedToObject
				- 4u //inheritance
			);
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParticleSystemExtContainer final : public Container<ParticleSystemExtData>
{
public:
	static ParticleSystemExtContainer Instance;
	static ObjectPool<ParticleSystemExtData> pools;

	ParticleSystemExtData* AllocateUnchecked(ParticleSystemClass* key)
	{
		ParticleSystemExtData* val = pools.allocate();

		if (val)
		{
			val->AttachedToObject = key;
			if (!Phobos::Otamaa::DoingLoadGame)
				val->InitializeConstant();
		}
		else
		{
			Debug::FatalErrorAndExit("The amount of [ParticleSystemExtData] is exceeded the ObjectPool size %d !", pools.getPoolSize());
		}

		return val;
	}

	void Remove(ParticleSystemClass* key)
	{
		if (ParticleSystemExtData* Item = TryFind(key)) {
			RemoveExtOf(key, Item);
		}
	}

	void RemoveExtOf(ParticleSystemClass* key, ParticleSystemExtData* Item)
	{
		pools.deallocate(Item);
		this->ClearExtAttribute(key);
	}
};

class ParticleSystemTypeExtData;
class NOVTABLE FakeParticleSystemClass : public ParticleSystemClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	ParticleSystemExtData* _GetExtData() {
		return *reinterpret_cast<ParticleSystemExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	ParticleSystemTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<ParticleSystemTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeParticleSystemClass) == sizeof(ParticleSystemClass), "Invalid Size !");