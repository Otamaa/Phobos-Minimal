#pragma once

enum class InitState
{
	Blank = 0x0, // CTOR'd
	Constanted = 0x1, // values that can be set without looking at Rules (i.e. country default loadscreen)
	Ruled = 0x2, // Rules has been loaded and props set (i.e. country powerplants taken from [General])
	Inited = 0x3, // values that need the object's state (i.e. is object a secretlab? -> load default boons)
	Completed = 0x4 // INI has been read and values set
};

template <typename T>
class Extension
{
	const T* AttachedToObject;
	InitState Initialized;

public:

	Extension(const T* OwnerObject) : AttachedToObject { OwnerObject }
		, Initialized { InitState::Blank }
	{}

	Extension(const Extension& other) = delete;

	void operator=(const Extension& RHS) = delete;

	virtual ~Extension() = default;

	// the object this Extension expands
	T* const OwnerObject() const
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
			[[fallthrough]];
		case InitState::Constanted:
			this->InitializeRuled();
			this->Initialized = InitState::Ruled;
			[[fallthrough]];
		case InitState::Ruled:
			this->Initialize();
			this->Initialized = InitState::Inited;
			[[fallthrough]];
		case InitState::Inited:
		case InitState::Completed:

		{
			if (pINI == CCINIClass::INI_Rules)
				this->LoadFromRulesFile(pINI);
		}
		this->LoadFromINIFile(pINI);
		this->Initialized = InitState::Completed;
		}
	}

	virtual void InvalidatePointer(void* ptr, bool bRemoved) = 0;
	virtual inline void SaveToStream(PhobosStreamWriter& Stm) = 0;
	virtual inline void LoadFromStream(PhobosStreamReader& Stm) = 0;

	template<typename StmType>
	inline void Serialize(StmType& Stm) { static_assert(true, "Please Implement Specific function for this !"); }

	template<>
	inline void Serialize(PhobosStreamWriter& Stm)
	{
		Stm.Save(this->Initialized);
	}

	template<>
	inline void Serialize(PhobosStreamReader& Stm)
	{
		Stm.Load(this->Initialized);
	}

protected:

	// right after construction. only basic initialization tasks possible;
	// owner object is only partially constructed! do not use global state!
	virtual void InitializeConstants() = 0;

	virtual void InitializeRuled() = 0;

	// called before the first ini file is read
	virtual void Initialize() = 0;

	// for things that only logically work in rules - countries, sides, etc
	virtual void LoadFromRulesFile(CCINIClass* pINI) = 0;

	// load any ini file: rules, game mode, scenario or map
	virtual void LoadFromINIFile(CCINIClass* pINI) = 0;

};


//ExtensionWrapper
//ExtensionContainerWrapper
//ExtensionGlobalWrapper
