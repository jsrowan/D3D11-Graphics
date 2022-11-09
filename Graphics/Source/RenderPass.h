#pragma once

#include "D3DCache.h"
#include "DeviceResources.h"
#include "D3DHelper.h"
#include "Camera.h"
#include "Components.h"
#include "RenderFromTextureEffect.h"

namespace dx
{
	struct RenderPass
	{
		virtual ~RenderPass() = default;

		virtual void ResolveResources(D3DCache& cache) = 0;
		virtual void Draw(const DeviceResources& resources, entt::registry& registry, 
			D3DHelper& helper, const FlyCamera& camera) = 0;
	};

	class OpaquePass : public RenderPass
	{
	public:
		OpaquePass(const DeviceResources& resources, D3DCache& cache);

		void ResolveResources(D3DCache& cache) override;
		void Draw(const DeviceResources& resources, entt::registry& registry,
			D3DHelper& helper, const FlyCamera& camera) override;

	private:
		winrt::com_ptr<ID3D11RenderTargetView> m_pFrameBuffer;
		winrt::com_ptr<ID3D11DepthStencilView> m_pDepthBuffer;
		winrt::com_ptr<ID3D11ShaderResourceView> m_pCascades;
	};

	class LightsPass : public RenderPass
	{
	public:
		LightsPass(const DeviceResources& resources, D3DCache& cache);

		void ResolveResources(D3DCache& cache) override;
		void Draw(const DeviceResources& resources, entt::registry& registry,
			D3DHelper& helper, const FlyCamera& camera) override;

	private:
		StructuredBuffer<Light::Data> m_lights;
	};

	class FullscreenPass : public RenderPass
	{
	public:
		FullscreenPass(const DeviceResources& resources, D3DCache& cache);

		void ResolveResources(D3DCache& cache) override;
		void Draw(const DeviceResources& resources, entt::registry& registry,
			D3DHelper& helper, const FlyCamera& camera) override;

	private:
		RenderFromTextureEffect m_effect;
	};

	class ShadowPass : public RenderPass
	{
	public:
		ShadowPass(const DeviceResources& resources, D3DCache& factory);

		void ResolveResources(D3DCache& factory) override;
		void Draw(const DeviceResources& resources, entt::registry& registry,
			D3DHelper& helper, const FlyCamera& camera) override;

	private:
		winrt::com_ptr<ID3D11DepthStencilView> m_cascades;
		winrt::com_ptr<ID3D11RasterizerState> m_pRasterizerState;
	};
}