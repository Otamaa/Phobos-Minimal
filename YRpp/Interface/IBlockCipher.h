#pragma once

#include <unknwn.h>

DECLARE_INTERFACE_IID_(IBlockCipher, IUnknown, "E0113100-6A7C-11D1-B6F9-00A024DDAFD1")
{
	virtual HRESULT __stdcall Submit_Key(int length, void *key) PURE;
	virtual HRESULT __stdcall Max_Key_Length(LONG *key_length) PURE;
	virtual HRESULT __stdcall Block_Size(LONG *key_length) PURE;
	virtual HRESULT __stdcall Encrypt(int length, void *plain_text, void *cypher_text) PURE;
	virtual HRESULT __stdcall Decrypt(int length, void *cypher_text, void *plain_tex) PURE;
};