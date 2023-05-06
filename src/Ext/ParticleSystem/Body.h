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
	class ExtData final : public Extension<ParticleSystemClass>
	{
	public:
		static constexpr size_t Canary = 0xAAA2BBBB;
		using base_type = ParticleSystemClass;

	public:

		BehavesLike Behave;
		std::vector<ParticleDatas> PreCalculatedParticlesData;
		std::vector<ParticleDatas> SomeArray_b;
		ParticleTypeClass* AdditionalHeldType;
		ExtData(ParticleSystemClass* OwnerObject) : Extension<base_type> { OwnerObject }
			, Behave { BehavesLike::Smoke }
			, PreCalculatedParticlesData { }
			, SomeArray_b { }
			, AdditionalHeldType { nullptr }
		{ }

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