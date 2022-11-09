#include "stdafx.h"

#include "RenderPass.h"
#include "Components.h"
#include "PBREffect.h"
#include "ShadowMapEffect.h"

namespace
{
	constexpr unsigned int SHADOW_MAP_SIZE = 2048;
}

namespace dx
{
	OpaquePass::OpaquePass(const DeviceResources& resources, D3DCache& cache)
	{
		auto* pDevice = resources.GetDevice();
		auto [width, height] = resources.GetSize();

		D3D11_TEXTURE2D_DESC rtDesc{};
		rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtDesc.Width = width;
		rtDesc.Height = height;
		rtDesc.ArraySize = 1;
		rtDesc.MipLevels = 1;
		rtDesc.SampleDesc.Count = 1;
		rtDesc.SampleDesc.Quality = 0;
		rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		cache.CreateTexture2D(pDevice, "FrameBuffer", rtDesc);
		cache.AddRenderTargetView(pDevice, "FrameBuffer", "FrameBuffer");
		cache.AddShaderResourceView(pDevice, "FrameBuffer", "FrameBuffer");

		D3D11_TEXTURE2D_DESC dsDesc{};
		dsDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		dsDesc.Width = width;
		dsDesc.Height = height;
		dsDesc.ArraySize = 1;
		dsDesc.MipLevels = 1;
		dsDesc.SampleDesc.Count = 1;
		dsDesc.SampleDesc.Quality = 0;
		dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		cache.CreateTexture2D(pDevice, "DepthBuffer", dsDesc);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		cache.AddDepthStencilView(pDevice, "DepthBuffer", "DepthBuffer", dsvDesc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = UINT_MAX;
		cache.AddShaderResourceView(pDevice, "DepthBuffer", "DepthBuffer", srvDesc);
	}

	void OpaquePass::ResolveResources(D3DCache& cache)
	{
		m_pFrameBuffer = cache.GetRenderTargetView("FrameBuffer");
		assert(m_pFrameBuffer);
		m_pDepthBuffer = cache.GetDepthStencilView("DepthBuffer");
		assert(m_pDepthBuffer);
		m_pCascades = cache.GetShaderResourceView("ShadowCascades");
		assert(m_pCascades);
	}
	
	void OpaquePass::Draw(const DeviceResources& resources, entt::registry& registry,
		D3DHelper& helper, const FlyCamera& camera)
	{
		using namespace DirectX;

		auto* pContext = resources.GetContext();

		// Per frame cbuffer
		XMStoreFloat3(&helper.cbPerFrame.data.eye, camera.GetEyePosition());
		XMStoreFloat4x4(&helper.cbPerFrame.data.viewProj,
			XMMatrixTranspose(camera.GetViewProjectionMatrix()));
		XMStoreFloat4x4(&helper.cbPerFrame.data.view,
			XMMatrixTranspose(camera.GetViewMatrix()));
		helper.cbPerFrame.Update(pContext);

		// Per object cbuffer
		XMStoreFloat4x4(&helper.cbPerObject.data.model, DirectX::XMMatrixIdentity());
		helper.cbPerObject.Update(pContext);

		BindRenderTargets(pContext, m_pDepthBuffer.get(), m_pFrameBuffer.get());

		constexpr std::array<float, 4> color = { 0.5f, 0.8f, 0.95f, 1.0f };
		pContext->ClearRenderTargetView(m_pFrameBuffer.get(), color.data());
		pContext->ClearDepthStencilView(m_pDepthBuffer.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pContext->RSSetViewports(1, &resources.GetViewport());
		pContext->RSSetState(helper.RasterizerStates().CullBack());
		pContext->OMSetDepthStencilState(helper.DepthStencilStates().DepthEnabledWrite(), 0);
		
		auto view = registry.view<PBREffect, Geometry>();
		for (auto obj : view)
		{
			auto& effect = view.get<PBREffect>(obj);
			auto& geometry = view.get<Geometry>(obj);
			geometry.Bind(pContext);
			effect.resources.cascades = m_pCascades;
			effect.Bind(pContext);
			pContext->DrawIndexed(geometry.indices.GetIndexCount(), 0, 0);
		}

		// Unbind render target and depth stencil
		BindRenderTargets(pContext, nullptr);
	}

	LightsPass::LightsPass(const DeviceResources& resources, D3DCache& cache) :
		m_lights(resources.GetDevice(), D3D11_USAGE_DYNAMIC, 16)
	{
		auto* pDevice = resources.GetDevice();

		cache.AddResource("LightsBuffer", m_lights.GetBuffer());
		cache.AddShaderResourceView(pDevice, "LightsBuffer", "LightsBuffer");
	}

	void LightsPass::ResolveResources(D3DCache& cache)
	{
	}

	void LightsPass::Draw(const DeviceResources& resources, entt::registry& registry,
		D3DHelper& helper, const FlyCamera& camera)
	{
		auto* pContext = resources.GetContext();

		auto view = registry.view<Light>();
		for (auto obj : view)
		{
			const auto& l = view.get<Light>(obj);
			m_lights[0] = l.data;
		}
		m_lights.Update(pContext);
		helper.cbPerFrame.data.nlights = static_cast<int>(view.size());
		helper.cbPerFrame.Update(pContext);
	}

	FullscreenPass::FullscreenPass(const DeviceResources& resources, D3DCache& cache) : 
		m_effect(resources.GetDevice())
	{
	}

	void FullscreenPass::ResolveResources(D3DCache& cache)
	{
		m_effect.resources.inputTexture = cache.GetShaderResourceView("FrameBuffer");
	}

	void FullscreenPass::Draw(const DeviceResources& resources, entt::registry& registry, 
		D3DHelper& helper, const FlyCamera& camera)
	{
		auto* pContext = resources.GetContext();
		auto* pRenderTarget = resources.GetRenderTarget();

		pContext->RSSetViewports(1, &resources.GetViewport());
		BindRenderTargets(pContext, nullptr, pRenderTarget);

		constexpr std::array<float, 4> color = { 0.0f, 0.0f, 0.0f, 0.0f };
		pContext->ClearRenderTargetView(pRenderTarget, color.data());
		
		pContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		pContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		pContext->OMSetDepthStencilState(helper.DepthStencilStates().DepthDisabled(), 0);
		m_effect.Bind(pContext);
		pContext->Draw(3, 0);

		BindRenderTargets(pContext, nullptr);
	}

	ShadowPass::ShadowPass(const DeviceResources& resources, D3DCache& cache)
	{
		auto* pDevice = resources.GetDevice();

		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.Width = SHADOW_MAP_SIZE;
		texDesc.Height = SHADOW_MAP_SIZE;
		texDesc.ArraySize = 3;
		texDesc.MipLevels = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		cache.CreateTexture2D(pDevice, "ShadowCascades", texDesc);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.ArraySize = 3;
		dsvDesc.Texture2DArray.FirstArraySlice = 0;
		dsvDesc.Texture2DArray.MipSlice = 0;
		cache.AddDepthStencilView(pDevice, "ShadowCascades", "ShadowCascades", dsvDesc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.ArraySize = 3;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		cache.AddShaderResourceView(pDevice, "ShadowCascades", "ShadowCascades", srvDesc);

		auto rsDesc = CommonRasterizerStates::CullNoneDesc();
		rsDesc.DepthClipEnable = false;
		m_pRasterizerState = CreateRasterizerState(pDevice, rsDesc);
	}

	void ShadowPass::ResolveResources(D3DCache& cache)
	{
		m_cascades = cache.GetDepthStencilView("ShadowCascades");
		assert(m_cascades);
	}

	// Shadow map method adapted from Vulkan CSM Sample: 
	// https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmappingcascade
	void ShadowPass::Draw(const DeviceResources& resources, entt::registry& registry,
		D3DHelper& helper, const FlyCamera& camera)
	{
		using namespace DirectX;

		// Viewport for shadow mapping
		static constexpr D3D11_VIEWPORT shadowViewport{
			0.0f,				// Top Left X
			0.0f,				// Top Left Y
			SHADOW_MAP_SIZE,	// Width
			SHADOW_MAP_SIZE,	// Height
			0.0f,				// Min Depth
			1.0f				// Max Depth
		};

		// Set up pipeline state for directional shadow map rendering
		auto* pContext = resources.GetContext();
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pContext->RSSetState(m_pRasterizerState.get());
		pContext->RSSetViewports(1, &shadowViewport);
		pContext->OMSetDepthStencilState(helper.DepthStencilStates().DepthEnabledWrite(), 0);

		// Set up depth bounds
		auto [zmin, zmax] = camera.GetClipPlanes();
		float range = zmax - zmin;
		float ratio = zmax / zmin;

		std::array<float, 3> cascadeSplits{};
		for (int i = 0; i < 3; i++)
		{
			float p = (i + 1) / 3.0f;
			float log = zmin * pow(ratio, p);
			float uniform = zmin + range * p;
			float d = 0.7f * (log - uniform) + uniform;
			cascadeSplits[i] = (d - zmin) / range;
		}

		// Calculate projection matrices
		auto viewProj = camera.GetViewProjectionMatrix();
		auto viewProjInv = XMMatrixInverse(nullptr, viewProj);
		float lastSplitDist = zmin;
		for (int i = 0; i < 3; i++)
		{
			float splitDist = cascadeSplits[i];

			// Transform camera frustum into world space
			std::array<XMVECTOR, 8> frustumCorners = {
				XMVectorSet(-1.0f, 1.0f, 0.0f, 1.0f),
				XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f),
				XMVectorSet(1.0f, -1.0f, 0.0f, 1.0f),
				XMVectorSet(-1.0f, -1.0f, 0.0f, 1.0f),
				XMVectorSet(-1.0f, 1.0f, 1.0f, 1.0f),
				XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f),
				XMVectorSet(1.0f, -1.0f, 1.0f, 1.0f),
				XMVectorSet(-1.0f, -1.0f, 1.0f, 1.0f)
			};
			for (int j = 0; j < 8; j++)
			{
				frustumCorners[j] = XMVector3TransformCoord(frustumCorners[j], viewProjInv);
			}

			// Slice the frustum according to our shadow partitions
			for (int j = 0; j < 4; j++)
			{
				auto dist = XMVectorSubtract(frustumCorners[j + 4], frustumCorners[j]);
				frustumCorners[j + 4] = XMVectorAdd(frustumCorners[j], XMVectorScale(dist, splitDist));
				frustumCorners[j] = XMVectorAdd(frustumCorners[j], XMVectorScale(dist, lastSplitDist));
			}

			// Calculate frustum center
			auto frustumCenter = XMVectorZero();
			for (int j = 0; j < 8; j++)
			{
				frustumCenter = XMVectorAdd(frustumCenter, frustumCorners[j]);
			}
			frustumCenter = XMVectorScale(frustumCenter, 1.0f / 8.0f);

			// Calculate bounding sphere
			float radius = 0.0f;
			for (int j = 0; j < 8; j++)
			{
				float dist = XMVectorGetX(XMVector3Length(XMVectorSubtract(frustumCorners[j], frustumCenter)));
				radius = std::max(radius, dist);
			}

			// Store split distance for this cascade in the constant buffer
			helper.cbPerFrame.data.cascadeSplits[i] = zmin + splitDist * range;

			// The calculations above can be shared for all lights, but right now we're just doing one.
			auto lightsView = registry.view<Light>();
			for (auto light : lightsView)
			{
				const auto& l = lightsView.get<Light>(light);
				if (l.castsShadows && l.type == Light::Type::eDirectional)
				{
					auto lightDir = XMVector3Normalize(XMLoadFloat3(&l.data.direction));
					auto eye = XMVectorSubtract(frustumCenter, XMVectorScale(lightDir, radius));
					auto up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
					auto lightView = XMMatrixLookAtRH(eye, frustumCenter, up);
					auto lightProj = XMMatrixOrthographicOffCenterRH(-radius, radius,
						-radius, radius, 0.0f, 2.0f * radius);
					auto lightViewProj = XMMatrixMultiplyTranspose(lightView, lightProj);

					XMStoreFloat4x4(&helper.cbPerFrame.data.lightViewProj[i], lightViewProj);
				}
			}
			lastSplitDist = splitDist;
		}
		helper.cbPerFrame.Update(pContext);

		// Do a depth-only pass
		ID3D11DepthStencilView* cascades = m_cascades.get();
		BindRenderTargets(pContext, cascades);
		pContext->ClearDepthStencilView(cascades, D3D11_CLEAR_DEPTH, 1.0f, 0);

		auto objView = registry.view<ShadowMapEffect, Geometry>();
		for (auto obj : objView)
		{
			const auto& effect = objView.get<ShadowMapEffect>(obj);
			const auto& geometry = objView.get<Geometry>(obj);
			geometry.Bind(pContext);
			effect.Bind(pContext);
			pContext->DrawIndexedInstanced(geometry.indices.GetIndexCount(), 3, 0, 0, 0);
		}

		// Unbind render target and depth stencil so we don't run into invalid state later
		BindRenderTargets(pContext, nullptr);
	}
}
