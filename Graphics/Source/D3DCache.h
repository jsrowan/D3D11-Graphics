#pragma once

#include "Shader.h"

namespace dx
{
	// Cached resource loading methods
	std::shared_ptr<VertexShader> CreateVertexShader(ID3D11Device* pDevice, const std::string& filename,
		const std::vector<std::pair<std::string, std::string>>& defines = {}, uint64_t optionsKey = 0);
	std::shared_ptr<PixelShader> CreatePixelShader(ID3D11Device* pDevice, const std::string& filename,
		const std::vector<std::pair<std::string, std::string>>& defines = {}, uint64_t optionsKey = 0);
	std::shared_ptr<ComputeShader> CreateComputeShader(ID3D11Device* pDevice, const std::string& filename,
		const std::vector<std::pair<std::string, std::string>>& defines = {}, uint64_t optionsKey = 0);
	winrt::com_ptr<ID3D11SamplerState> CreateSamplerState(ID3D11Device* pDevice,
		const D3D11_SAMPLER_DESC& desc);
	winrt::com_ptr<ID3D11RasterizerState> CreateRasterizerState(ID3D11Device* pDevice,
		const D3D11_RASTERIZER_DESC& desc);
	winrt::com_ptr<ID3D11DepthStencilState> CreateDepthStencilState(ID3D11Device* pDevice,
		const D3D11_DEPTH_STENCIL_DESC& desc);
	winrt::com_ptr<ID3D11BlendState> CreateBlendState(ID3D11Device* pDevice,
		const D3D11_BLEND_DESC& desc);
	winrt::com_ptr<ID3D11ShaderResourceView> CreateTexture(ID3D11Device* pDevice,
		const std::string& filename);

	class D3DCache
	{
	public:
		D3DCache() = default;

		// Shared resource creation methods
		void CreateTexture2D(ID3D11Device* pDevice, const std::string& name, const D3D11_TEXTURE2D_DESC& desc);
		// Add a resource that has already been created elsewhere, eg. a Buffer
		void AddResource(const std::string& name, ID3D11Resource* pResource);

		// Shared view creation methods
		void AddShaderResourceView(ID3D11Device* pDevice, const std::string& resourceName,
			const std::string& viewName, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc);
		void AddShaderResourceView(ID3D11Device* pDevice, const std::string& resourceName,
			const std::string& viewName);
		void AddUnorderedAccessView(ID3D11Device* pDevice, const std::string& resourceName,
			const std::string& viewName, const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc);
		void AddUnorderedAccessView(ID3D11Device* pDevice, const std::string& resourceName,
			const std::string& viewName);
		void AddDepthStencilView(ID3D11Device* pDevice, const std::string& resourceName,
			const std::string& viewName, const D3D11_DEPTH_STENCIL_VIEW_DESC& desc);
		void AddDepthStencilView(ID3D11Device* pDevice, const std::string& resourceName,
			const std::string& viewName);
		void AddRenderTargetView(ID3D11Device* pDevice, const std::string& resourceName,
			const std::string& viewName, const D3D11_RENDER_TARGET_VIEW_DESC& desc);
		void AddRenderTargetView(ID3D11Device* pDevice, const std::string& resourceName,
			const std::string& viewName);

		// View Getter Methods
		winrt::com_ptr<ID3D11ShaderResourceView> GetShaderResourceView(const std::string& name) const;
		winrt::com_ptr<ID3D11UnorderedAccessView> GetUnorderedAccessView(const std::string& name) const;
		winrt::com_ptr<ID3D11DepthStencilView> GetDepthStencilView(const std::string& name) const;
		winrt::com_ptr<ID3D11RenderTargetView> GetRenderTargetView(const std::string& name) const;	

	private:
		// Shared resource views created by the application - lookup by name
		std::unordered_map<std::string, winrt::com_ptr<ID3D11ShaderResourceView>> m_srvTable;
		std::unordered_map<std::string, winrt::com_ptr<ID3D11DepthStencilView>> m_dsvTable;
		std::unordered_map<std::string, winrt::com_ptr<ID3D11RenderTargetView>> m_rtvTable;
		std::unordered_map<std::string, winrt::com_ptr<ID3D11UnorderedAccessView>> m_uavTable;

		// Resources these views are derived from 
		std::unordered_map<std::string, winrt::com_ptr<ID3D11Resource>> m_resourceTable;
	};
}