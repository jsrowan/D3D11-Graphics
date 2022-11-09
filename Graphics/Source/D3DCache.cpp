#include "stdafx.h"

#include "D3DCache.h"

#include "Util.h"
#include "DDSTextureLoader11.h"

using winrt::com_ptr;
using winrt::hresult;
using winrt::check_hresult;

namespace
{
    using namespace dx;

    // Pipeline state objects
    std::unordered_map<std::string, com_ptr<ID3D11SamplerState>> g_samplerStates;
    std::unordered_map<std::string, com_ptr<ID3D11BlendState>> g_blendStates;
    std::unordered_map<std::string, com_ptr<ID3D11RasterizerState>> g_rasterizerStates;
    std::unordered_map<std::string, com_ptr<ID3D11DepthStencilState>> g_depthStencilStates;

    // Shaders
    std::unordered_map<std::string, std::shared_ptr<VertexShader>> g_vertexShaders;
    std::unordered_map<std::string, std::shared_ptr<PixelShader>> g_pixelShaders;
    std::unordered_map<std::string, std::shared_ptr<ComputeShader>> g_computeShaders;

    // Textures
    std::unordered_map<std::string, com_ptr<ID3D11ShaderResourceView>> g_textures;
}

namespace dx
{
    std::shared_ptr<VertexShader> CreateVertexShader(ID3D11Device* pDevice, const std::string& filename,
        const std::vector<std::pair<std::string, std::string>>& defines, uint64_t optionsKey)
    {
        std::string key = filename + std::to_string(optionsKey);
        std::shared_ptr<VertexShader> ret = g_vertexShaders[key];
        if (!ret)
        {
            ret = std::make_shared<VertexShader>(pDevice, filename, optionsKey, defines);
        }
        return ret;
    }

    std::shared_ptr<PixelShader> CreatePixelShader(ID3D11Device* pDevice, const std::string& filename,
        const std::vector<std::pair<std::string, std::string>>& defines, uint64_t optionsKey)
    {
        std::string key = filename + std::to_string(optionsKey);
        std::shared_ptr<PixelShader> ret = g_pixelShaders[key];
        if (!ret)
        {
            ret = std::make_shared<PixelShader>(pDevice, filename, optionsKey, defines);
        }
        return ret;
    }

    std::shared_ptr<ComputeShader> CreateComputeShader(ID3D11Device* pDevice, const std::string& filename,
        const std::vector<std::pair<std::string, std::string>>& defines, uint64_t optionsKey)
    {
        std::string key = filename + std::to_string(optionsKey);
        std::shared_ptr<ComputeShader> ret = g_computeShaders[key];
        if (!ret)
        {
            ret = std::make_shared<ComputeShader>(pDevice, filename, optionsKey, defines);
        }
        return ret;
    }

    com_ptr<ID3D11SamplerState> CreateSamplerState(ID3D11Device* pDevice,
        const D3D11_SAMPLER_DESC& desc)
    {
        std::string key = CreateKey(desc);
        com_ptr<ID3D11SamplerState> ret = g_samplerStates[key];
        if (!ret)
        {
            check_hresult(pDevice->CreateSamplerState(&desc, ret.put()));
        }
        return ret;
    }

    com_ptr<ID3D11RasterizerState> CreateRasterizerState(ID3D11Device* pDevice,
        const D3D11_RASTERIZER_DESC& desc)
    {
        std::string key = CreateKey(desc);
        com_ptr<ID3D11RasterizerState> ret = g_rasterizerStates[key];
        if (!ret)
        {
            check_hresult(pDevice->CreateRasterizerState(&desc, ret.put()));
        }
        return ret;
    }

    com_ptr<ID3D11DepthStencilState> CreateDepthStencilState(ID3D11Device* pDevice,
        const D3D11_DEPTH_STENCIL_DESC& desc)
    {
        std::string key = CreateKey(desc);
        com_ptr<ID3D11DepthStencilState> ret = g_depthStencilStates[key];
        if (!ret)
        {
            check_hresult(pDevice->CreateDepthStencilState(&desc, ret.put()));
        }
        return ret;
    }

    com_ptr<ID3D11BlendState> CreateBlendState(ID3D11Device* pDevice,
        const D3D11_BLEND_DESC& desc)
    {
        std::string key = CreateKey(desc);
        com_ptr<ID3D11BlendState> ret = g_blendStates[key];
        if (!ret)
        {
            check_hresult(pDevice->CreateBlendState(&desc, ret.put()));
        }
        return ret;
    }

