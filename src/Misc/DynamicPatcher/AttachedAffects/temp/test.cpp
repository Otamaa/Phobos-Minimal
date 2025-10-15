#include <Misc/Otamaa/Libs/Base/Always.h>
#include <iostream>
#include <vector>
#include <functional>

template<typename EventArgs>
class Event
{
	friend class ClassWithEvent;

public:
	void operator+=(std::function<void(const EventArgs&)> callback)
	{
		m_funcsList.emplace_back(std::move(callback));
	}

private:
	void Invoke(const EventArgs& args) const
	{
		for (auto& func : m_funcsList)
		{
			func(args);
		}
	}

	std::vector<std::function<void(const EventArgs&)>> m_funcsList;
};

class ClassWithEvent
{
protected:
	template<typename EventArgs>
	void InvokeEvent(const Event<EventArgs>& event, const EventArgs& args)
	{
		event.Invoke(args);
	}
};



class AppleFallEventArgs
{
public:
	int size;
	int weight;
};

class ApplesTree : public ClassWithEvent
{
public:
	Event<AppleFallEventArgs> AppleFallEvent;

	void triggerAppleFall(int size, int weight)
	{
		AppleFallEventArgs args;

		args.size = size;
		args.weight = weight;

		InvokeEvent(AppleFallEvent, args);
	}
};

class ApplesCollector
{
public:
	void HandleAppleFallEvent(const AppleFallEventArgs& args)
	{
		std::cout << "Apple size is " << args.size << "weight is " << args.weight << std::endl;
	}
};

__interface IAttachEffectBehaviour
{

	// 返回AE是否还存活
	bool IsAlive();
	// AE激活，开始生效
	void Enable(DWORD* pOwner, DWORD* pHouse, DWORD* pAttacker);
	// AE关闭，销毁相关资源
	void Disable(int X, int Y, int Z);
	// 重置计时器
	void ResetDuration();
	// 更新
	void OnUpdate(DWORD* pOwner, bool isDead);
	// 被超时空冻结更新
	void OnTemporalUpdate(DWORD* ext, DWORD* pTemporal);
	// 挂载AE的单位出现在地图上
	void OnPut(DWORD* pOwner, DWORD* pCoord, unsigned faceDir);
	// 挂载AE的单位从地图隐藏
	void OnRemove(DWORD* pOwner);
	// 收到伤害
	void OnReceiveDamage(DWORD* pOwner, int* pDamage, int DistanceFromEpicenter, DWORD* pWH, DWORD* pAttacker, bool IgnoreDefenses, bool PreventPassengerEscape, DWORD* pAttackingHouse);
	// 收到伤害导致死亡
	void OnDestroy(DWORD* pOwner);
	// 按下G键
	void OnGuardCommand();
	// 按下S键
	void OnStopCommand();
};

__interface Test
{
public:
	void InitEffect(DWORD* eType, DWORD* aeType);
};

__interface IEffectType
{
	bool TryReadType(DWORD reader, std::string section);
};

template<typename T>
concept BaseFromIEffectType = std::is_base_of<IEffectType, T>::value;

template<BaseFromIEffectType T>
class Ef : public IAttachEffectBehaviour , public Test
{
public:

	T* Type;
	DWORD* AEType;
	DWORD* OwnerAEM;

	Ef() :
		Type { nullptr }
		,AEType { nullptr }
		,OwnerAEM { nullptr }
	{ }

	void InitEffect(DWORD* eType, DWORD* aeType)
	{
		Type = (T*)eType;
		AEType = aeType;
	}

	void Enable(DWORD* pOwner, DWORD* pHouse, DWORD* pAttacker) override
	{

	}

	void OnEnable(DWORD* pOwner, DWORD* pHouse, DWORD* pAttacker) { }
};


class EnableAction
{
public:
	DWORD* pOwner;
	DWORD* pHouse;
	DWORD* pAttacker;
};

class AttachEffect : public IAttachEffectBehaviour , public ClassWithEvent
{
	std::vector<IAttachEffectBehaviour> effects { };
public:

	Event<EnableAction> EnableActionEvent;

	void triggerEnableAction(DWORD* pOwner, DWORD* pHouse, DWORD* pAttacker)
	{
		EnableAction args;

		args.pOwner = pOwner;
		args.pHouse = pHouse;
		args.pAttacker = pAttacker;

		InvokeEvent(EnableActionEvent, args);
	}

	void RegisterAction(const IAttachEffectBehaviour& behaviour)
	{
		effects.push_back(behaviour);
		this->EnableActionEvent += behaviour.Enable;
	}
};

/*
void Execute()
{
	ApplesTree applesTree; //base
	ApplesCollector applesCollector;
	AttachEffect AE;

	AE->EnableActionEvent +=
	applesTree.AppleFallEvent += [&](auto& args) { applesCollector.HandleAppleFallEvent(args); };

	applesTree.triggerAppleFall(1, 2);
}*/