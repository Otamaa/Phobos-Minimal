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
	static COMPILETIMEEVAL const char* ClassName = "ParticleSystemExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "ParticleSystemClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

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

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
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

	ParticleSystemClass* This() const { return reinterpret_cast<ParticleSystemClass*>(this->AttachedToObject); }
	const ParticleSystemClass* This_Const() const { return reinterpret_cast<const ParticleSystemClass*>(this->AttachedToObject); }

public:

	void UpdateLocations();
	void UpdateState();
	void UpdateColor();
	void UpdateSpark();
	void UpdateRailgun();
	void UpdateWindDirection();
	void UpdateSmoke();
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
	static COMPILETIMEEVAL const char* ClassName = "ParticleSystemExtContainer";

public:
	static ParticleSystemExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

};

class ParticleSystemTypeExtData;
class NOVTABLE FakeParticleSystemClass : public ParticleSystemClass
{
public:

	void __AI();
	void __Smoke_AI();
	void __Gas_AI();
	void __Fire_AI();
	void __Spark_AI();
	void __Railgun_AI();
	void __Web_AI();

	void UpdateAllParticlesFront();
	void UpdateAllParticlesBehind();
	void UpdateAndCoordAllParticles();
	void RemoveDeadParticles();

	template<auto Func>
	void ProcessParticleLifecycle();

	ParticleClass* CreateHoldsWhatParticle(const CoordStruct& position, const CoordStruct& target);

	// Spark
	bool ShouldSpawnThisFrame() const;
	int CalculateParticleCount() const;
	void ProcessSparkSpawning();
	void SpawnSparkParticle();
	void SetupRandomVelocity(ParticleClass* particle);
	void CreateSpotlightIfNeeded();
	void UpdateSpotlight();

	// Gas
	void TransitionToNextParticle(ParticleClass* oldParticle);

	// Smoke
	void UpdateSmokeAttachedPosition();
	void SpawnChildParticles(ParticleClass* parent);
	void SpawnChildParticle(ParticleClass* parent, ParticleTypeClass* childType,
					   int offsetX, int offsetY);
	void SpawnSmokeParticles();
	void UpdateSpawnTiming();

	// Fire
	bool UpdateAttachedPosition();
	void UpdatePositionFromOwner(TechnoClass* owner);
	void SpawnFireParticles(bool forceSpawn);

	// Railgun
	void CreateLaserBeam();
	float CalculateVelocityPerturbation(float progress) const;
	void SetupParticleVelocity(ParticleClass* particle, const Vector3D<float>& direction, float progress);
	CoordStruct CalculateSpawnPosition(float progress, const Vector3D<float>& spiralOffset);

	Vector3D<float> CalculateSpiralOffset(float progress, float distance, const Matrix3D& rotationMatrix);
	void SpawnSpiralParticle(int index, int totalCount, float distance, const Matrix3D& rotationMatrix);
	Vector3D<float> CalculateTrajectory() const;
	void CreateSpiralTrail();

	ParticleSystemExtData* _GetExtData() {
		return *reinterpret_cast<ParticleSystemExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	ParticleSystemTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<ParticleSystemTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeParticleSystemClass) == sizeof(ParticleSystemClass), "Invalid Size !");