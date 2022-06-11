#pragma once
#include "../Definitions/Transform.h"

#include <Surface.h>
#include <string>
#include <d3d9.h>
#include "../com_ptr.h"
#include "FXGraphic.h"

enum class LockFlags : DWORD {
	//
// Summary:
//     None.
	None = 0,
	//
	// Summary:
	//     No documentation.
	DoNotCopyData = 1,
	//
	// Summary:
	//     No documentation.
	ReadOnly = 16,
	//
	// Summary:
	//     No documentation.
	NoSystemLock = 2048,
	//
	// Summary:
	//     No documentation.
	NoOverwrite = 4096,
	//
	// Summary:
	//     No documentation.
	Discard = 8192,
	//
	// Summary:
	//     No documentation.
	DoNotWait = 16384,
	//
	// Summary:
	//     No documentation.
	NoDirtyUpdate = 32768

};

enum class ShaderFlags
{
	//
	// Summary:
	//     No documentation.
	OptimizationLevel1 = 0,
	//
	// Summary:
	//     None.
	None = 0,
	//
	// Summary:
	//     No documentation.
	Debug = 1,
	//
	// Summary:
	//     No documentation.
	SkipValidation = 2,
	//
	// Summary:
	//     No documentation.
	SkipOptimization = 4,
	//
	// Summary:
	//     No documentation.
	PackMatrixRowMajor = 8,
	//
	// Summary:
	//     No documentation.
	PackMatrixColumnMajor = 16,
	//
	// Summary:
	//     No documentation.
	PartialPrecision = 32,
	//
	// Summary:
	//     No documentation.
	ForceVSSoftwareNoOpt = 64,
	//
	// Summary:
	//     No documentation.
	ForcePSSoftwareNoOpt = 128,
	//
	// Summary:
	//     No documentation.
	NoPreshader = 256,
	//
	// Summary:
	//     No documentation.
	AvoidFlowControl = 512,
	//
	// Summary:
	//     No documentation.
	PreferFlowControl = 1024,
	//
	// Summary:
	//     No documentation.
	EnableBackwardsCompatibility = 4096,
	//
	// Summary:
	//     No documentation.
	IeeeStrictness = 8192,
	//
	// Summary:
	//     No documentation.
	OptimizationLevel0 = 16384,
	//
	// Summary:
	//     No documentation.
	OptimizationLevel3 = 32768,
	//
	// Summary:
	//     No documentation.
	OptimizationLevel2 = 49152,
	//
	// Summary:
	//     No documentation.
	UseLegacyD3DX9_31Dll = 65536
};

