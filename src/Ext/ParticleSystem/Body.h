#pragma once

#include <Utilities/Container.h>
#include <Utilities/TemplateDefB.h>

#include <ParticleSystemClass.h>

class ParticleClass;
class ParticleTypeClass;
class ParticleSystemExtData final
{
public:
	static constexpr size_t Canary = 0xAAA2BBBB;
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
				.Process(LinkedParticleType, true)
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

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(ParticleSystemExtData) -
			(4u //AttachedToObject
			 );
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParticleSystemExtContainer final : public Container<ParticleSystemExtData>
{
public:
	static std::vector<ParticleSystemExtData*> Pool;
	static ParticleSystemExtContainer Instance;

	ParticleSystemExtData* AllocateUnchecked(ParticleSystemClass* key)
	{
		ParticleSystemExtData* val = nullptr;
		if (!Pool.empty()) {
			val = Pool.front();
			Pool.erase(Pool.begin());
			//re-init
		} else {
			val = DLLAllocWithoutCTOR<ParticleSystemExtData>();
		}

		if (val) {
			val->ParticleSystemExtData::ParticleSystemExtData();
			val->AttachedToObject = key;
			val->InitializeConstant();
			return val;
		}

		return nullptr;
	}

	ParticleSystemExtData* Allocate(ParticleSystemClass* key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame)
			return nullptr;

		this->ClearExtAttribute(key);

		if (ParticleSystemExtData* val = AllocateUnchecked(key))
		{
			this->SetExtAttribute(key, val);
			return val;
		}

		return nullptr;
	}

	void Remove(ParticleSystemClass* key)
	{
		if (ParticleSystemExtData* Item = TryFind(key))
		{
			Item->~ParticleSystemExtData();
			Item->AttachedToObject = nullptr;
			Pool.push_back(Item);
			this->ClearExtAttribute(key);
		}
	}

	void Clear()
	{
		if (!Pool.empty())
		{
			auto ptr = Pool.front();
			Pool.erase(Pool.begin());
			if (ptr)
			{
				delete ptr;
			}
		}
	}

	//CONSTEXPR_NOCOPY_CLASSB(ParticleSystemExtContainer, ParticleSystemExtData, "ParticleSystemClass");
};