#pragma once

#include <CCINIClass.h>
#include <SwizzleManagerClass.h>
#include <Memory.h>

#include "Debug.h"
#include "Stream.h"
#include "Swizzle.h"

#include <string>

class AbstractClass;
static constexpr size_t AbstractExtOffset = 0x18;

template <class T>
concept HasAbsID = requires(T) { T::AbsID; };

template <class T>
concept HasDeriveredAbsID = requires(T) { T::AbsDerivateID; };

template <class T>
concept HasTypeBase = requires(T) { T::AbsTypeBase; };

template <typename T>
concept Initable = requires(T t) { t.Initialize(); };

template <typename T>
concept CanLoadFromINIFile =
	requires (T t , CCINIClass* pINI, bool parseFailAddr) { t.LoadFromINIFile(pINI, parseFailAddr); };

template <typename T>
concept CanLoadFromRulesFile =
	requires (T t, CCINIClass * pINI) { t.LoadFromRulesFile(pINI); };

template <typename T>
concept CTORInitable =
	requires (T t) { t.InitializeConstant(); };


template <typename T>
concept PointerInvalidationSubscribable =
	requires (AbstractClass* ptr, bool removed) { T::InvalidatePointer(ptr, removed); };

template <typename T>
concept ThisPointerInvalidationSubscribable =
	requires (T t, AbstractClass* ptr, bool removed) { t.InvalidatePointer(ptr, removed); };

template <typename T>
concept PointerInvalidationIgnorAble =
	requires (AbstractClass* ptr) { T::InvalidateIgnorable(ptr); };

template <typename T>
concept CanLoadFromStream =
	requires (PhobosStreamReader & stm) { T::LoadFromStream(stm); };

template <typename T>
concept CanSaveToStream =
	requires (PhobosStreamWriter & stm) { T::SaveToStream(stm); };

template <typename T>
concept CanThisLoadFromStream =
	requires (T t, PhobosStreamReader & stm) { t.LoadFromStream(stm); };

template <typename T>
concept CanThisSaveToStream =
	requires (T t, PhobosStreamWriter & stm) { t.SaveToStream(stm); };

template <typename T>
concept CanThisPtrLoadFromStream =
	requires (T t, PhobosStreamReader & stm) { t->LoadFromStream(stm); };

template <typename T>
concept CanThisPtrSaveToStream =
	requires (T t, PhobosStreamWriter & stm) { t->SaveToStream(stm); };

template <class T>
concept HasOffset = requires(T) { T::ExtOffset; };

template <typename T>
class Container
{
private:
	using base_type = typename T::base_type;
	using extension_type = typename T;
	using base_type_ptr = base_type*;
	using const_base_type_ptr = const base_type*;
	using extension_type_ptr = extension_type*;
	using extension_type_ref_ptr = extension_type**;
	using const_extension_type_ptr = const extension_type*;

	base_type_ptr SavingObject { nullptr };
	IStream* SavingStream { nullptr };
	//std::string Name;

public:

	//constexpr explicit Container(const char* pName) : SavingObject { nullptr }
	//	, SavingStream { nullptr }
	//	, Name { pName }
	//{
	//}

	/*virtual ~Container() = default;*/

	//constexpr inline auto GetName() const
	//{
	//	return this->Name.data();
	//}

	constexpr inline base_type_ptr GetSavingObject() const
	{
		return SavingObject;
	}

	constexpr inline IStream* GetStream() const
	{
		return this->SavingStream;
	}

	constexpr FORCEINLINE void ClearExtAttribute(base_type_ptr key)
	{
		if constexpr (HasOffset<T>)
			(*(uintptr_t*)((char*)key + T::ExtOffset)) = 0;
		else
			(*(uintptr_t*)((char*)key + AbstractExtOffset)) = 0;
	}

	constexpr FORCEINLINE void SetExtAttribute(base_type_ptr key, extension_type_ptr val)
	{
		if constexpr (HasOffset<T>)
			(*(uintptr_t*)((char*)key + T::ExtOffset)) = (uintptr_t)val;
		else
			(*(uintptr_t*)((char*)key + AbstractExtOffset)) = (uintptr_t)val;
	}

