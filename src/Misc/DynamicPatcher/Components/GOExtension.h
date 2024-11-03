#pragma once

#include "IExtData.h"
#include "GameObject.h"
#include "TExtension.h"

// TODO : Component S/L
// something feels not right atm

template <typename TBase, typename TExt>
class GOExtensionData : public TExtension<TBase>, public IExtData
{
public:

	void EnsureConstanted(TExt::base_type* const pAttach)
	{
		this->SetOwnerObject(pAttach);

		m_GameObject.Tag = typeid(TExt).name();
		m_GameObject.SetExtData(this);

		AttachComponents();

		m_GameObject.EnsureAwaked();
	}

	void EnsureDestroyed()
	{
		m_GameObject.ForeachChild([](Component* c)	{
			c->EnsureDestroy();
		}, true);
	}

#pragma region Save/Load
	template <typename T>
	void Serialize(T& stream)
	{
		stream
			.Process(this->globalScriptsCreated)
			.Process(this->m_GameObject)
			;
	}

	virtual void LoadFromStream(PhobosStreamReader& stream) override
	{
		TExtension<TBase>::LoadFromStream(stream);
		this->Serialize(stream);
	}

	virtual void SaveToStream(PhobosStreamWriter& stream) override
	{
		TExtension<TBase>::SaveToStream(stream);
		this->Serialize(stream);
	}
#pragma endregion

	template <typename TScript>
	TScript* FindOrAttach() {
		return m_GameObject.FindOrAttach<TScript>();
	}

	template <typename TScript>
	TScript* GetScript() {
		return m_GameObject.GetComponentInChildren<TScript>();
	}

	template <typename TStatus>
	TStatus* GetExtStatus()
	{
		if (!_status) {
			_status = m_GameObject.GetComponentInChildren<TStatus>();
		}

		return static_cast<TStatus*>(_status);
	}

	void SetExtStatus(Component* pStatus) {
		_status = pStatus;
	}

private:

	GameObject m_GameObject {};

	//AE like statusses
	//fetch it only one time
	Component* _status { nullptr };

	bool globalScriptsCreated { false };

	virtual void AttachComponents() override
	{
		if (!globalScriptsCreated) {
			//add script that come with this extdata
			//m_GameObject.AddComponent(scriptName);

			globalScriptsCreated = true;
		}
	}

};