    com_ptr<ID3D11ShaderResourceView> CreateTexture(ID3D11Device* pDevice,
        const std::string& filename)
    {
        com_ptr<ID3D11ShaderResourceView> ret = g_textures[filename];
        if (!ret)
        {
            DirectX::CreateDDSTextureFromFile(pDevice, StringToWstring(filename).c_str(), nullptr, ret.put());
        }
        return ret;
    }

    void D3DCache::CreateTexture2D(ID3D11Device* pDevice, const std::string& name,
        const D3D11_TEXTURE2D_DESC& desc)
    {
        com_ptr<ID3D11Texture2D> pTex;
        check_hresult(pDevice->CreateTexture2D(&desc, nullptr, pTex.put()));
        m_resourceTable[name] = std::move(pTex);
    }

    void D3DCache::AddResource(const std::string& name, ID3D11Resource* pResource)
    {
        m_resourceTable[name].copy_from(pResource);
    }

    void D3DCache::AddShaderResourceView(ID3D11Device* pDevice, const std::string& resourceName,
        const std::string& viewName, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
    {
        ID3D11Resource* pResource = m_resourceTable.at(resourceName).get();
        check_hresult(pDevice->CreateShaderResourceView(pResource, &desc, m_srvTable[viewName].put()));
    }

    void D3DCache::AddShaderResourceView(ID3D11Device* pDevice, const std::string& resourceName,
        const std::string& viewName)
    {
        ID3D11Resource* pResource = m_resourceTable.at(resourceName).get();
        check_hresult(pDevice->CreateShaderResourceView(pResource, nullptr, m_srvTable[viewName].put()));
    }

    void D3DCache::AddUnorderedAccessView(ID3D11Device* pDevice, const std::string& resourceName,
        const std::string& viewName, const D3D11_UNORDERED_ACCESS_VIEW_DESC& desc)
    {
        ID3D11Resource* pResource = m_resourceTable.at(resourceName).get();
        check_hresult(pDevice->CreateUnorderedAccessView(pResource, &desc, m_uavTable[viewName].put()));
    }

    void D3DCache::AddUnorderedAccessView(ID3D11Device* pDevice, const std::string& resourceName,
        const std::string& viewName)
    {
        ID3D11Resource* pResource = m_resourceTable.at(resourceName).get();
        check_hresult(pDevice->CreateUnorderedAccessView(pResource, nullptr, m_uavTable[viewName].put()));
    }

    void D3DCache::AddDepthStencilView(ID3D11Device* pDevice, const std::string& resourceName,
        const std::string& viewName, const D3D11_DEPTH_STENCIL_VIEW_DESC& desc)
    {
        ID3D11Resource* pResource = m_resourceTable.at(resourceName).get();
        check_hresult(pDevice->CreateDepthStencilView(pResource, &desc, m_dsvTable[viewName].put()));
    }

    void D3DCache::AddDepthStencilView(ID3D11Device* pDevice, const std::string& resourceName,
        const std::string& viewName)
    {
        ID3D11Resource* pResource = m_resourceTable.at(resourceName).get();
        check_hresult(pDevice->CreateDepthStencilView(pResource, nullptr, m_dsvTable[viewName].put()));
    }

    void D3DCache::AddRenderTargetView(ID3D11Device* pDevice, const std::string& resourceName,
        const std::string& viewName, const D3D11_RENDER_TARGET_VIEW_DESC& desc)
    {
        ID3D11Resource* pResource = m_resourceTable.at(resourceName).get();
        check_hresult(pDevice->CreateRenderTargetView(pResource, &desc, m_rtvTable[viewName].put()));
    }

    void D3DCache::AddRenderTargetView(ID3D11Device* pDevice, const std::string& resourceName,
        const std::string& viewName)
    {
        ID3D11Resource* pResource = m_resourceTable.at(resourceName).get();
        check_hresult(pDevice->CreateRenderTargetView(pResource, nullptr, m_rtvTable[viewName].put()));
    }

    winrt::com_ptr<ID3D11ShaderResourceView> D3DCache::GetShaderResourceView(const std::string& name) const
    {
        return m_srvTable.at(name);
    }

    winrt::com_ptr<ID3D11UnorderedAccessView> D3DCache::GetUnorderedAccessView(const std::string& name) const
    {
        return m_uavTable.at(name);
    }

    winrt::com_ptr<ID3D11DepthStencilView> D3DCache::GetDepthStencilView(const std::string& name) const
    {
        return m_dsvTable.at(name);
    }

    winrt::com_ptr<ID3D11RenderTargetView> D3DCache::GetRenderTargetView(const std::string& name) const
    {
        return m_rtvTable.at(name);
    }
}