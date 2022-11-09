#include "stdafx.h"

#include "D3DHelper.h"

#include "D3DCache.h"
#include "Util.h"

namespace dx
{
    CommonSamplerStates::CommonSamplerStates(ID3D11Device* pDevice)
    {
        m_pLinearWrap = CreateSamplerState(pDevice, LinearWrapDesc());
        m_pLinearClamp = CreateSamplerState(pDevice, LinearClampDesc());
        m_pAnisotropicWrap = CreateSamplerState(pDevice, AnisotropicWrapDesc());
        m_pAnisotropicClamp = CreateSamplerState(pDevice, AnisotropicClampDesc());
        m_pPointWrap = CreateSamplerState(pDevice, PointWrapDesc());
        m_pPointClamp = CreateSamplerState(pDevice, PointClampDesc());
        m_pPointComp = CreateSamplerState(pDevice, PointCompDesc());
        m_pLinearComp = CreateSamplerState(pDevice, LinearCompDesc());
    }

    constexpr D3D11_SAMPLER_DESC CommonSamplerStates::LinearWrapDesc()
    {
        D3D11_SAMPLER_DESC desc{};
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.MaxLOD = D3D11_FLOAT32_MAX;
        return desc;
    }

    constexpr D3D11_SAMPLER_DESC CommonSamplerStates::LinearClampDesc()
    {
        D3D11_SAMPLER_DESC desc{};
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.MaxLOD = D3D11_FLOAT32_MAX;
        return desc;
    }

    constexpr D3D11_SAMPLER_DESC CommonSamplerStates::AnisotropicWrapDesc()
    {
        D3D11_SAMPLER_DESC desc{};
        desc.Filter = D3D11_FILTER_ANISOTROPIC;
        desc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.MaxLOD = D3D11_FLOAT32_MAX;
        return desc;
    }

    constexpr D3D11_SAMPLER_DESC CommonSamplerStates::AnisotropicClampDesc()
    {
        D3D11_SAMPLER_DESC desc{};
        desc.Filter = D3D11_FILTER_ANISOTROPIC;
        desc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.MaxLOD = D3D11_FLOAT32_MAX;
        return desc;
    }

    constexpr D3D11_SAMPLER_DESC CommonSamplerStates::PointWrapDesc()
    {
        D3D11_SAMPLER_DESC desc{};
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.MaxLOD = D3D11_FLOAT32_MAX;
        return desc;
    }

    constexpr D3D11_SAMPLER_DESC CommonSamplerStates::PointClampDesc()
    {
        D3D11_SAMPLER_DESC desc{};
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.MaxLOD = D3D11_FLOAT32_MAX;
        return desc;
    }

    constexpr D3D11_SAMPLER_DESC CommonSamplerStates::PointCompDesc()
    {
        D3D11_SAMPLER_DESC desc{};
        desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
        desc.ComparisonFunc = D3D11_COMPARISON_LESS;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.MaxLOD = D3D11_FLOAT32_MAX;
        return desc;
    }

    constexpr D3D11_SAMPLER_DESC CommonSamplerStates::LinearCompDesc()
    {
        D3D11_SAMPLER_DESC desc{};
        desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        desc.ComparisonFunc = D3D11_COMPARISON_LESS;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.MaxLOD = D3D11_FLOAT32_MAX;
        return desc;
    }

    CommonRasterizerStates::CommonRasterizerStates(ID3D11Device* pDevice)
    {
        m_pCullNone = CreateRasterizerState(pDevice, CullNoneDesc());
        m_pCullFront = CreateRasterizerState(pDevice, CullFrontDesc());
        m_pCullBack = CreateRasterizerState(pDevice, CullBackDesc());
        m_pWireframe = CreateRasterizerState(pDevice, WireframeDesc());
    }

    constexpr D3D11_RASTERIZER_DESC CommonRasterizerStates::CullNoneDesc()
    {
        D3D11_RASTERIZER_DESC desc{};
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.DepthClipEnable = true;
        return desc;
    }

    constexpr D3D11_RASTERIZER_DESC CommonRasterizerStates::CullFrontDesc()
    {
        D3D11_RASTERIZER_DESC desc{};
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_FRONT;
        desc.DepthClipEnable = true;
        return desc;
    }

    constexpr D3D11_RASTERIZER_DESC CommonRasterizerStates::CullBackDesc()
    {
        D3D11_RASTERIZER_DESC desc{};
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_BACK;
        desc.DepthClipEnable = true;
        return desc;
    }

    constexpr D3D11_RASTERIZER_DESC CommonRasterizerStates::WireframeDesc()
    {
        D3D11_RASTERIZER_DESC desc{};
        desc.FillMode = D3D11_FILL_WIREFRAME;
        desc.CullMode = D3D11_CULL_NONE;
        desc.DepthClipEnable = true;
        return desc;
    }


    CommonDepthStencilStates::CommonDepthStencilStates(ID3D11Device* pDevice)
    {
        m_pDepthDisabled = CreateDepthStencilState(pDevice, DepthDisabledDesc());
        m_pDepthEnabled = CreateDepthStencilState(pDevice, DepthEnabledDesc());
        m_pDepthEnabledWrite = CreateDepthStencilState(pDevice, DepthEnabledWriteDesc());
    }

    constexpr D3D11_DEPTH_STENCIL_DESC CommonDepthStencilStates::DepthDisabledDesc()
    {
        D3D11_DEPTH_STENCIL_DESC desc{};
        desc.DepthEnable = false;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.StencilEnable = false;
        desc.StencilReadMask = 0;
        desc.StencilWriteMask = 0;
        return desc;
    }

