#pragma once

#include <Ext/Abstract/Body.h>

#include <ParticleSystemClass.h>

struct ParticleDatas
{
	Vector3D<float> vel;
	Vector3D<float> velB;
	float A;
	float B;
	float C;
	int MaxEC;
	BYTE Empty;
	ColorStruct Colors;
};

class ParticleClass;
class ParticleTypeClass;
class ParticleSystemExt
{
public:

	enum class Behave : int
	{
		None = 0,
		Spark = 1,
		Railgun = 2,
		Smoke = 3
	};

	class ExtData final : public Extension<ParticleSystemClass>
	{
	public:
		static constexpr size_t Canary = 0xAAA2BBBB;
		using base_type = ParticleSystemClass;

	public:
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
					;
			}
		};
		static_assert(sizeof(Draw) == 0x2C, "Invalid Size");
		HelperedVector<Draw> SmokeData { };

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

		ExtData(ParticleSystemClass* OwnerObject) : Extension<base_type> { OwnerObject }
		{
			this->InitializeConstants();
		}

		virtual ~ExtData() override = default;
		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }
		void InitializeConstants();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<ParticleSystemExt::ExtData>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

};