#pragma once

#include <AbstractClass.h>

#include <Utilities/PhobosFixedString.h>
#include <Utilities/Macro.h>
#include <Utilities/SavegameDef.h>

//#define READ_SAVE(what)\
//	pStm->Read(&what, sizeof(what), nullptr);

//#define WRITE_SAVE(what)\
//	pStm->Write(&what, sizeof(what), nullptr);

// All ext classes should derive from this class
class IExtension
{
public:

	IExtension() = default;
	virtual ~IExtension() = default;

	virtual size_t GetSize() const = 0;

	//these load exacly after AbstracClass::S/L called
	//suitable to re-set AttachedObject or anything that require early S/L iteration
	virtual HRESULT Load(IStream* pStm, AbstractClass* pThis) = 0;
	virtual HRESULT Save(IStream* pStm) = 0;

	//this one used for `Container` later to S/L after everything done
	//which using `Container` class as core infrastructures
	virtual inline void SaveToStream(PhobosStreamWriter& Stm) = 0;
	virtual inline void LoadFromStream(PhobosStreamReader& Stm) = 0;

	//real name `Detach`
	virtual void InvalidatePointer(void* ptr, bool bRemoved) = 0;
	// called after the Extension Constructed
	virtual void InitializeConstants() {}
	virtual void Uninitialize() {}

#define FAIL_CHECK(hr) if(FAILED(hr)) return hr;
};

// Derivered with attached object
template<typename T>
class TExtension : public IExtension
{
private:
	T* AttachedToObject;

public:
	static const DWORD Canary;

	TExtension(T* const OwnerObject) : IExtension { }
		, AttachedToObject{ OwnerObject }
	{ InitializeConstants(); }

	TExtension() : IExtension { }
		, AttachedToObject { nullptr }
	{ }

	TExtension(const TExtension&) = delete;
	void operator=(const TExtension&) = delete;

	virtual ~TExtension() override = default;

	inline T* const& OwnerObject() const {
		return this->AttachedToObject;
	}

	inline void SetOwnerObject(T* const& pOwner)
	{ AttachedToObject = pOwner; }

	inline void LoadFromINI(CCINIClass* pINI)
	{
		if (pINI)
		{
			this->Initialize();

			if (pINI == CCINIClass::INI_Rules)
				this->LoadFromRulesFile(pINI);

			this->LoadFromINIFile(pINI);
		}
	}

protected:

	// called before the first ini file is read
	virtual void Initialize() { }

	// for things that only logically work in rules - countries, sides, etc
	virtual void LoadFromRulesFile(CCINIClass* pINI) {}

	// load any ini file: rules, game mode, scenario or map
	virtual void LoadFromINIFile(CCINIClass* pINI) {}

	virtual HRESULT Load(IStream* pStm, AbstractClass* pThis) override {
		SetOwnerObject(static_cast<T*>(pThis));

		return S_OK;
	}

	virtual HRESULT Save(IStream* pStm) override { return S_OK; }
};

// This class is just a wrapper to replace
class ExtensionWrapper
{
public:
	ExtensionWrapper() :
		FlagDirty { FALSE }
		, ExtensionObject { nullptr }
	{ }

	~ExtensionWrapper() {
		this->DestoryExtensionObject();
	}

	size_t GetSize() const {

		if (this->ExtensionObject)
			return this->ExtensionObject->GetSize();

		return 0;
	}

	//replace bool Dirty -> Ext*
	inline static ExtensionWrapper*& GetWrapper(void* pThis) {
		return *reinterpret_cast<ExtensionWrapper**>((int)pThis + 0x20);
	}

	inline void DestoryExtensionObject() {
		if (this->ExtensionObject) {
			this->ExtensionObject->Uninitialize();
			GameDelete(this->ExtensionObject);
			this->ExtensionObject = nullptr;
		}
	}

	template<typename TExt , typename TObj>
	inline void CreateExtensionObject(TObj Obj) {

		this->DestoryExtensionObject();
		this->ExtensionObject = GameCreate<TExt>(Obj);

		//if(!this->ExtensionObject && Obj) {
		//	this->ExtensionObject = GameCreate<TExt>(Obj);
		//	this->ExtensionObject->InitializeConstants();
		//}
		//else {
		//	Debug::Log("Failed To Create Extension !\n");
		//}
	}

	HRESULT Load(IStream* pStm, AbstractClass* pThis) const
	{
		if (auto pExtData = this->ExtensionObject)
		{
			HRESULT hr = pStm->Read(pExtData, pExtData->GetSize(), nullptr);

			if (SUCCEEDED(hr))
				return pExtData->Load(pStm, pThis);

			return hr;
		}

		return S_OK;
	}

