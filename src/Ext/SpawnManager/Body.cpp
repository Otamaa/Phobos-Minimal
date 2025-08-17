#include "Body.h"

#include <Ext/Techno/Body.h>
#include <Utilities/Macro.h>


// =============================
// load / save
//template <typename T>
//void SpawnManagerExt::ExtData::Serialize(T& Stm) {
//	//Debug::LogInfo("Processing Element From SpawnManagerExt ! ");
//	Stm
//		.Process(this->Initialized)
//		;
//}

// =============================
// container
//SpawnManagerExt::ExtContainer SpawnManagerExt::ExtMap;

// =============================
// container hooks
//

//ASMJIT_PATCH(0x6B6E7F, SpawnManagerClass_CTOR,0x6 )
//{
//	GET(SpawnManagerClass*, pItem, ESI);
//	SpawnManagerExt::ExtMap.Allocate(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH(0x6B703E, SpawnManagerClass_DTOR, 0x6)
//{
//	GET(SpawnManagerClass*, pItem, EDI);
//	SpawnManagerExt::ExtMap.Remove(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x6B7F10, SpawnManagerClass_SaveLoad_Prefix, 0x6)
//ASMJIT_PATCH(0x6B80B0, SpawnManagerClass_SaveLoad_Prefix, 0x5)
//{
//
//	GET_STACK(SpawnManagerClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//
//	SpawnManagerExt::ExtMap.PrepareStream(pItem, pStm);
//	return 0;
//}
//
//ASMJIT_PATCH(0x6B80A3, SpawnManagerClass_Load_Suffix, 0x5)
//{
//	GET(HRESULT, nRes, EBP);
//
//	if(SUCCEEDED(nRes))
//		SpawnManagerExt::ExtMap.LoadStatic();
//
//	return 0;
//}
//
//ASMJIT_PATCH(0x6B810D, SpawnManagerClass_Save_Suffix, 0x6)
//{
//	GET(HRESULT, nRes, EAX);
//
//	if (SUCCEEDED(nRes))
//		SpawnManagerExt::ExtMap.SaveStatic();
//
//	return 0x0;
//}

//Detach Func ! 0x006B7C60

void FakeSpawnManagerClass::_Detach(AbstractClass* pTarget)
{
	// default to false , sadly
	FakeSpawnManagerClass::_DetachB(pTarget, true);
}

void FakeSpawnManagerClass::_DetachB(AbstractClass* pTarget, bool removed)
{
	if (pTarget == this->Target && removed)
	{
		this->Target = nullptr;

		if (!this->NewTarget){
			//will call _Detach function internally
			this->ResetTarget();
		}
	}

	if (pTarget == this->NewTarget && removed) {
		this->NewTarget = 0;
	}

	// Search through SpawnControls
	for (int max = this->SpawnedNodes.Count - 1; max >= 0; --max) {
		if (auto pSpawnee = this->SpawnedNodes.Items[max]) {
			if ((pSpawnee->Unit && ((pSpawnee->Unit == pTarget && removed)
				|| pSpawnee->Unit->Health <= 0
				|| pSpawnee->Unit->IsSinking
				|| !pSpawnee->Unit->IsAlive
				|| pSpawnee->Unit->IsCrashing
				|| pSpawnee->Unit->IsKamikaze))
				|| pSpawnee->IsSpawnMissile) {
				this->SpawnedNodes.Items[max]->Unit = nullptr;
				this->SpawnedNodes.Items[max]->NodeSpawnTimer.Start(this->RegenRate);
				this->SpawnedNodes.Items[max]->Status = SpawnNodeStatus::Dead;
			}
		}
	}

	if (pTarget == this->Owner && removed && !Phobos::Otamaa::ExeTerminated) {
		this->KillNodes();
		//will call _Detach function internally
		this->ResetTarget();
	}
}

DEFINE_FUNCTION_JUMP(CALL, 0x6B7637, FakeSpawnManagerClass::_Detach);
DEFINE_FUNCTION_JUMP(CALL, 0x6B7ACD, FakeSpawnManagerClass::_Detach);
DEFINE_FUNCTION_JUMP(CALL, 0x6B7C16, FakeSpawnManagerClass::_Detach);

DEFINE_JUMP(LJMP, 0x707B19, 0x707B29);