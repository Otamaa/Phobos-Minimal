#include <CCFileClass.h>

#include <Utilities/Debug.h>
#include <d3d9.h>

typedef HRESULT(__stdcall* D3DXCreateTextureFromFileInMemory)(LPDIRECT3DDEVICE9 pDevice, LPCVOID pSrcData, UINT SrcDataSize, LPDIRECT3DTEXTURE9 *ppTexture);

extern HMODULE Loaded_d3dx9_29 = nullptr;
static const char* Module_name = "d3dx9_29.dll";
static bool IsLoaded = false;

void inline LoadD3DX9_29Lib() {
	Loaded_d3dx9_29 =  LoadLibrary(Module_name);
	IsLoaded = Loaded_d3dx9_29;
}

void inline FreeD3DX_29Lib() {
	if(IsLoaded && Loaded_d3dx9_29)
		FreeLibrary(Loaded_d3dx9_29);
}

void inline CheckD3DX9_29DllAvail() {
	if (!IsLoaded && !Loaded_d3dx9_29)
		Debug::FatalErrorAndExit("%s Is Not Present ! , Please Check Fix !. \n", Module_name);
}

void Load(HMODULE pLib, LPDIRECT3DDEVICE9 pDevice , const char* pFile , LPDIRECT3DTEXTURE9 *ppTexture)
{
	if (!pLib) {
		Debug::Log("Failed To Load d3dx9_29.dll ! for %s. \n", pFile);
		return;
	}

	auto pOpen = GameCreate<CCFileClass>(pFile);

	if (!pOpen) {
		Debug::Log("Failed To Create CCFileClass for %s !. \n", pFile);
		return;
	}

	auto pProc = *reinterpret_cast<D3DXCreateTextureFromFileInMemory*>(GetProcAddress(pLib, "D3DXCreateTextureFromFileInMemory"));

	if (pOpen->Open(FileAccessMode::Read)) {
		if (pProc) {
			auto Loaded = static_cast<LPCVOID>(CCFileClass::ReadWholeFile(pOpen));
			auto nSize = pOpen->GetFileSize();
			//Device ? , Textures ? , how ?
			if (SUCCEEDED(pProc(pDevice, Loaded, nSize, ppTexture)))
				Debug::Log("%s Texture created !.\n", pFile);
			else
				Debug::Log("Failed to create texture for %s !. \n", pFile);
		}
	}

	GameDelete<true>(pOpen);
}