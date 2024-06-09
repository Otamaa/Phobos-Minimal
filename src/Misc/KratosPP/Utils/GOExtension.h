#pragma once

#include <Misc/KratosPP/Components/GameObject.h>
#include <Misc/KratosPP/Utils/TContainer.h>

template <typename TBase, typename TExt>
class GOExtensionData : public TExtension<TBase>
{
public:
	using base_type = TBase;

	GOExtensionData(TBase* OwnerObject) : TExtension<TBase>(OwnerObject)
	{
		meObject.baseName = this->baseName;
		m_GameObject.Tag = typeid(TExt).name();
		m_GameObject.SetExtData(this);
		AttachComponents();
	}

	~GOExtensionData() override
	{
		try
		{
			m_GameObject.ForeachChild([](Component* c)
			{
				c->EnsureDestroy();
			}, true);
		}
		catch (const std::exception& e)
		{
			Debug::Log("~GOExtension() throw exception: %s \n", e.what());
		}
	}

#pragma region Save/Load
	template <typename T>
	void Serialize(T& stream)
	{
		stream
			// 序列化GameObject对象，将调用GameObject的Save/Load函数，
			// 由GameObject自己维护自己的读存档长度
			// GameObject自身储存的列表如何变化，都不影响Ext的读存档
			.Process(this->m_GameObject);
	}

	virtual void LoadFromStream(PhobosStreamReader& stream) override
	{
		// 首先读取ext
		TExtension<TBase>::LoadFromStream(stream);
		this->Serialize(stream);
	}
	virtual void SaveToStream(PhobosStreamWriter& stream) override
	{
		TExtension<TBase>::SaveToStream(stream);
		this->Serialize(stream);
	}
#pragma endregion

	//----------------------
	// GameObject
	GameObject* GetGameObject()
	{
		return m_GameObject.GetAwaked();
	}
	__declspec(property(get = GetGameObject)) GameObject* _GameObject;

	/// @brief Helper调用，通过Ext查找或附加GameObject下的脚本
	/// @tparam TScript
	/// @return
	template <typename TScript>
	TScript* FindOrAttach()
	{
		return m_GameObject.FindOrAttach<TScript>();
	}

	/// @brief Helper调用，通过Ext查找GameObject下的脚本
	/// @tparam TScript
	/// @return
	template <typename TScript>
	TScript* GetScript()
	{
		return m_GameObject.GetComponentInChildren<TScript>();
	}

	template <typename TStatus>
	TStatus* GetExtStatus()
	{
		if (_status == nullptr)
		{
			_status = m_GameObject.GetComponentInChildren<TStatus>();
		}
		return static_cast<TStatus*>(_status);
	}

	void SetExtStatus(Component* pStatus)
	{
		_status = pStatus;
	}
private:
	//----------------------
	// GameObject
	// GO作为实例进行储存
	GameObject m_GameObject {};

	// Status Component
	Component* _status = nullptr;

	//----------------------
	// Scripts

	/// <summary>
	/// 逻辑开始运行时再对Component进行实例化
	/// Component的Awake分两种情况调用：
	/// 当正常开始游戏时，TechnoClass_Init触发调用_GameObject，唤醒Components，此时可以获得TechnoType；
	/// 但当从存档载入时，TechnoClass_Init不会触发，而是通过TechnoClass_Load_Suffix实例化TechnoExt，此时无法获得TechnoType，
	/// 在LoadFromStream里读取_awaked，跳过执行Awake()，直接通过LoadFromStream读取数据
	/// </summary>
	virtual void AttachComponents() override
	{

		if (globalScriptsCreated)
		{
			return;
		}
		// Search and instantiate global script objects in TechnoExt
		std::list<std::string> globalScripts {};

		// 在Ext中读取需要添加的脚本名单
		TExt::AddGlobalScripts(globalScripts, this);

		for (std::string& scriptName : globalScripts)
		{
			// Component的创建需要使用new，在Component::EnsureDesroy中Delete
			m_GameObject.AddComponent(scriptName);
		}
		globalScriptsCreated = true;
	}

	// 已经初始化全局脚本的标记
	bool globalScriptsCreated = false;
};

#define CONT(aaa)\
class  aaa##Container : public Container::Main<##aaa##>\
{\
public:\
	constexpr aaa##Container() : Container::Main<##aaa##>(typeid(##aaa##).name()) { }\
	~##aaa##Container() = default;\
};
