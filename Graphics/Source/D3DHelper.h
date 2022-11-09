#pragma once

#include "Buffers.h"

namespace dx
{
#pragma pack(16)
	struct PerFrameConstants
	{
		int nlights;							// Number of dynamic lights in the scene
		DirectX::XMFLOAT3 eye;					// Camera eye position
		DirectX::XMFLOAT4X4 viewProj;			// Premultiplied view-projection matrix
		DirectX::XMFLOAT4X4 view;				// View matrix
		DirectX::XMFLOAT4X4 lightViewProj[3];	// For cascaded shadow mapping
		float cascadeSplits[3];					// View space depth splits for shadow cascades
		float pad;
	};
#pragma pack()

#pragma pack(16)
	struct PerObjectConstants
	{
		DirectX::XMFLOAT4X4 model;				// Model matrix
	};
#pragma pack()

	class CommonSamplerStates
	{
	public:
		CommonSamplerStates(ID3D11Device* pDevice);

		ID3D11SamplerState* LinearWrap() const { return m_pLinearWrap.get(); }
		ID3D11SamplerState* LinearClamp() const { return m_pLinearClamp.get(); }
		ID3D11SamplerState* AnisotropicWrap() const { return m_pAnisotropicWrap.get(); }
		ID3D11SamplerState* AnisotropicClamp() const { return m_pAnisotropicClamp.get(); }
		ID3D11SamplerState* PointWrap() const { return m_pPointWrap.get(); }
		ID3D11SamplerState* PointClamp() const { return m_pPointClamp.get(); }
		ID3D11SamplerState* PointComp() const { return m_pPointComp.get(); }
		ID3D11SamplerState* LinearComp() const { return m_pLinearComp.get(); }

		static constexpr D3D11_SAMPLER_DESC LinearWrapDesc();
		static constexpr D3D11_SAMPLER_DESC LinearClampDesc();
		static constexpr D3D11_SAMPLER_DESC AnisotropicWrapDesc();
		static constexpr D3D11_SAMPLER_DESC AnisotropicClampDesc();
		static constexpr D3D11_SAMPLER_DESC PointWrapDesc();
		static constexpr D3D11_SAMPLER_DESC PointClampDesc();
		static constexpr D3D11_SAMPLER_DESC PointCompDesc();
		static constexpr D3D11_SAMPLER_DESC LinearCompDesc();

	private:
		winrt::com_ptr<ID3D11SamplerState> m_pLinearWrap;
		winrt::com_ptr<ID3D11SamplerState> m_pLinearClamp;
		winrt::com_ptr<ID3D11SamplerState> m_pAnisotropicWrap;
		winrt::com_ptr<ID3D11SamplerState> m_pAnisotropicClamp;
		winrt::com_ptr<ID3D11SamplerState> m_pPointWrap;
		winrt::com_ptr<ID3D11SamplerState> m_pPointClamp;
		winrt::com_ptr<ID3D11SamplerState> m_pPointComp;
		winrt::com_ptr<ID3D11SamplerState> m_pLinearComp;
	};

	class CommonRasterizerStates
	{
	public:
		CommonRasterizerStates(ID3D11Device* pDevice);

		ID3D11RasterizerState* CullNone() const { return m_pCullNone.get(); }
		ID3D11RasterizerState* CullFront() const { return m_pCullFront.get(); }
		ID3D11RasterizerState* CullBack() const { return m_pCullBack.get(); }
		ID3D11RasterizerState* Wireframe() const { return m_pWireframe.get(); }

		static constexpr D3D11_RASTERIZER_DESC CullNoneDesc();
		static constexpr D3D11_RASTERIZER_DESC CullFrontDesc();
		static constexpr D3D11_RASTERIZER_DESC CullBackDesc();
		static constexpr D3D11_RASTERIZER_DESC WireframeDesc();

	private:
		winrt::com_ptr<ID3D11RasterizerState> m_pCullNone;
		winrt::com_ptr<ID3D11RasterizerState> m_pCullFront;
		winrt::com_ptr<ID3D11RasterizerState> m_pCullBack;
		winrt::com_ptr<ID3D11RasterizerState> m_pWireframe;
	};

	class CommonBlendStates
	{
	public:
		CommonBlendStates(ID3D11Device* pDevice);

		ID3D11BlendState* Disabled() const { return m_pDisabled.get(); }
		ID3D11BlendState* Alpha() const { return m_pAlpha.get(); }
		ID3D11BlendState* PremultipliedAlpha() const { return m_pPremultipliedAlpha.get(); }
		ID3D11BlendState* Additive() const { return m_pAdditive.get(); }
		ID3D11BlendState* ColorWriteDisabled() const { return m_pColorWriteDisabled.get(); }

		static constexpr D3D11_BLEND_DESC DisabledDesc();
		static constexpr D3D11_BLEND_DESC AlphaDesc();
		static constexpr D3D11_BLEND_DESC PremultipliedAlphaDesc();
		static constexpr D3D11_BLEND_DESC AdditiveDesc();
		static constexpr D3D11_BLEND_DESC ColorWriteDisabledDesc();

	private:
		winrt::com_ptr<ID3D11BlendState> m_pDisabled;
		winrt::com_ptr<ID3D11BlendState> m_pAlpha;
		winrt::com_ptr<ID3D11BlendState> m_pPremultipliedAlpha;
		winrt::com_ptr<ID3D11BlendState> m_pAdditive;
		winrt::com_ptr<ID3D11BlendState> m_pColorWriteDisabled;
	};

	class CommonDepthStencilStates
	{
	public:
		CommonDepthStencilStates(ID3D11Device* pDevice);

		ID3D11DepthStencilState* DepthDisabled() const { return m_pDepthDisabled.get(); }
		ID3D11DepthStencilState* DepthEnabled() const { return m_pDepthEnabled.get(); }
		ID3D11DepthStencilState* DepthEnabledWrite() const { return m_pDepthEnabledWrite.get(); }

		static constexpr D3D11_DEPTH_STENCIL_DESC DepthDisabledDesc();
		static constexpr D3D11_DEPTH_STENCIL_DESC DepthEnabledDesc();
		static constexpr D3D11_DEPTH_STENCIL_DESC DepthEnabledWriteDesc();

	private:
		winrt::com_ptr<ID3D11DepthStencilState> m_pDepthDisabled;
		winrt::com_ptr<ID3D11DepthStencilState> m_pDepthEnabled;
		winrt::com_ptr<ID3D11DepthStencilState> m_pDepthEnabledWrite;
	};

	class D3DHelper
	{
	public:
		D3DHelper(ID3D11Device* pDevice);

		ConstantBuffer<PerFrameConstants> cbPerFrame;
		ConstantBuffer<PerObjectConstants> cbPerObject;

		const CommonSamplerStates& SamplerStates() const { return m_samplerStates; }
		const CommonRasterizerStates& RasterizerStates() const { return m_rasterizerStates; }
		const CommonBlendStates& BlendStates() const { return m_blendStates; }
		const CommonDepthStencilStates& DepthStencilStates() const { return m_depthStencilStates; }

		// Common constant buffers and samplers can be bound once at startup as an optimization.
		// They are defined for all shaders in Common.hlsli. Still allow them to be accessed 
		// individually in case they need to be bound to another as well slot for some reason.
		void BindConstantBuffers(ID3D11DeviceContext* pContext) const;
		void BindSamplers(ID3D11DeviceContext* pContext) const;

	private:
		CommonSamplerStates m_samplerStates;
		CommonRasterizerStates m_rasterizerStates;
		CommonBlendStates m_blendStates;
		CommonDepthStencilStates m_depthStencilStates;
	};
}