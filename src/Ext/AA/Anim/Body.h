#pragma once
#include <Ext/AA/Object/Body.h>
#include <TechnoClass.h>

class AnimClassExt : public AnimClass
{
public:
	static constexpr size_t Canary = 0xAAAAAAAA;
	using base_type = AnimClass;

	class ExtData : public ObjectClassExt::ExtData
	{
	public:

		ExtData(AnimClass* OwnerObject) : ObjectClassExt::ExtData(OwnerObject)
		{ }

		ExtData(AnimClassExt* OwnerObject) : ExtData(static_cast<AnimClass*>(OwnerObject))
		{ }

		~ExtData() = default;

		ObjectClassExt* GetObjectExt() const {
			return static_cast<ObjectClassExt*>(Get());
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override { }
		virtual void SaveToStream(PhobosStreamWriter& Stm) override { }
	};

	class ExtContainer final : public TExtensionContainer<AnimClassExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};

static_assert(sizeof(AnimClassExt) == sizeof(AnimClass), "Missmatch Size !");