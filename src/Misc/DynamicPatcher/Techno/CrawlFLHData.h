#pragma once

#include <Utilities/TemplateDef.h>

struct CrawlingFLHData
{


	void Read(INI_EX& nParser, const char* pSection)
	{
		PrimaryCrawlFLH.Read(nParser, pSection, "PrimaryCrawlingFLH");
		Elite_PrimaryCrawlFLH.Read(nParser, pSection, "ElitePrimaryCrawlingFLH");

		SecondaryCrawlFLH.Read(nParser, pSection, "SecondaryCrawlingFLH");
		Elite_SecondaryCrawlFLH.Read(nParser, pSection, "EliteSecondaryCrawlingFLH");
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		//Debug::LogInfo("Processing Element From CrawlingFLHData ! ");
		Stm
			.Process(PrimaryCrawlFLH)
			.Process(Elite_PrimaryCrawlFLH)
			.Process(SecondaryCrawlFLH)
			.Process(Elite_SecondaryCrawlFLH)
			;
	}
};
