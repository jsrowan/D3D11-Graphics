#pragma once

#include "Effect.h"

namespace dx
{
	class ShadowMapEffect : public Effect
	{
	public:
		union Options
		{
			struct Bits
			{
				bool hasTexcoords : 1;
				bool hasTangents : 1;
			};

			Bits bits;
			uint32_t key;

			Options() : key(0) { }
		};

		ShadowMapEffect(ID3D11Device* pDevice, Options options) :
			m_options(options)
		{
			auto defines = GetDefines(options);
			m_pVS = CreateVertexShader(pDevice, "Source/Shaders/ShadowMap.vs.hlsl", defines, options.key);
		}

		void Bind(ID3D11DeviceContext* pContext) const override
		{
			m_pVS->Bind(pContext);
			// Bind a null pixel shader
			SetPS(pContext, nullptr);
		}

	private:
		Options m_options;

		std::shared_ptr<VertexShader> m_pVS;

		static std::vector<std::pair<std::string, std::string>> GetDefines(Options options)
		{
			std::vector<std::pair<std::string, std::string>> defines;
			if (options.bits.hasTexcoords)
			{
				defines.push_back({ "HAS_TEXCOORDS", "1" });
			}
			if (options.bits.hasTangents)
			{
				defines.push_back({ "HAS_TANGENTS", "1" });
			}
			return defines;
		}
	};
}