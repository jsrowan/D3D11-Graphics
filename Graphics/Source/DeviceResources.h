#pragma once

#include "D3DCache.h"

namespace dx
{
	class DeviceResources
	{
	public:
		DeviceResources(HWND hWnd);

		DeviceResources(const DeviceResources& other) = delete;
		DeviceResources& operator=(const DeviceResources& other) = delete;

		void Resize(int width, int height);

		// Accessors
		std::pair<unsigned int, unsigned int> GetSize() const 
		{
			return std::pair(m_backBufferDesc.Width, m_backBufferDesc.Height);
		}

		ID3D11Device* GetDevice() const
		{
			return m_pDevice.get();
		}

		ID3D11DeviceContext* GetContext() const
		{
			return m_pContext.get();
		}

		ID3D11RenderTargetView* GetRenderTarget() const
		{
			return m_pRenderTarget.get();
		}

		ID3D11DepthStencilView* GetDepthStencil() const
		{
			return m_pDepthStencil.get();
		}

		const D3D11_VIEWPORT& GetViewport() const
		{
			return m_viewport;
		}

		void Present();

	private:
		winrt::com_ptr<ID3D11Device> m_pDevice;
		winrt::com_ptr<ID3D11DeviceContext> m_pContext;
		winrt::com_ptr<IDXGISwapChain> m_pSwapChain;

		winrt::com_ptr<ID3D11RenderTargetView> m_pRenderTarget;
		winrt::com_ptr<ID3D11DepthStencilView> m_pDepthStencil;

		D3D_FEATURE_LEVEL m_featureLevel;
		D3D11_TEXTURE2D_DESC m_backBufferDesc;
		D3D11_VIEWPORT m_viewport;
		DXGI_ADAPTER_DESC m_adapterDesc;

		void ConfigureBackBuffer();
	};
}