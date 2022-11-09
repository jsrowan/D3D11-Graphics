#include "stdafx.h"

#include "DeviceResources.h"

using winrt::com_ptr;
using winrt::check_hresult;

namespace dx
{
	DeviceResources::DeviceResources(HWND hWnd)
	{
		// Required D3D feature level
		constexpr D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_1;

		unsigned int deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		check_hresult(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags, &level, 1,
			D3D11_SDK_VERSION, m_pDevice.put(), &m_featureLevel, m_pContext.put()));

		com_ptr<IDXGIAdapter> pAdapter;
		com_ptr<IDXGIFactory> pFactory;
		auto pDXGIDevice = m_pDevice.as<IDXGIDevice3>();
		check_hresult(pDXGIDevice->GetAdapter(pAdapter.put()));
		check_hresult(pAdapter->GetDesc(&m_adapterDesc));
		check_hresult(pAdapter->GetParent(IID_PPV_ARGS(pFactory.put())));

		DXGI_SWAP_CHAIN_DESC scDesc{};
		scDesc.Windowed = true;
		scDesc.BufferCount = 2;
		scDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.SampleDesc.Count = 1;
		scDesc.SampleDesc.Quality = 0;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		scDesc.OutputWindow = hWnd;

		check_hresult(pFactory->CreateSwapChain(m_pDevice.get(), &scDesc, m_pSwapChain.put()));
		check_hresult(pFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

		// Print device info
		std::wcout << m_adapterDesc.Description << "\n";

		ConfigureBackBuffer();
	}

	// Create back buffer resources, either on program start or when the window size is changed.
	void DeviceResources::ConfigureBackBuffer()
	{
		// Create render target
		winrt::com_ptr<ID3D11Texture2D> pBackBuffer;
		check_hresult(m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.put())));
		pBackBuffer->GetDesc(&m_backBufferDesc);
		check_hresult(m_pDevice->CreateRenderTargetView(pBackBuffer.get(), nullptr, m_pRenderTarget.put()));

		// Create depth stencil target
		D3D11_TEXTURE2D_DESC dsDesc{};
		dsDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		dsDesc.Width = m_backBufferDesc.Width;
		dsDesc.Height = m_backBufferDesc.Height;
		dsDesc.MipLevels = 1;
		dsDesc.ArraySize = 1;
		dsDesc.SampleDesc.Count = 1;
		dsDesc.SampleDesc.Quality = 0;
		dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		winrt::com_ptr<ID3D11Texture2D> pDepthStencil;
		check_hresult(m_pDevice->CreateTexture2D(&dsDesc, nullptr, pDepthStencil.put()));
		check_hresult(m_pDevice->CreateDepthStencilView(pDepthStencil.get(), nullptr, m_pDepthStencil.put()));

		// Create viewport
		m_viewport = CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_backBufferDesc.Width),
			static_cast<float>(m_backBufferDesc.Height), 0.0f, 1.0f);
		m_pContext->RSSetViewports(1, &m_viewport);
	}

	// Resize the window
	void DeviceResources::Resize(int width, int height)
	{
		// Release all resources held by the back buffer
		m_pRenderTarget = nullptr;
		m_pDepthStencil = nullptr;
		m_pContext->Flush();
		check_hresult(m_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));
		ConfigureBackBuffer();
	}

	void DeviceResources::Present()
	{
		m_pSwapChain->Present(1, 0);
	}
}
