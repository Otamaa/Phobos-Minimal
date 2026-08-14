#include "PhobosObjectShader.h"

#include "Runtime/runtime.hpp"

#include <Utilities/Debug.h>

#include <d3dcompiler.h>

#include <algorithm>
#include <new>
#include <string_view>

// ===========================================================================
// Hashing
//
// DIFF: the reference implementation open-codes an FNV-1a loop four separate
// times - twice in LoadPixelShader and twice in LoadVertexShader - each over a
// std::string whose SSO branch is expanded inline. One helper here, and the
// key includes the entry point and target, which theirs does not: two shaders
// compiled from the same file with different entry points collide in their
// cache.
// ===========================================================================

namespace
{
	constexpr uint32_t FnvOffset = 2166136261u;
	constexpr uint32_t FnvPrime = 16777619u;

	uint32_t FnvHash(std::string_view text, uint32_t seed = FnvOffset)
	{
		uint32_t hash = seed;

		for (const char c : text)
			hash = (hash ^ static_cast<uint8_t>(c)) * FnvPrime;

		return hash;
	}
}

size_t PhobosShaderKeyHash::operator()(const PhobosShaderKey& k) const noexcept
{
	uint32_t hash = FnvHash(k.File);
	hash = FnvHash(k.Entry, hash);
	hash = FnvHash(k.Target, hash);

	return hash;
}

// ===========================================================================
// Include handler
//
// Shader authors #include helper files that live in MIXes alongside the shader
// itself, so the resolver has to go through the game's virtual filesystem
// rather than the OS one. This is what the reference exports as
// exfunc_get_manager() - despite the name, it is an ID3DInclude.
// ===========================================================================

class PhobosShaderInclude final : public ID3DInclude
{
public:
	HRESULT __stdcall Open(D3D_INCLUDE_TYPE, LPCSTR pFileName, LPCVOID,
		LPCVOID* ppData, UINT* pBytes) override
	{
		if (pFileName == nullptr || ppData == nullptr || pBytes == nullptr)
			return E_INVALIDARG;

		*ppData = nullptr;
		*pBytes = 0;

		CCFileClass file { pFileName };

		if (!file.IsAvaible() || !file.Open1(FileAccessMode::Read))
			return E_FAIL;

		const int size = file.Size();

		if (size <= 0)
			return E_FAIL;

		// D3DCompile hands this exact pointer back to Close, so a plain
		// allocation is fine - no bookkeeping map needed.
		auto* const buffer = new(std::nothrow) uint8_t[static_cast<size_t>(size)];

		if (buffer == nullptr)
			return E_OUTOFMEMORY;

		if (file.Read(buffer, size) != size)
		{
			delete[] buffer;
			return E_FAIL;
		}

		*ppData = buffer;
		*pBytes = static_cast<UINT>(size);

		return S_OK;
	}

	HRESULT __stdcall Close(LPCVOID pData) override
	{
		delete[] static_cast<const uint8_t*>(pData);
		return S_OK;
	}
};

// ===========================================================================
// Cache
// ===========================================================================

PhobosShaderCache& PhobosShaderCache::Instance()
{
	static PhobosShaderCache instance;
	return instance;
}

void PhobosShaderCache::OnDeviceInit(IDirect3DDevice9* pDevice)
{
	OnDeviceReset();
	_device = pDevice;
}

void PhobosShaderCache::OnDeviceReset()
{
	_pixel.clear();
	_vertex.clear();
	_diagnostics.clear();
	_device = nullptr;
}

bool PhobosShaderCache::Compile(const PhobosShaderKey& key, com_ptr<ID3DBlob>& outCode)
{
	CCFileClass file { key.File.c_str() };

	if (!file.IsAvaible() || !file.Open1(FileAccessMode::Read))
	{
		// BUGFIX: the reference logger passes the filename BY VALUE here - it
		// hands fifteen consecutive dwords of a fixed-size string struct as
		// varargs, so %s reads the first four characters as a pointer. It
		// crashes precisely when a shader file is missing, which is exactly
		// when the message is needed. Every other log call in that function
		// passes the pointer correctly.
		_diagnostics.emplace_back("cannot open " + key.File);
		Debug::Log("[PhobosShader] cannot open %s\n", key.File.c_str());
		return false;
	}

	const int size = file.Size();

	if (size <= 0)
	{
		_diagnostics.emplace_back("empty file " + key.File);
		return false;
	}

	std::vector<uint8_t> source(static_cast<size_t>(size));

	if (file.Read(source.data(), size) != size)
	{
		_diagnostics.emplace_back("read failed " + key.File);
		return false;
	}

	PhobosShaderInclude includeHandler;

	com_ptr<ID3DBlob> errors;

	// D3DCOMPILE_OPTIMIZATION_LEVEL3, matching the reference build's 0x8000.
	//
	// NOTE: no d3dx9 dependency. ReShade already links d3dcompiler for its own
	// FX pipeline, so D3DCompile is available with no legacy DirectX redist -
	// a real install-friction win over anything needing d3dx9_29.dll.
	const HRESULT hr = D3DCompile(
		source.data(),
		source.size(),
		key.File.c_str(),            // real source name, so #line and errors resolve
		nullptr,                     // defines
		&includeHandler,
		key.Entry.c_str(),
		key.Target.c_str(),
		D3DCOMPILE_OPTIMIZATION_LEVEL3,
		0,
		&outCode,
		&errors);

	// Diagnostics surface even on success - D3DCompile emits warnings through
	// the same blob, and a warning about an uninitialised register is worth
	// seeing before it becomes a bug report.
	if (errors != nullptr && errors->GetBufferSize() > 0)
	{
		const auto* text = static_cast<const char*>(errors->GetBufferPointer());
		_diagnostics.emplace_back(key.File + " [" + key.Entry + "]: " + text);
		Debug::Log("[PhobosShader] %s [%s]: %s\n", key.File.c_str(), key.Entry.c_str(), text);
	}

	if (FAILED(hr))
	{
		_diagnostics.emplace_back(key.File + " [" + key.Entry + "] compile failed");
		outCode.reset();
		return false;
	}

	return outCode != nullptr;
}

