//TODO:

template<size_t idx>
static void* AresExtMap_Find(void* const key)
{
	return AresData::AresThiscall<AresData::FunctionIndices::ExtMapFindID, void*, DWORD, void*>()(AresData::AresStaticInstanceFinal[idx], key);
}

#include <Ext/ParticleSystem/Body.h>

struct AresParticleExtData
{
	ParticleSystemClass* OwnerObject;
	InitState State;
	int Type;
	ParticleTypeClass* HoldType;
	/*	std::vector<T> usually compiled like these
	* struct std_vector_T // size 0xC
	* {
	*	 T* first;
	*	 T* last;
	*    T* end;
	* }
	*/
	std::vector<ParticleDatas> DataA; //stored state data
	std::vector<ParticleDatas> DataB; //stored state data (only used on gas)
};
//the alloc size doesnt match the class size for some reason ?
//static_assert(sizeof(AresParticleExtData) == 64);

//DEFINE_HOOK(0x62FD60, ParticleSystemClass_Update, 9)
//{
//	GET(ParticleSystemClass*, pThis, ECX);
//	const auto pThisExt = (AresParticleExtData*)AresExtMap_Find<0>(pThis);
//	if(!pThisExt->DataA.empty() && !pThisExt->DataB.empty())
//	Debug::Log("ParticeSystem [%s] With ExtPtr [%x - offs %x] ! \n", pThis->get_ID(), pThisExt);
//	//return HandleParticleSys(pThis) ? 0x62FE43 : 0;
//	return 0x0;
//}