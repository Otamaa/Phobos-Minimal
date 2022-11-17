#pragma once

#include <AbstractClass.h>
#include <Utilities/Savegame.h>
#include <Ext/AA/Utilities/ExtHelpers.h>

class CCINIClass;

template <typename T>
concept IsIExtension = std::is_base_of<IExntension, T>::value;

template<typename T>
class ExtensionWrapperAbract
{
private:
	using base_type = typename T::base_type;
	using extension_type = typename T::ExtData;
	using base_type_ptr = base_type*;
	using const_base_type_ptr = const base_type*;
	using extension_type_ptr = extension_type*;
	using const_extension_type_ptr = const extension_type*;

	FixedString<0x100> Name;

public:
	explicit ExtensionWrapperAbract(const char* pName) :
		Name {}
	{
		Name = pName;
	}

	virtual ~ExtensionWrapperAbract() = default;

	inline auto GetName() const
	{
		return this->Name.data();
	}

	template<bool CheckKey = false>
	extension_type_ptr FindOrAllocate(base_type_ptr key)
	{
		if constexpr (CheckKey)
		{
			if (!key)
			{
				Debug::Log("Trying to Create Ext From nullptr ! \n");
				return nullptr;
			}
		}

		if (key->unknown_18)
		{
			Debug::Log("[%x][%s] Trying to Create Ext from key that Already have one [%x] !\n", key, Name.data(), key->unknown_18);
			return reinterpret_cast<extension_type_ptr>(key->unknown_18);
		}

		auto pAlloc = new extension_type(key);
		pAlloc->EnsureConstanted();

		key->unknown_18 = static_cast<IExntension*>(pAlloc);
		return pAlloc;
	}

	template<bool check = false>
	void Remove(base_type_ptr key)
	{
		if constexpr (check)
		{
			if (!key)
			{
				Debug::Log("Failed To Remove %s Extension witn nullptr key ! ", Name.data());
				return;
			}
		}

		if (key->unknown_18)
		{
			delete key->unknown_18;
			key->unknown_18 = nullptr;
		}
	}

	template<bool Check = false>
	extension_type_ptr Find(const_base_type_ptr key) const
	{
		if constexpr (Check)
		{
			if (!key)
			{
				return nullptr;
			}
		}

		return reinterpret_cast<extension_type_ptr>(key->unknown_18);
	}

	void LoadFromINI(const_base_type_ptr key, CCINIClass* pINI)
	{
		if (auto ptr = this->Find<true>(key))
			ptr->LoadFromINI(pINI);
	}

	virtual void PointerGotInvalid(void* ptr, bool bRemoved) {}

private:
	ExtensionWrapperAbract(const ExtensionWrapperAbract&) = delete;
	ExtensionWrapperAbract& operator = (const ExtensionWrapperAbract&) = delete;
	ExtensionWrapperAbract& operator = (ExtensionWrapperAbract&&) = delete;
};

template<typename T>
class ExtensionWrapperAbractSaveLoad : public ExtensionWrapperAbract<T>
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

public:
	explicit ExtensionWrapperAbractSaveLoad(const char* pName) : ExtensionWrapperAbract<T> { pName }
		SavingObject { nullptr },
		SavingStream { nullptr },
	{ }

	virtual ~ExtensionWrapperAbractSaveLoad() = default;

	inline base_type_ptr GetSavingObject() const
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

	virtual void PointerGotInvalid(void* ptr, bool bRemoved) override
	{
		ExtensionWrapperAbract<T>::InvalidatePointer(ptr, bRemoved);
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
		// get the ext data
		extension_type_ptr buffer = this->Find<true>(key);

		if (!buffer)
		{
			Debug::Log("[SaveKey] Could not find value.\n");
			return nullptr;
		}

		// write the current pointer, the size of the block, and the canary
		AresByteStream saver(sizeof(extension_type));
		AresStreamWriter writer(saver);

		writer.Save(T::Canary);
		//writer.Save(buffer);
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
		// get the extData
		extension_type_ptr buffer = this->FindOrAllocate(key);
		if (!buffer)
		{
			Debug::Log("[LoadKey] Could not find or allocate value.\n");
			return nullptr;
		}

		AresByteStream loader(0);
		if (!loader.ReadBlockFromStream(pStm))
		{
			Debug::Log("[LoadKey] Failed to read data from save stream?!\n");
			return nullptr;
		}

		AresStreamReader reader(loader);
		if (reader.Expect(T::Canary))
		{
			buffer->LoadFromStream(reader);
			if (reader.ExpectEndOfBlock())
				return buffer;
		}

		return nullptr;
	}

private:
	ExtensionWrapperAbractSaveLoad(const ExtensionWrapperAbractSaveLoad&) = delete;
	ExtensionWrapperAbractSaveLoad& operator = (const ExtensionWrapperAbractSaveLoad&) = delete;
	ExtensionWrapperAbractSaveLoad& operator = (ExtensionWrapperAbractSaveLoad&&) = delete;
};

class AbstractExt : public AbstractClass
{
public:
	using base_type = AbstractClass;

	class ExtData : public Extension<AbstractClass>, public IExntension
	{
	public:
		AbstractType ExtType;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override { }
		virtual void LoadFromStream(PhobosStreamReader& Stm)  override { }

	protected:

		bool STDMETHODCALLTYPE Internal_Load(PhobosStreamReader& Stm)
		{
			Extension<AbstractClass>::Serialize(Stm);
			return Stm.Load(this->ExtType);
		}

		bool STDMETHODCALLTYPE Internal_Save(PhobosStreamWriter& Stm)
		{
			Extension<AbstractClass>::Serialize(Stm);
			Stm.Save(this->ExtType);
			return true;
		}

	public:
		ExtData(const AbstractClass* this_ptr) : Extension<AbstractClass>(this_ptr)
			, ExtType { AbstractType::Abstract } {
			ExtType = this_ptr->WhatAmI();
		}

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* target, bool all = true) override { }
		virtual AbstractClass* OwnerObject() const { return const_cast<AbstractClass*>(Extension<AbstractClass>::OwnerObject()); }
		AbstractType What_Am_I() const { return ExtType; }

	private:
		//const AbstractClass* AttachedToObject;

		ExtData(const ExtData&) = delete;
		void operator = (const ExtData&) = delete;

	public:

		// right after construction. only basic initialization tasks possible;
		// owner object is only partially constructed! do not use global state!
		virtual void InitializeConstants() override { }

		virtual void InitializeRuled() override { }

		// called before the first ini file is read
		virtual void Initialize() override { }

		// for things that only logically work in rules - countries, sides, etc
		virtual void LoadFromRulesFile(CCINIClass* pINI) override { }

		// load any ini file: rules, game mode, scenario or map
		virtual void LoadFromINIFile(CCINIClass* pINI) override { }
	};

	class ExtContainer final : public ExtensionWrapperAbract<AbstractExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};

static_assert(sizeof(AbstractExt) == sizeof(AbstractClass), "Missmatch Size !");
