#include <Phobos.h>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <cstring>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

//completely replacing the old way loaded file are linked within
//the linked list with map for better maintaiability and readbility
//the map has no ownership of the resource i suppoe 
//it only map it to the appropriate place 
//not sure how the game know that the file linked here is already invalid 
//safety issues
//also it seems this part is only holding onto pointer but never clean it up
//so when the game not exit and user using this function actively 
//there is big chance memory leak happen somewhere
std::unordered_map<std::string, void*> g_GlobalFileLinks {};

void* __fastcall FakeFileLoader::_Retrieve(const char* name, bool forceShapeCache)
{
#ifndef ReplaceImpl

	if (!name)
		return nullptr;

	// Uppercase copy — MIX lookups and zip keys are always uppercase
	char upperName[260];
	strcpy(upperName, name);
	_strupr(upperName);

	auto find = g_GlobalFileLinks.find(upperName);

	// Load from any FileClass — raw alloc or ShapeCache depending on type
	auto LoadFrom = [&](FileClass& file) -> void*
		{
			if (forceShapeCache || strstr(upperName, ".SHP"))
			{
				return (void*)GameCreate<SHPCaches>(name);
			}

			return CCFileClass::Load_Alloc_Data(file);
		};


	// ------------------------------------------------------------------
	// BST cache — already-loaded data (MIX or zip from prior call)
	//     Zero allocation, zero I/O on hit.
	// ------------------------------------------------------------------
	bool alreadyExist = false;
	if (find != g_GlobalFileLinks.end())
	{
		if (find->second)
			return find->second;

		alreadyExist = true;
	}

	// ------------------------------------------------------------------
	// ZipFileSystem — highest priority
	//     Checked before BST so art/ini overrides beat already-cached vanilla.
	//     Non-.SHP files (INI, PAL, CSF, audio, map files) work fully.
	//     .SHP files fall through to vanilla path — see note in LoadFrom.
	// ------------------------------------------------------------------
	//if (ZipFileSystem::Instance().Contains(upperName)) {
	//	// .SHP needs ShapeCache ctor which re-opens by name — skip zip for now
	//	const bool isSHP = forceShapeCache || strstr(upperName, ".SHP") != nullptr;
	//	if (!isSHP) {
	//		void* fileData = nullptr;
	//
	//		//find first zip file that have this file and extract it
	//		ZipFileSystem::Instance().ForEach([crc, &fileData](const ZipEntry& entry) {
	//		 if (SafeChecksummer()(entry.EntryName.c_str(), entry.EntryName.size()) == crc) {
	//				 ZipBackedFileClass zipFile(entry.EntryName.c_str());
	//				 if (zipFile.Valid()) {
	//					 Debug::LogInfo("[ZipFS] Retrieving file {}' from archive'{}'", entry.EntryName.c_str(), entry.EntryName.size());
	//					 fileData = CCFileClass::Load_Alloc_Data(zipFile);
	//					 return; //only if valid
	//				 }
	//			}
	//		});
	//
	//		if (fileData)
	//			return CacheAndReturn(fileData);
	//
	//		// Extraction failed despite Contains() — warn, fall through to [2]/[3]
	//		Debug::LogInfo("[ZipFS] Extract failed for '{}', falling back", upperName);
	//	}
	//	// .SHP: intentional fall-through to [2]/[3] — vanilla ShapeCache handles it
	//}

	// ------------------------------------------------------------------
	// Disk / vanilla MIX fallback — lowest priority
	//     CCFileClass transparently resolves through MixFileClass::MIXes.
	//     Result cached into BST so next call hits [2].
	// ------------------------------------------------------------------
	CCFileClass file(name);
	if (!file.IsAvaible(false))
		return nullptr;

	//Debug::Log("File Loaded %s sz %d\n", name, file.Size());

	auto pPtr = LoadFrom(file);

	if (alreadyExist)
		find->second = pPtr;
	else
		g_GlobalFileLinks.emplace(upperName, pPtr);

	return pPtr;

#else
	return Retrieve(name, forceShapeCache);
#endif

}

DEFINE_FUNCTION_JUMP(LJMP, 0x5B40B0, FakeFileLoader::_Retrieve)
DEFINE_FUNCTION_JUMP(CALL, 0x41CAF7, FakeFileLoader::_Retrieve)
DEFINE_FUNCTION_JUMP(CALL, 0x41CB08, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4279DA, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x427A04, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x427B15, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x427BE9, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x427C07, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x42891E, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4309FD, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x430A61, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45E904, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45E988, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45E999, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45EA16, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45EA79, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F28D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F2A5, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F525, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F543, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F615, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F6E9, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F7C1, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F82D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F84B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45F91D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45FA0B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45FA39, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x45FA6B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x47EFFD, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x47F00E, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x47F26A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4A38DE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4A3985, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4A8862, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4A8873, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6D07, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6D1A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6D2D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6E52, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B6F41, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B7349, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x4B73A8, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x51916D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5194FF, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BBE3, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BC55, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BCFD, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BE6D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BF26, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52BFDA, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52C08E, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x52C142, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x531381, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x534C04, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x534CC3, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x546725, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5468DB, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5468F8, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5469BF, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x560D7B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x561093, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5D2EBA, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F76EE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F773C, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F778A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F77D8, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9249, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9267, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9281, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9685, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5F9931, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE68C, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE6AE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE6F6, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE714, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FE928, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x5FEBEC, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x62769F, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x66C5F4, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x66C606, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x677FAC, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x677FBE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x690660, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6906BD, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x69071A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x690A4D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x690B1D, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6A5012, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6A8167, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6ABD57, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6ABD68, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6ABD79, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6B1B94, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6B1BCE, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6B57C2, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6CE89B, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6CE8B1, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6CEE20, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6CEE38, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x6DAE07, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x715820, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x715A38, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x715A54, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x715B05, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x716C77, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x716D04, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x716D1A, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x716D6F, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x71DFBB, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x73CEE0, FakeFileLoader::_Retrieve); //OREGATH.SHP
DEFINE_FUNCTION_JUMP(CALL, 0x73D3EF, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x747490, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x7474A1, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x747BB4, FakeFileLoader::_Retrieve);
DEFINE_FUNCTION_JUMP(CALL, 0x748093, FakeFileLoader::_Retrieve);

void __fastcall _Cache_File(char* filename)
{
	// ????
	if (!filename)
		return;

	char upperName[260];
	strcpy(upperName, filename);
	_strupr(upperName);
	g_GlobalFileLinks[upperName] = nullptr;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x5B4270, _Cache_File)


void __fastcall _Destroy_Cache_()
{
	g_GlobalFileLinks.clear();
}

DEFINE_FUNCTION_JUMP(LJMP, 0x5B4310, _Cache_File)