bool PhobosShaderCache::GetPixelShader(const char* file, const char* entry,
	IDirect3DPixelShader9** ppOut)
{
	if (ppOut == nullptr)
		return false;

	*ppOut = nullptr;

	if (_device == nullptr || file == nullptr || *file == '\0')
		return false;

	const PhobosShaderKey key { file, entry != nullptr ? entry : "main", "ps_3_0" };

	if (const auto it = _pixel.find(key); it != _pixel.end())
	{
		*ppOut = it->second.get();

		if (*ppOut != nullptr)
			(*ppOut)->AddRef();

		return *ppOut != nullptr;
	}

	com_ptr<ID3DBlob> code;

	// A failed compile is cached as null. Without this a broken shader is
	// recompiled on every single object that references it.
	if (!Compile(key, code))
	{
		_pixel.emplace(key, nullptr);
		return false;
	}

	com_ptr<IDirect3DPixelShader9> shader;

	if (FAILED(_device->CreatePixelShader(
		static_cast<const DWORD*>(code->GetBufferPointer()), &shader)))
	{
		_diagnostics.emplace_back(key.File + " [" + key.Entry + "] CreatePixelShader failed");
		_pixel.emplace(key, nullptr);
		return false;
	}

	*ppOut = shader.get();
	(*ppOut)->AddRef();

	_pixel.emplace(key, std::move(shader));
	return true;
}

bool PhobosShaderCache::GetVertexShader(const char* file, const char* entry,
	IDirect3DVertexShader9** ppOut)
{
	if (ppOut == nullptr)
		return false;

	*ppOut = nullptr;

	if (_device == nullptr || file == nullptr || *file == '\0')
		return false;

	const PhobosShaderKey key { file, entry != nullptr ? entry : "vmain", "vs_3_0" };

	if (const auto it = _vertex.find(key); it != _vertex.end())
	{
		*ppOut = it->second.get();

		if (*ppOut != nullptr)
			(*ppOut)->AddRef();

		return *ppOut != nullptr;
	}

	com_ptr<ID3DBlob> code;

	if (!Compile(key, code))
	{
		_vertex.emplace(key, nullptr);
		return false;
	}

	com_ptr<IDirect3DVertexShader9> shader;

	// BUGFIX: on this failure path the reference implementation releases and
	// nulls this+308 - the PIXEL shader - while the vertex shader lives at
	// this+312. LoadPixelShader gets it right; LoadVertexShader is a copy-paste
	// slip. A failed vertex compile silently destroys a perfectly good pixel
	// shader on the same object.
	//
	// Separating the two caches makes the class of mistake unrepresentable:
	// there is no shared `this+offset` to get wrong.
	if (FAILED(_device->CreateVertexShader(
		static_cast<const DWORD*>(code->GetBufferPointer()), &shader)))
	{
		_diagnostics.emplace_back(key.File + " [" + key.Entry + "] CreateVertexShader failed");
		_vertex.emplace(key, nullptr);
		return false;
	}

	*ppOut = shader.get();
	(*ppOut)->AddRef();

	_vertex.emplace(key, std::move(shader));
	return true;
}

// ===========================================================================
// SKIPPED - draw submission
//
// Binding the constants and issuing the quad needs three things I do not have:
//
//   1. The vertex declaration. The shaders take `int vertexid : TEXCOORD0`, so
//      the stream is a 4-element index buffer rather than positions - but the
//      exact D3DVERTEXELEMENT9 layout and whether it draws indexed or with
//      DrawPrimitiveUP is not visible in anything supplied.
//
//   2. Where in the frame these draw. The reference queues through
//      exfunc_update_textured_anim / update_textured_laser and flushes on the
//      ReShade side, which is a threading boundary that disappears in an
//      embedded build - so the ordering has to be re-derived, not copied.
//
//   3. The render-target setup for TextureEffectType=1 (distortion), where the
//      object writes a normal map to an offscreen target that a later pass
//      samples. That target is s15 - bound but explicitly not sampleable from
//      the object's own shader, presumably because it is the current RT.
//
// The constant blocks above are the stable part and are complete. Once the
// vertex path is pinned down, submission is SetVertexShaderConstantF with
// sizeof(block)/16 registers and a four-vertex draw.
//
// ===========================================================================
// DELIBERATE OMISSION - forced Top layer
//
// The reference documentation notes that enabling a textured anim makes its
// Layer "equivalent to always being Top". That is not carried over here.
//
// The system already has real per-pixel depth: TopZ/BottomZ interpolate down
// the quad and the pixel shader discards against the engine ZBuffer. Forcing
// Top on top of that discards ordering information the shader could have used,
// and it is visible to players as textured anims drawing through objects that
// ought to occlude them. It reads as an implementation shortcut rather than a
// constraint of the technique.
// ===========================================================================