    constexpr D3D11_DEPTH_STENCIL_DESC CommonDepthStencilStates::DepthEnabledDesc()
    {
        D3D11_DEPTH_STENCIL_DESC desc{};
        desc.DepthEnable = true;
        desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.StencilEnable = false;
        desc.StencilReadMask = 0;
        desc.StencilWriteMask = 0;
        return desc;
    }

    constexpr D3D11_DEPTH_STENCIL_DESC CommonDepthStencilStates::DepthEnabledWriteDesc()
    {
        D3D11_DEPTH_STENCIL_DESC desc{};
        desc.DepthEnable = true;
        desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.StencilEnable = false;
        desc.StencilReadMask = 0;
        desc.StencilWriteMask = 0;
        return desc;
    }

    CommonBlendStates::CommonBlendStates(ID3D11Device* pDevice)
    {
        m_pDisabled = CreateBlendState(pDevice, DisabledDesc());
        m_pAlpha = CreateBlendState(pDevice, AlphaDesc());
        m_pPremultipliedAlpha = CreateBlendState(pDevice, PremultipliedAlphaDesc());
        m_pAdditive = CreateBlendState(pDevice, AdditiveDesc());
        m_pColorWriteDisabled = CreateBlendState(pDevice, ColorWriteDisabledDesc());
    }

    constexpr D3D11_BLEND_DESC CommonBlendStates::DisabledDesc()
    {
        D3D11_BLEND_DESC desc{};
        for (auto& rtDesc : desc.RenderTarget)
        {
            rtDesc.BlendEnable = false;
            rtDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        }
        return desc;
    }

    constexpr D3D11_BLEND_DESC CommonBlendStates::AlphaDesc()
    {
        D3D11_BLEND_DESC desc{};
        for (auto& rtDesc : desc.RenderTarget)
        {
            rtDesc.BlendEnable = true;
            rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
            rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
            rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
            rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
            rtDesc.DestBlendAlpha = D3D11_BLEND_ONE;
        }
        return desc;
    }

    constexpr D3D11_BLEND_DESC CommonBlendStates::PremultipliedAlphaDesc()
    {
        D3D11_BLEND_DESC desc{};
        for (auto& rtDesc : desc.RenderTarget)
        {
            rtDesc.BlendEnable = true;
            rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
            rtDesc.SrcBlend = D3D11_BLEND_ONE;
            rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
            rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
            rtDesc.DestBlendAlpha = D3D11_BLEND_ONE;
        }
        return desc;
    }

    constexpr D3D11_BLEND_DESC CommonBlendStates::AdditiveDesc()
    {
        D3D11_BLEND_DESC desc{};
        for (auto& rtDesc : desc.RenderTarget)
        {
            rtDesc.BlendEnable = true;
            rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
            rtDesc.SrcBlend = D3D11_BLEND_ONE;
            rtDesc.DestBlend = D3D11_BLEND_ONE;
            rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
            rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
            rtDesc.DestBlendAlpha = D3D11_BLEND_ONE;
        }
        return desc;
    }

    constexpr D3D11_BLEND_DESC CommonBlendStates::ColorWriteDisabledDesc()
    {
        D3D11_BLEND_DESC desc{};
        for (auto& rtDesc : desc.RenderTarget)
        {
            rtDesc.RenderTargetWriteMask = 0;
        }
        return desc;
    }

    D3DHelper::D3DHelper(ID3D11Device* pDevice) :
        cbPerFrame(pDevice),
        cbPerObject(pDevice),
        m_samplerStates(pDevice),
        m_rasterizerStates(pDevice),
        m_blendStates(pDevice),
        m_depthStencilStates(pDevice)
    {
    }

    void D3DHelper::BindConstantBuffers(ID3D11DeviceContext* pContext) const
    {
        cbPerObject.BindVS(pContext, 0);
        cbPerObject.BindPS(pContext, 0);
        cbPerObject.BindCS(pContext, 0);
        cbPerFrame.BindVS(pContext, 1);
        cbPerFrame.BindPS(pContext, 1);
        cbPerFrame.BindCS(pContext, 1);
    }

    void D3DHelper::BindSamplers(ID3D11DeviceContext* pContext) const
    {
        BindSamplersVS(pContext, 0,
            SamplerStates().LinearWrap(),
            SamplerStates().LinearClamp(),
            SamplerStates().AnisotropicWrap(),
            SamplerStates().AnisotropicClamp(),
            SamplerStates().PointWrap(),
            SamplerStates().PointClamp(),
            SamplerStates().PointComp(),
            SamplerStates().LinearComp());
        BindSamplersPS(pContext, 0,
            SamplerStates().LinearWrap(),
            SamplerStates().LinearClamp(),
            SamplerStates().AnisotropicWrap(),
            SamplerStates().AnisotropicClamp(),
            SamplerStates().PointWrap(),
            SamplerStates().PointClamp(),
            SamplerStates().PointComp(),
            SamplerStates().LinearComp());
        BindSamplersCS(pContext, 0,
            SamplerStates().LinearWrap(),
            SamplerStates().LinearClamp(),
            SamplerStates().AnisotropicWrap(),
            SamplerStates().AnisotropicClamp(),
            SamplerStates().PointWrap(),
            SamplerStates().PointClamp(),
            SamplerStates().PointComp(),
            SamplerStates().LinearComp());
    }
}