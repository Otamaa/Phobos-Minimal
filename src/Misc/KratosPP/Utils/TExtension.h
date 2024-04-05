#pragma once

#include "../Interfaces/IExtData.h"
#include <CCINIClass.h>

template <typename T>
class TExtension : public IExtData
{
public :
	enum class InitState
	{
		Blank = 0x0,	  // CTOR'd
		Constanted = 0x1, // values that can be set without looking at Rules (i.e. country default loadscreen)
		Ruled = 0x2,	  // Rules has been loaded and props set (i.e. country powerplants taken from [General])
		Inited = 0x3,	  // values that need the object's state (i.e. is object a secretlab? -> load default boons)
		Completed = 0x4	  // INI has been read and values set
	};

private:

	T* AttachedToObject;
	InitState Initialized;

public:


	static const DWORD Canary;

	TExtension(T* const OwnerObject) : AttachedToObject { OwnerObject }, Initialized { InitState::Blank }
	{ }

	TExtension(const TExtension& other) = delete;

	void operator=(const TExtension& RHS) = delete;

	virtual ~TExtension() = default;

	// the object this Extension expands
	T* const& OwnerObject() const
	{
		return this->AttachedToObject;
	}

	void EnsureConstanted()
	{
		if (this->Initialized < InitState::Constanted)
		{
			this->InitializeConstants();
			this->Initialized = InitState::Constanted;
		}
	}

	void LoadFromINI(CCINIClass* pINI)
	{
		if (!pINI)
			return;

		switch (this->Initialized)
		{
		case InitState::Blank:
			this->EnsureConstanted();
		case InitState::Constanted:
			this->InitializeRuled();
			this->Initialized = InitState::Ruled;
		case InitState::Ruled:
			this->Initialize();
			this->Initialized = InitState::Inited;
		case InitState::Inited:
		case InitState::Completed:
			if (pINI == CCINIClass::INI_Rules)
				this->LoadFromRulesFile(pINI);

			this->LoadFromINIFile(pINI);
			this->Initialized = InitState::Completed;
		}
	}

	virtual void InvalidatePointer(void* ptr) { };
	virtual void Detach(void* ptr, bool all) { };

	virtual inline void SaveToStream(PhobosStreamWriter& Stm)
	{
		//Stm.Save(this->AttachedToObject);
		Stm.Save(this->Initialized);
	}

	virtual inline void LoadFromStream(PhobosStreamReader& Stm)
	{
		//Stm.Load(this->AttachedToObject);
		Stm.Load(this->Initialized);
	}

protected:
	// right after construction. only basic initialization tasks possible;
	// owner object is only partially constructed! do not use global state!
	virtual void InitializeConstants() { }

	virtual void InitializeRuled() { }

	// called before the first ini file is read
	virtual void Initialize() { }

	// for things that only logically work in rules - countries, sides, etc
	virtual void LoadFromRulesFile(CCINIClass* pINI) { }

	// load any ini file: rules, game mode, scenario or map
	virtual void LoadFromINIFile(CCINIClass* pINI) { }
};
