#pragma once

#include "Effect.h"
#include "D3DCache.h"
#include "D3DHelper.h"
#include "Shader.h"

namespace dx
{
	class PBREffect : public Effect
	{
	public:
		union Options
		{
			struct Bits
			{
				bool hasTexcoords : 1;
				bool hasTangents : 1;
				bool useColorMap : 1;
				bool useOcclusionMap : 1;
				bool useRoughnessMap : 1;
				bool useMetalnessMap : 1;
				bool useNormalMap : 1;
			};

			Bits bits;
			uint32_t key;

			Options() : key(0) { }
		};

		// Configurable resources
		struct Resources
		{
			// Global list of lights
			winrt::com_ptr<ID3D11ShaderResourceView> lights;

			// Textures
			winrt::com_ptr<ID3D11ShaderResourceView> color;
			winrt::com_ptr<ID3D11ShaderResourceView> orm;
			winrt::com_ptr<ID3D11ShaderResourceView> normal;

			winrt::com_ptr<ID3D11ShaderResourceView> cascades;
		};

		// Per-material constants
#pragma pack(16)
		struct Constants
		{
			float roughnessFactor;
			float metallicFactor;
			float padding0[2];
			DirectX::XMFLOAT4 baseColorFactor;
		};
#pragma pack()

		Resources resources;

		Constants& GetConstants()
		{
			m_cbDirty = true;
			return m_constants.data;
		}

		PBREffect(ID3D11Device* pDevice, Options options) : 
			m_options(options),
			m_constants(pDevice),
			m_cbDirty(true)
		{
			auto defines = GetDefines(options);
			m_pVS = CreateVertexShader(pDevice, "Source/Shaders/PBR.vs.hlsl", defines, options.key);
			m_pPS = CreatePixelShader(pDevice, "Source/Shaders/PBR.ps.hlsl", defines, options.key);
		}

		void Bind(ID3D11DeviceContext* pContext) const override
		{
			if (m_cbDirty)
			{
				m_constants.Update(pContext);
				m_cbDirty = false;
			}
			
			m_pVS->Bind(pContext);
			m_pPS->Bind(pContext);
			
			BindShaderResourcesPS(pContext, 0, resources.lights.get());
			m_constants.BindPS(pContext, 2);

			if (m_options.bits.useColorMap)
			{
				BindShaderResourcesPS(pContext, 1, resources.color.get());
			}
			if (m_options.bits.useOcclusionMap || m_options.bits.useRoughnessMap || m_options.bits.useMetalnessMap)
			{
				BindShaderResourcesPS(pContext, 2, resources.orm.get());
			}
			if (m_options.bits.useNormalMap)
			{
				BindShaderResourcesPS(pContext, 3, resources.normal.get());
			}
			BindShaderResourcesPS(pContext, 4, resources.cascades.get());
		}
		
	private:
		Options m_options;
		ConstantBuffer<Constants> m_constants;
		mutable bool m_cbDirty;

		// Resources that cannot be configured by the user

		std::shared_ptr<VertexShader> m_pVS;
		std::shared_ptr<PixelShader> m_pPS;

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
			if (options.bits.useColorMap)
			{
				defines.push_back({ "USE_COLOR_MAP", "1" });
			}
			if (options.bits.useOcclusionMap)
			{
				defines.push_back({ "USE_OCCLUSION_MAP", "1" });
			}
			if (options.bits.useRoughnessMap)
			{
				defines.push_back({ "USE_ROUGHNESS_MAP", "1" });
			}
			if (options.bits.useMetalnessMap)
			{
				defines.push_back({ "USE_METALNESS_MAP", "1" });
			}
			if (options.bits.useNormalMap)
			{
				defines.push_back({ "USE_NORMAL_MAP", "1" });
			}
			return defines;
		}
	};
}