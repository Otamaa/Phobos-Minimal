#pragma once

#include <AbstractClass.h>

#include <Utilities/Container.h>
#include <Utilities/PhobosFixedString.h>
#include <Utilities/Macro.h>
#include <Utilities/SavegameDef.h>

static constexpr size_t AbstractExtOffset = 0x18;

// All ext classes should derive from this class
// Derivered with attached object
template<typename T>
class TExtension : public IExtension
{
private:
	T* AttachedToObject;

public:

	TExtension(T* const OwnerObject) : IExtension { }
		, AttachedToObject { OwnerObject }
	{ 
		//WhatIAm = OwnerObject->WhatAmI();
		//Debug::Log("Alloc Ext For %s ! \n", AbstractClass::GetAbstractClassName(WhatIAm));
	}

	TExtension() : IExtension { }
		, AttachedToObject { nullptr }
		//, WhatIAm { AbstractType::Abstract }
	{ }

	TExtension(const TExtension&) = delete;
	void operator=(const TExtension&) = delete;

	virtual ~TExtension() = default;

	T* const& Get() const
	{
		return this->AttachedToObject;
	}

	void SetOwnerObject(T* const& pOwner)
	{ AttachedToObject = pOwner; }

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

	void EnsureConstanted()
	{
		if (this->Initialized < InitState::Constanted)
		{
			this->InitializeConstants();
			this->Initialized = InitState::Constanted;
		}
	}

	// overrideable virtuals !
	virtual void InvalidatePointer(void* ptr, bool bRemoved) { }

	virtual inline void SaveToStream(PhobosStreamWriter& Stm) { 
		Stm.Save(this->Initialized);
		//Stm.Save(this->WhatIAm);
	}

	virtual inline void LoadFromStream(PhobosStreamReader& Stm) {
		Stm.Load(this->Initialized);
		//Stm.Load(this->WhatIAm);
	}

	static constexpr AbstractType WhatIam = T::AbsID;
	//virtual size_t GetSize() const { return sizeof(*this); }

protected:	// overrideable virtuals !

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

template<typename T>
class TExtensionContainer
{
private:
	using base_type = typename T::base_type;
	using extension_type = typename T::ExtData;
	using base_type_ptr = base_type*;
	using const_base_type_ptr = const base_type*;
	using extension_type_ptr = extension_type*;
	using const_extension_type_ptr = const extension_type*;

	base_type_ptr SavingObject;
	IStream* SavingStream;
	FixedString<0x100> Name;

public:
	explicit TExtensionContainer(const char* pName) :
		SavingObject { nullptr },
		SavingStream { nullptr },
		Name { }
	{
		Name = pName;
	}

	virtual ~TExtensionContainer() = default;

	inline auto size() const
	{
		return sizeof(extension_type);
	}

	inline auto GetName() const
	{
		return this->Name.data();
	}

	inline auto GetSavingObject() const
	{
		return SavingObject;
	}

	void PrepareStream(base_type_ptr key, IStream* pStm)
	{
		Debug::Log("[PrepareStream] Next is %p of type '%s'\n", key, this->Name.data());
		this->SavingObject = key;
		this->SavingStream = pStm;
	}

	inline IStream* GetStream() const
	{
		return this->SavingStream;
	}

	extension_type_ptr SetIExtension(base_type_ptr key)
	{
		(*(uintptr_t*)((char*)key + AbstractExtOffset)) = 0;

		if (const auto val = new extension_type(key))
		{
			val->EnsureConstanted();
			(*(uintptr_t*)((char*)key + AbstractExtOffset)) = (uintptr_t)val;
		}

		return (extension_type_ptr)(*(uintptr_t*)((char*)key + AbstractExtOffset));
	}

	extension_type_ptr Allocate(const_base_type_ptr key)
	{
		return SetIExtension(key);
	}

	void inline Remove(base_type_ptr key)
	{
		if (auto Item = (extension_type_ptr)(*(uintptr_t*)((char*)key + AbstractExtOffset)))
		{
			delete Item;
			(*(uintptr_t*)((char*)key + AbstractExtOffset)) = 0;
		}
	}

	template<bool Check = false>
	extension_type_ptr GetOrSetIExtension(base_type_ptr key)
	{
		if (auto pExt = GetIExtension<Check>(key))
			return pExt;

		return SetIExtension(key);
	}

	template<bool Check = false>
	extension_type_ptr FindOrAllocate(base_type_ptr key)
	{
		return GetOrSetIExtension<Check>(key);
	}

	template<bool Check = false>
	NOINLINE extension_type_ptr GetIExtension(const_base_type_ptr key)
	{
		static_assert(extension_type::WhatIam == base_type::AbsID);

		if constexpr (Check)
		{
			if (!key)
				return nullptr;
		}

		return (extension_type_ptr)(*(uintptr_t*)((char*)key + AbstractExtOffset));
	}

