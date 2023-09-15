#pragma once

#include <ParticleSystemClass.h>

#include <memory>

struct UninitAttachedSystem
{
	void operator() (ParticleSystemClass* pAnim) const
	{
		if (pAnim && pAnim->IsAlive)
		{
			pAnim->Owner = nullptr;
			pAnim->UnInit();
		}
	}
};

class ConvertClass;

template <typename T>
using UniqueGamePtr = std::unique_ptr<T, GameDeleter>;

template <typename T>
using UniqueGamePtrB = std::unique_ptr<T, GameDTORCaller>;

using UniqueParticleSystemClassPtr = std::unique_ptr<ParticleSystemClass, UninitAttachedSystem>;