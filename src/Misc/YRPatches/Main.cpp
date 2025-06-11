
#include <Utilities/Patch.h>
#include <Syringe.h>
#include <CCINIClass.h>

#include <Ext/Scenario/Body.h>
#include <Misc/Ares/CSF.h>

ASMJIT_PATCH(0x686C2A, ScenarioClass_ReadTutorialFromMap, 0x6)
{
	GET(CCINIClass*, pINI, EBP);

	if(pINI->GetSection("Tutorial")) {
		const int tut_count = pINI->GetKeyCount("Tutorial");
		Debug::Log("Reading Tutorial  from MAP , entry count = %d\n", tut_count);

		for (int i = 0; i < tut_count; ++i) {
			if (pINI->ReadString("Tutorial", pINI->GetKeyName("Tutorial", i),
				Phobos::readDefval, Phobos::readBuffer) > 0)
			{
				Debug::LogInfo("Value {} , at Idx {}", Phobos::readBuffer, i);
			}
		}

	}

	return 0x0;
}

/*
void __thiscall
read_tut_from_map(INIClass* scenario, char* section, char* entry, int fallback)
{
	char buf[0x200] = { 0 };
	wchar_t wbuf[0x400] = { 0 };

	char* entryName;
	csfLabelEntry* labelEntry;
	csfString* csf;
	int len;
	int wlen;

	int i = INIClass__EntryCount(scenario, "Tutorial");
	if (i > 100)
		i = 100;


	while (i-- > 0)
	{
		entryName = INIClass__GetEntry(scenario, "Tutorial", i);

		labelEntry = &CSF_Label_Array[CSF_Label_Array_Count];
		CSF_Label_Array_Count++;

		memcpy(labelEntry->label, entryName, 31);


		csf = new(sizeof(csfString));
		len = INIClass__GetString(scenario, "Tutorial", entryName, 0, buf, 0x200);
		if (len > 0)
			wlen = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, buf, -1, csf->Text, 0x400);

		CSF_Text_Array[CSF_Text_Array_Count] = csf;

		labelEntry->numValues = 1;
		labelEntry->index = CSF_Text_Array_Count;

		CSF_Text_Array_Count++;

		WWDebug_Printf("*****************Found entry '%s' = %s\n", labelEntry->label, buf);
	}
	qsort(CSF_Label_Array, CSF_Label_Array_Count, sizeof(csfLabelEntry),
		  (int (__attribute__((__cdecl__))*)(const void*, const void*))strcmpi);

	INIClass__GetBool(scenario, section, entry, fallback);
}*/