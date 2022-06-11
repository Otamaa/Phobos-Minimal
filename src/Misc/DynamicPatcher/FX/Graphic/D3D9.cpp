#include "D3D9.h"

bool Extended_D3D9::_inited = false;
com_ptr<IDirect3DVertexBuffer9> Extended_D3D9::vertices = nullptr;
void* Extended_D3D9::vertexDecl = nullptr;
void* Extended_D3D9::additionalTexture = nullptr;
void* Extended_D3D9::sharedTexture = nullptr;

bool FXGraphic::RenderToNewWindow = false;
std::string FXGraphic::GraphicDirectory = "Graphic";
std::string FXGraphic::ShaderDirectory = "Shaders";
std::string FXGraphic::TextureDirectory = "Textures";

com_ptr<ID3D11DeviceContext> FXGraphic::d3dImmediateContext = nullptr;
com_ptr<IDXGISwapChain> FXGraphic::swapChain = nullptr;
com_ptr<IDXGIDevice> FXGraphic::d3dDevice = nullptr;
com_ptr<ID3D11Texture2D> FXGraphic::_backBuffer = nullptr;
com_ptr<ID3D11Texture2D> FXGraphic::_renderBuffer = nullptr;
com_ptr<ID3D11RenderTargetView> FXGraphic::_renderView = nullptr;
com_ptr<ID3D11Texture2D> FXGraphic::_renderBuffer = nullptr;