	template<bool Check = false>
	NOINLINE extension_type_ptr Find(const_base_type_ptr key) const
	{
		static_assert(extension_type::WhatIam == base_type::AbsID);

		if constexpr (Check)
		{
			if (!key)
				return nullptr;
		}

		return (extension_type_ptr)(*(uintptr_t*)((char*)key + AbstractExtOffset));
	}

	void JustAllocate(base_type_ptr key, bool bCond, const std::string_view& nMessage)
	{
		if (!key || (!bCond && !nMessage.empty()))
		{
			Debug::Log("%s \n", nMessage.data());
			return;
		}

		SetIExtension(key);
	}

	void LoadFromINI(const_base_type_ptr key, CCINIClass* pINI)
	{
		if (auto ptr = GetIExtension<true>(key))
			ptr->LoadFromINI(pINI);
	}

	void SaveStatic()
	{
		if (this->SavingObject && this->SavingStream)
		{
			Debug::Log("[SaveStatic] Saving object %p as '%s'\n", this->SavingObject, this->Name.data());
			if (!this->Save(this->SavingObject, this->SavingStream))
				Debug::FatalErrorAndExit("[SaveStatic] Saving failed!\n");
		}
		else
		{
			Debug::Log("[SaveStatic] Object or Stream not set for '%s': %p, %p\n",
				this->Name.data(), this->SavingObject, this->SavingStream);
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
	}

	void LoadStatic()
	{
		if (this->SavingObject && this->SavingStream)
		{
			Debug::Log("[LoadStatic] Loading object %p as '%s'\n", this->SavingObject, this->Name.data());
			if (!this->Load(this->SavingObject, this->SavingStream))
				Debug::FatalErrorAndExit("[LoadStatic] Loading object %p as '%s failed!\n", this->SavingObject, this->Name.data());
		}
		else
		{
			Debug::Log("[LoadStatic] Object or Stream not set for '%s': %p, %p\n",
				this->Name.data(), this->SavingObject, this->SavingStream);
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
	}

	virtual void InvalidatePointer(void* ptr, bool bRemoved) { };
	virtual void Clear() { }

	void PointerGotInvalid(void* ptr, bool bRemoved) {
		this->InvalidatePointer(ptr, bRemoved);
	}

protected:

	// override this method to do type-specific stuff
	virtual bool Save(base_type_ptr key, IStream* pStm)
	{
		return this->SaveKey(key, pStm) != nullptr;
	}

	// override this method to do type-specific stuff
	virtual bool Load(base_type_ptr key, IStream* pStm)
	{
		return this->LoadKey(key, pStm) != nullptr;
	}

	extension_type_ptr SaveKey(base_type_ptr key, IStream* pStm)
	{

		// this really shouldn't happen
		if (!key)
		{
			Debug::Log("[SaveKey] Attempted for a null pointer! WTF!\n");
			return nullptr;
		}

		// get the ext data
		extension_type_ptr buffer = GetIExtension(key);
		if (!buffer)
		{
			Debug::Log("[SaveKey] Could not find value.\n");
			return nullptr;
		}

		// write the current pointer, the size of the block, and the canary
		PhobosByteStream saver(sizeof(extension_type));
		PhobosStreamWriter writer(saver);

		writer.Save(T::Canary);
		writer.Save(buffer);

		// save the data
		buffer->SaveToStream(writer);

		// save the block
		if (!saver.WriteBlockToStream(pStm))
		{
			Debug::Log("[SaveKey] Failed to save data.\n");
			return nullptr;
		}

		Debug::Log("[SaveKey] Save used up 0x%X bytes\n", saver.Size());

		return buffer;
	}

	extension_type_ptr LoadKey(base_type_ptr key, IStream* pStm)
	{

		// this really shouldn't happen
		if (!key)
		{
			Debug::Log("[LoadKey] Attempted for a null pointer! WTF!\n");
			return nullptr;
		}

		// get the extData
		extension_type_ptr buffer = SetIExtension(key);

		if (!buffer)
		{
			Debug::Log("[LoadKey] Could not find or allocate value.\n");
			return nullptr;
		}

		PhobosByteStream loader(0);
		if (!loader.ReadBlockFromStream(pStm))
		{
			Debug::Log("[LoadKey] Failed to read data from save stream?!\n");
			return nullptr;
		}

		PhobosStreamReader reader(loader);
		if (reader.Expect(T::Canary) && reader.RegisterChange(buffer))
		{
			buffer->LoadFromStream(reader);
			if (reader.ExpectEndOfBlock())
				return buffer;
		}

		return nullptr;
	}

private:
	TExtensionContainer(const TExtensionContainer&) = delete;
	TExtensionContainer& operator = (const TExtensionContainer&) = delete;
	TExtensionContainer& operator = (TExtensionContainer&&) = delete;
};