	HRESULT Save(IStream* pStm) const
	{
		if (auto pExtData = this->ExtensionObject)
		{
			HRESULT hr = pStm->Write(pExtData, pExtData->GetSize(), nullptr);

			if (SUCCEEDED(hr))
				return pExtData->Save(pStm);

			return hr;
		}

		return S_OK;
	}

	__declspec(noinline) bool IsDirty() const { //dont inline this , causing crash !
		return this->FlagDirty;
	};

	inline void SetDirtyFlag(bool fDirty) {
		this->FlagDirty = fDirty;
	}

private:
	bool FlagDirty;

public:
	IExtension* ExtensionObject;
};

template<typename T>
class TExtensionContainer
{
private:
	using base_type = typename T::base_type;
	using extension_type = typename T::ExtData;
	using base_type_ptr = base_type*;
	using const_base_type_ptr = const base_type*;
	using no_const_base_type_ptr = base_type*;
	using extension_type_ptr = extension_type*;

	base_type_ptr SavingObject;
	IStream* SavingStream;
	FixedString<0x100> Name;

public:
	explicit TExtensionContainer(const char* pName) :
		SavingObject { nullptr },
		SavingStream { nullptr },
		Name { pName }
	{}

	virtual ~TExtensionContainer() = default;

	inline auto size() const {
		return sizeof(extension_type);
	}

	inline auto GetName() const {
		return this->Name.data();
	}

	void PrepareStream(base_type_ptr key, IStream* pStm) {
		Debug::Log("[PrepareStream] Next is %p of type '%s'\n", key, this->Name.data());
		this->SavingObject = key;
		this->SavingStream = pStm;
	}

	inline IStream* GetStream() const {
		return this->SavingStream;
	}

	void SaveStatic() {
		if (this->SavingObject && this->SavingStream) {
			Debug::Log("[SaveStatic] Saving object %p as '%s'\n", this->SavingObject, this->Name.data());
			if (!this->Save(this->SavingObject, this->SavingStream))
				Debug::FatalErrorAndExit("[SaveStatic] Saving failed!\n");
		} else {
			Debug::Log("[SaveStatic] Object or Stream not set for '%s': %p, %p\n",
				this->Name.data(), this->SavingObject, this->SavingStream);
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
	}

	void LoadStatic() {
		if (this->SavingObject && this->SavingStream) {
			Debug::Log("[LoadStatic] Loading object %p as '%s'\n", this->SavingObject, this->Name.data());
			if (!this->Load(this->SavingObject, this->SavingStream))
				Debug::FatalErrorAndExit("[LoadStatic] Loading object %p as '%s failed!\n", this->SavingObject, this->Name.data());
		} else {
			Debug::Log("[LoadStatic] Object or Stream not set for '%s': %p, %p\n",
				this->Name.data(), this->SavingObject, this->SavingStream);
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
	}

protected:

	// override this method to do type-specific stuff
	virtual bool Save(base_type_ptr key, IStream* pStm) {
		return this->SaveKey(key, pStm) != nullptr;
	}

	// override this method to do type-specific stuff
	virtual bool Load(base_type_ptr key, IStream* pStm) {
		return this->LoadKey(key, pStm) != nullptr;
	}

	extension_type_ptr SaveKey(base_type_ptr key, IStream* pStm) {

		// this really shouldn't happen
		if (!key) {
			Debug::Log("[SaveKey] Attempted for a null pointer! WTF!\n");
			return nullptr;
		}

		// get the value data
		extension_type_ptr buffer = T::GetExtData(key);
		if (!buffer) {
			Debug::Log("[SaveKey] Could not find value.\n");
			return nullptr;
		}

		// write the current pointer, the size of the block, and the canary
		PhobosByteStream saver(buffer->GetSize());
		PhobosStreamWriter writer(saver);

		writer.Save(extension_type::Canary);
		//writer.Save(buffer);
		// save the data
		buffer->SaveToStream(writer);

		// save the block
		if (!saver.WriteBlockToStream(pStm)) {
			Debug::Log("[SaveKey] Failed to save data.\n");
			return nullptr;
		}

		Debug::Log("[SaveKey] Save used up 0x%X bytes\n", saver.Size());

		return buffer;
	}

	extension_type_ptr LoadKey(base_type_ptr key, IStream* pStm) {

		// this really shouldn't happen
		if (!key) {
			Debug::Log("[LoadKey] Attempted for a null pointer! WTF!\n");
			return nullptr;
		}

		// get the value data
		extension_type_ptr buffer = T::GetExtData(key);
		if (!buffer) {
			Debug::Log("[LoadKey] Could not find or allocate value.\n");
			return nullptr;
		}

		PhobosByteStream loader(0);
		if (!loader.ReadBlockFromStream(pStm)) {
			Debug::Log("[LoadKey] Failed to read data from save stream?!\n");
			return nullptr;
		}

		PhobosStreamReader reader(loader);
		if (reader.Expect(extension_type::Canary) //&& reader.RegisterChange(buffer)
			) {
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