	constexpr FORCEINLINE extension_type_ptr GetExtAttribute(base_type_ptr key)
	{
		if constexpr (HasOffset<T>)
			return (extension_type_ptr)(*(uintptr_t*)((char*)key + T::ExtOffset));
		else
			return (extension_type_ptr)(*(uintptr_t*)((char*)key + AbstractExtOffset));
	}

	// Allocate extensionptr without any checking
	extension_type_ptr AllocateUnchecked(base_type_ptr key)
	{
		// avoiding stupid memset ,......
		if (extension_type_ptr val = DLLCreate<extension_type>()) {

			val->AttachedToObject = key;
			if constexpr (CTORInitable<T>) {
				if(!Phobos::Otamaa::DoingLoadGame)
					val->InitializeConstant();
			}

			return val;
		}

		return nullptr;
	}

	extension_type_ptr Allocate(base_type_ptr key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame)
			return nullptr;

		this->ClearExtAttribute(key);

		if (extension_type_ptr val = AllocateUnchecked(key))
		{
			this->SetExtAttribute(key, val);
			return val;
		}

		return nullptr;
	}

	extension_type_ptr FindOrAllocate(base_type_ptr key)
	{
		// Find Always check for nullptr here
		if (extension_type_ptr const ptr = TryFind(key))
			return ptr;

		return this->Allocate(key);
	}

	constexpr extension_type_ptr Find(base_type_ptr key)
	{
		return this->GetExtAttribute(key);
	}

	constexpr extension_type_ptr TryFind(base_type_ptr key)
	{
		if (!key)
			return nullptr;

		return this->GetExtAttribute(key);
	}

	void Remove(base_type_ptr key)
	{
		if (extension_type_ptr Item = TryFind(key))
		{
			DLLCallDTOR<false>(Item);
			this->ClearExtAttribute(key);
		}
	}

	void LoadFromINI(base_type_ptr key, CCINIClass* pINI, bool parseFailAddr)
	{
		if constexpr (CanLoadFromINIFile<T>)
		{
			if (extension_type_ptr ptr = this->TryFind(key))
			{
				if (!pINI) {
					//Debug::Log("[%s] LoadFrom INI Called WithInvalid CCINIClass ptr ! \n", typeid(T).name());
					return;
				}

				switch (ptr->Initialized) {
					case InitState::Blank:
					{
						if constexpr (Initable<T>)
							ptr->Initialize();

						ptr->Initialized = InitState::Inited;

						if constexpr (CanLoadFromRulesFile<T>) {
							if (pINI == CCINIClass::INI_Rules) {
								ptr->LoadFromRulesFile(pINI);
							}
						}

						//Load from rules INI File
						ptr->LoadFromINIFile(pINI, parseFailAddr);
						ptr->Initialized = InitState::Ruled;
					}
					break;
					case InitState::Ruled:
					case InitState::Constanted:
					{
						//load anywhere other than rules
						ptr->LoadFromINIFile(pINI, parseFailAddr);
						//this function can be called again multiple time but without need to re-init the data
						ptr->Initialized = InitState::Ruled;
					}
					break;
					{
					default:
						break;
					}
				}
			}
		}
	}

	void InvalidatePointerFor(base_type_ptr key, AbstractClass* const ptr, bool bRemoved)
	{
		if constexpr (ThisPointerInvalidationSubscribable<T>){
			if (Phobos::Otamaa::ExeTerminated || !ptr)
				return;

			extension_type_ptr Extptr = this->TryFind(key);

			if (!Extptr)
				return;

			if constexpr (PointerInvalidationIgnorAble<T>){
				if (!extension_type::InvalidateIgnorable(ptr))
					Extptr->InvalidatePointer(ptr, bRemoved);
			} else {
					Extptr->InvalidatePointer(ptr, bRemoved);
			}
		}
	}

	void PrepareStream(base_type_ptr key, IStream* pStm)
	{
		static_assert(T::Canary < std::numeric_limits<size_t>::max(), "Canary Too Big !");
		//Debug::Log("[PrepareStream] Next is %p of type '%s'\n", key, this->Name.data());
		this->SavingObject = key;
		this->SavingStream = pStm;
	}

	void SaveStatic()
	{
		auto obj = this->SavingObject;
		//Debug::Log("[SaveStatic] For object %p as '%s Start\n", obj, this->Name.data());
		if (obj && this->SavingStream) {
			if (!this->Save(obj, this->SavingStream))
				Debug::FatalErrorAndExit("[SaveStatic] Saving failed!\n");
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
		//Debug::Log("[SaveStatic] For object %p as '%s Done\n", obj, this->Name.data());
	}

	bool LoadStatic()
	{
		auto obj = this->SavingObject;
		//Debug::Log("[LoadStatic] For object %p as '%s Start\n", obj, this->Name.data());
		if (this->SavingObject && this->SavingStream)
		{
			if (!this->Load(obj, this->SavingStream)){
				//Debug::FatalErrorAndExit("[LoadStatic] Loading object %p as '%s failed!\n", obj, this->Name.data());
				return false;
			}
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
		//Debug::Log("[LoadStatic] For object %p as '%s Done\n", obj, this->Name.data());
		return true;
	}

protected:

	// override this method to do type-specific stuff
	virtual bool Save(base_type_ptr key, IStream* pStm)
	{
		if constexpr (CanThisSaveToStream<T>)
		{
			// this really shouldn't happen
			if (!key)
			{
				//Debug::Log("[SaveKey] Attempted for a null pointer! WTF!\n");
				return false;
			}

			// get the value data
			extension_type_ptr const buffer = this->Find(key);

			if (!buffer)
			{
				//Debug::Log("[SaveKey] Could not find value.\n");
				return false;
			}

			// write the current pointer, the size of the block, and the canary
			PhobosByteStream saver { extension_type::size_Of() };
			PhobosStreamWriter writer { saver };

			writer.Save(T::Canary);
			writer.Save(buffer);

			// save the data
			buffer->SaveToStream(writer);

			// save the block
			if (saver.WriteBlockToStream(pStm))
			{
				//Debug::Log("[SaveKey] Save used up 0x%X bytes\n", saver.Size());
				return true;
			}
		}

		//Debug::Log("[SaveKey] Failed to save data.\n");
		return false;
	}

	// override this method to do type-specific stuff
	virtual bool Load(base_type_ptr key, IStream* pStm)
	{
		if constexpr (CanThisLoadFromStream<T>)
		{
			// this really shouldn't happen
			if (!key)
			{
				//Debug::Log("[LoadKey] Attempted for a null pointer! WTF!\n");
				return false;
			}

			this->ClearExtAttribute(key);
			auto buffer = this->AllocateUnchecked(key);
			this->SetExtAttribute(key, buffer);

			PhobosByteStream loader { 0 };
			if (!loader.ReadBlockFromStream(pStm))
			{
				//Debug::Log("[LoadKey] Failed to read data from save stream?!\n");
				return false;
			}

			PhobosStreamReader reader { loader };
			if (reader.Expect(T::Canary) && reader.RegisterChange(buffer))
			{
				buffer->LoadFromStream(reader);
				if (reader.ExpectEndOfBlock())
					return true;
			}
		}

		return false;
	}


//private:
//	Container<T>(const Container<T>&) = delete;
//	Container<T>& operator = (const Container<T>&) = delete;
//	Container<T>& operator = (Container<T>&&) = delete;
};

//#define CONSTEXPR_NOCOPY_CLASS(containerT , name)\
//constexpr ExtContainer() : Container<containerT> { ##name## } {}\
//virtual ~ExtContainer() override = default;\
//private:\
//ExtContainer(const ExtContainer&) = delete;\
//ExtContainer(ExtContainer&&) = delete; \
//ExtContainer& operator=(const ExtContainer& other) = delete;
//
//#define CONSTEXPR_NOCOPY_CLASSB(containerName , containerT , name)\
//constexpr containerName() : Container<containerT> { ##name## } {}\
//virtual ~containerName() override = default;\
//private:\
//containerName(const containerName&) = delete;\
//containerName(containerName&&) = delete; \
//containerName& operator=(const containerName& other) = delete;