#pragma once

#include "Effect.h"
#include "D3DCache.h"
#include "D3DHelper.h"

namespace dx
{
	class RenderFromTextureEffect : public Effect
	{
	public:
		struct Resources
		{
			winrt::com_ptr<ID3D11ShaderResourceView> inputTexture;
		};

		Resources resources;

		RenderFromTextureEffect(ID3D11Device* pDevice)
		{
			m_pVS = CreateVertexShader(pDevice, "Source/Shaders/FullScreenTriangle.vs.hlsl");
			m_pPS = CreatePixelShader(pDevice, "Source/Shaders/RenderFromTexture.ps.hlsl");
		}

		void Bind(ID3D11DeviceContext* pContext) const override
		{
			m_pVS->Bind(pContext);
			m_pPS->Bind(pContext);
			BindShaderResourcesPS(pContext, 0, resources.inputTexture.get());
		}

	private:
		std::shared_ptr<VertexShader> m_pVS;
		std::shared_ptr<PixelShader> m_pPS;
	};
}