class Extended_D3D9
{
	static bool _inited;
	static com_ptr<IDirect3DVertexBuffer9> vertices;
	static void* effect;
	static void* vertexDecl;
	static void* additionalTexture;
	static void* sharedTexture;

public:
	static void Initialize(IDirect3DDevice9* device)
	{
		HANDLE *shared_handle = nullptr;
		D3DVERTEXBUFFER_DESC internal_desc = {};
		internal_desc.Type = D3DRESOURCETYPE::D3DRTYPE_TEXTURE;
		internal_desc.Format = D3DFORMAT::D3DFMT_UNKNOWN;
		internal_desc.Size = static_cast<UINT>(sizeof(FloatVector4) * 2 * 4);
		internal_desc.Usage = 0;
		internal_desc.FVF = 0;
		internal_desc.Pool = D3DPOOL::D3DPOOL_MANAGED;

		if (SUCCEEDED(device->CreateVertexBuffer(internal_desc.Size, internal_desc.Usage, internal_desc.FVF, internal_desc.Pool, &vertices, shared_handle)))
		{
			void* pData = nullptr;
			vertices->Lock(0, 0, &pData, (DWORD)LockFlags::None);
			FloatVector4 Vec4[] = { FloatVector4(-1.0f, 1.0f, 0.0f, 0.0f),FloatVector4(0.0f, 0.0f, 0.0f, 0.0f),
										  FloatVector4(1.0f, 1.0f, 0.0f, 0.0f), FloatVector4(1.0f, 0.0f, 0.0f, 0.0f),
										FloatVector4(-1.0f, -1.0f, 0.0f, 0.0f), FloatVector4(0.0f, 1.0f, 0.0f, 0.0f),
										FloatVector4(1.0f, -1.0f, 0.0f, 0.0f), FloatVector4(1.0f, 1.0f, 0.0f, 0.0f) };
			pData = &Vec4;
			vertices->Unlock();
		}



		effect = Effect.FromString(device, D3D9_shader, ShaderFlags.None);

		VertexElement[] vertexElems = new[] {
			new VertexElement(0, 0, DeclarationType.Float4, DeclarationMethod.Default, DeclarationUsage.Position, 0),
				new VertexElement(0, 16, DeclarationType.Float4, DeclarationMethod.Default, DeclarationUsage.Color, 0),
				VertexElement.VertexDeclarationEnd
		};

		// Creates and sets the Vertex Declaration
		vertexDecl = new VertexDeclaration(device, vertexElems);

		var rect = YRGraphic.SurfaceRect;
		additionalTexture = new Texture(device, rect.Width, rect.Height, 1, Usage.Dynamic, Format.A8B8G8R8, Pool.Default);

		//using (var res = FXGraphic.BackBuffer.QueryInterface<SharpDX.DXGI.Resource>())
		//{
		//    var sharedHandle = res.SharedHandle;
		//    sharedTexture = new Texture(device, rect.Width, rect.Height, 1, Usage.RenderTarget , Format.A8B8G8R8, Pool.Default, ref sharedHandle);
		//}
	}
	public static void Dispose()
	{
		vertices ? .Dispose();
		effect ? .Dispose();
		vertexDecl ? .Dispose();
		additionalTexture ? .Dispose();
		sharedTexture ? .Dispose();
	}
	public static void CopyBuffer()
	{
		var rect = YRGraphic.SurfaceRect;

		var map9 = additionalTexture.LockRectangle(0, LockFlags.None);
		var map11 = FXGraphic.ImmediateContext.MapSubresource(FXGraphic.BackBuffer, 0, SharpDX.Direct3D11.MapMode.Read, SharpDX.Direct3D11.MapFlags.None);

		PatcherYRpp.Pointer<byte> dst = map9.DataPointer;
		PatcherYRpp.Pointer<byte> src = map11.DataPointer;

		for (int y = 0; y < rect.Height; y++)
		{
			SharpDX.Utilities.CopyMemory(dst, src, rect.Width * 4);
			dst += map9.Pitch;
			src += map11.RowPitch;
		}

		FXGraphic.ImmediateContext.UnmapSubresource(FXGraphic.BackBuffer, 0);
		additionalTexture.UnlockRectangle(0);
	}

	
	static void CopyBuffer(IDirect3DDevice9* device)
	{
		auto rect = DSurface::Primary->Get_Rect();
		com_ptr<IDirect3DSurface9> surface = nullptr;
		HANDLE *shared_handle = nullptr;

		if (SUCCEEDED(device->CreateOffscreenPlainSurface(rect.Width, rect.Height, D3DFORMAT::D3DFMT_R5G6B5, D3DPOOL::D3DPOOL_SYSTEMMEM, &surface, shared_handle)))
		{
			com_ptr<IDirect3DSurface9> DeviceSurface = nullptr;
			if (SUCCEEDED(device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE::D3DBACKBUFFER_TYPE_FORCE_DWORD, &DeviceSurface))) {
				device->GetRenderTargetData(DeviceSurface.get(), surface.get());

					auto map9 = surface->LockRect(nullptr,nullptr,(DWORD)LockFlags::ReadOnly);
					auto map11 = FXGraphic.ImmediateContext.MapSubresource(YRGraphic.BufferTextureView.Resource, 0, SharpDX.Direct3D11.MapMode.WriteDiscard, SharpDX.Direct3D11.MapFlags.None);

					PatcherYRpp.Pointer<byte> dst = map11.DataPointer;
					PatcherYRpp.Pointer<byte> src = map9.DataPointer;

					for (int y = 0; y < rect.Height; y++)
					{
						SharpDX.Utilities.CopyMemory(dst, src, rect.Width * 2);
						dst += map11.RowPitch;
						src += map9.Pitch;
					}

				FXGraphic.ImmediateContext.UnmapSubresource(YRGraphic.BufferTextureView.Resource, 0);
				surface.UnlockRectangle();
				surface.Dispose();
			}
		}
	}

	static void OnPresent(IDirect3DDevice9* device)
	{
		CopyBuffer(device);
	}

	static std::string D3D9_shader
	{
		"sampler additional_sampler : register(s0);                     " +
		"                                                               " +
		"struct VS_IN                                                   " +
		"{                                                              " +
		"    float4 pos : POSITION;                                     " +
		"	float4 uv : TEXCOORD;                                       " +
		"};                                                             " +
		"                                                               " +
		"struct PS_IN                                                   " +
		"{                                                              " +
		"    float4 pos : POSITION;                                     " +
		"	float4 uv : TEXCOORD;                                       " +
		"};                                                             " +
		"                                                               " +
		"PS_IN VS(VS_IN input)                                          " +
		"{                                                              " +
		"    PS_IN output = (PS_IN)0;                                   " +
		"                                                               " +
		"    output.pos = input.pos;                                    " +
		"    output.uv = input.uv;                                      " +
		"                                                               " +
		"    return output;                                             " +
		"}                                                              " +
		"                                                               " +
		"float4 PS(PS_IN input) : COLOR                                 " +
		"{                                                              " +
		"    return tex2D(additional_sampler, input.uv).rgba;           " +
		"}                                                              " +
		"                                                               " +
		"technique Main                                                 " +
		"{                                                              " +
		"    pass P0                                                    " +
		"    {                                                          " +
		"        VertexShader = compile vs_3_0 VS();                    " +
		"        PixelShader = compile ps_3_0 PS();                     " +
		"    }                                                          " +
		"}                                                              " +
		"                                                               " +
		"                                                               " }
	;
};