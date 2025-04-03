#pragma once

#include <Utilities/SavegameDef.h>

class AnimTypeClass;
class AnimClass;
class SpawnsStatus
{
	bool _initSpawnFlag { false };
	CDTimerClass _spawnInitDelayTimer {};
	CDTimerClass _spawnDelayTimer {};
	int _spawnCount { 0 };

	void ResetLoopSpawn()
	{
		_initSpawnFlag = false;
		_spawnInitDelayTimer.Stop();
		_spawnDelayTimer.Stop();
		_spawnCount = 0;
	}

public:

	void OnUpdate(AnimClass* pLinked);
	void OnLoop(AnimClass* pLinked);
	void OnDone(AnimClass* pLinked);
	void OnNext(AnimClass* pLinked,AnimTypeClass* pNext);

#pragma region save/load
	template <typename T>
	bool Serialize(T& stream)
	{
		return stream
			.Process(this->_initSpawnFlag)
			.Process(this->_spawnInitDelayTimer)
			.Process(this->_spawnDelayTimer)
			.Process(this->_spawnCount)
			.Success();
	};

	bool Load(PhobosStreamReader& stream, bool registerForChange)
	{
		return this->Serialize(stream);
	}

	bool Save(PhobosStreamWriter& stream) const
	{
		return const_cast<SpawnsStatus*>(this)->Serialize(stream);
	}
#pragma endregion
};
