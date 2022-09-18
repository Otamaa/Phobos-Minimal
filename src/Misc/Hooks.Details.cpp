#ifdef DetailsPatch


struct FakeRulesExt
{
	struct ExtData
	{
		Valueable<int>  DetailMinFrameRateMedium { 0 };
		Valueable<bool> DetailLowDisableBullet { false };
	};


private:
	static std::unique_ptr<ExtData> Data;
public:
	static  ExtData* Global() { return Data.get(); }

	static bool DetailsCurrentlyEnabled()
	{
		// not only checks for the min frame rate from the rules, but also whether
		// the low frame rate is actually desired. in that case, don't reduce.
		auto const current = FPSCounter::CurrentFrameRate();
		auto const wanted = static_cast<unsigned int>(
			60 / Math::clamp(GameOptionsClass::Instance->GameSpeed, 1, 6));

		return current >= wanted || current >= Detail::GetMinFrameRate();
	}

	static bool DetailsCurrentlyEnabled(int const minDetailLevel)
	{
		return GameOptionsClass::Instance->DetailLevel >= minDetailLevel
			&& DetailsCurrentlyEnabled();
	}

	static inline bool IsFPSEligible()
	{
		auto const wanted = static_cast<unsigned int>(
			60 / Math::clamp(GameOptionsClass::Instance->GameSpeed, 1, 6));

		if (FPSCounter::CurrentFrameRate() >= wanted)
			return false;

		auto nMedDetails = Global()->DetailMinFrameRateMedium.Get();
		auto const nBuff = RulesGlobal->DetailBufferZoneWidth;
		static bool nSomeBool = false;

		if (nSomeBool)
		{
			if (FPSCounter::CurrentFrameRate() < nMedDetails + nBuff)
				return 1;
			nSomeBool = false;
		}
		else
		{
			if (FPSCounter::CurrentFrameRate() >= nMedDetails)
				return 0;

			nMedDetails += nBuff;

			nSomeBool = true;
		}

		return FPSCounter::CurrentFrameRate() < nMedDetails;
	}

	static inline bool DetailsCurrentlyEnabled_Changed(int const nCurDetailLevel)
	{
		if (DetailsCurrentlyEnabled() && nCurDetailLevel > 0)
			return false;

		if (IsFPSEligible && nCurDetailLevel > 1)
			return false;

		return true;
	}
};

DEFINE_HOOK(0x422FCC, AnimClass_DrawDetail, 0x5)
{
	GET(AnimClass*, pThis, ESI);
	return FakeRulesExt::DetailsCurrentlyEnabled_Changed(pThis->Type->DetailLevel) ? 0x422FEC : 0x4238A3;
}

DEFINE_HOOK(0x42307D, AnimClass_DrawDetail_Translucency, 0x6)
{
	GET(AnimTypeClass*, pType, EAX);

	if (GameOptionsClass::Instance->DetailLevel < pType->TranslucencyDetailLevel)
		return 0x4230FE;

	if (!FakeRulesExt::IsFPSEligible())
		return 0x42308D;

	return pType->TranslucencyDetailLevel <= 1 ? 0x42308D : 0x4230FE;

}

DEFINE_HOOK(0x4680E2, BulletClass_Detail, 0x6)
{
	if (!FakeRulesExt::Global()->DetailLowDisableBullet)
		return 0;

	if (FakeRulesExt::DetailsCurrentlyEnabled())
		return 0x468422;

	return !GameOptionsClass::Instance->DetailLevel ? 0x468422 : 0x0;
}

DEFINE_HOOK(0x53D591, IonBlastClass_Detail, 0x6)
{
	if (GameOptionsClass::Instance->GameSpeed < 2)
		return 0x53D842;
	return (!FakeRulesExt::IsFPSEligible()) ? 0x53D597 : 0x53D842;
}

DEFINE_HOOK(0x550268, LaserDrawClass_Detail, 0x6)
{
	if (FakeRulesExt::DetailsCurrentlyEnabled())
		return 0x5509CB;

	if (!GameOptionsClass::Instance->GameSpeed)
		return = 0x5509CB;

	return 0x0;
}

DEFINE_HOOK(0x62CFBB, ParticleClass_Detail_Translucency, 0x7)
{
	if (MGameOptionsClass::Instance->GameSpeed < 2)
		return 0x62CFEC;

	return (!FakeRulesExt::IsFPSEligible()) ? 0x62CFC4 : 0x62CFEC;
}
#endif