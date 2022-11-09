#include "stdafx.h"

#include "App.h"

#include "Effect.h"
#include "PBREffect.h"
#include "RenderFromTextureEffect.h"
#include "ShadowMapEffect.h"
#include "Components.h"
#include "GeometryHelper.h"

namespace dx
{
	App::App() :
		m_window(),
		m_resources(m_window.GetHWnd()), 
		m_helper(m_resources.GetDevice()),
		m_camera(m_resources.GetSize().first, m_resources.GetSize().second),
		m_pRegistry(std::make_shared<entt::registry>()),
		m_sceneGraph(m_pRegistry)
	{
		auto* pDevice = m_resources.GetDevice();
		auto* pContext = m_resources.GetContext();

		m_helper.BindSamplers(pContext);
		m_helper.BindConstantBuffers(pContext);

		// Create a directional light
		Light light{};
		light.type = Light::Type::eDirectional;
		light.data.color = { 1.0f, 1.0f, 1.0f };
		light.data.direction = { 0.0f, -6.0f, -1.0f };
		light.data.intensity = 20.0f;
		light.castsShadows = true;
		auto lightEntity = m_pRegistry->create();
		m_pRegistry->emplace<Light>(lightEntity, light);

		m_renderPasses.push_back(std::make_unique<LightsPass>(m_resources, m_cache));
		m_renderPasses.push_back(std::make_unique<ShadowPass>(m_resources, m_cache));
		m_renderPasses.push_back(std::make_unique<OpaquePass>(m_resources, m_cache));
		m_renderPasses.push_back(std::make_unique<FullscreenPass>(m_resources, m_cache));

		for (const auto& pass : m_renderPasses)
		{
			pass->ResolveResources(m_cache);
		}

		m_sceneGraph.LoadModel(pDevice, m_cache, "Assets/Sponza/Sponza.mdl");

		m_window.OnTick.Register(this, &App::Tick);
		m_window.OnResize.Register(this, &App::Resize);
	}

	int App::Run()
	{
		return m_window.MessageLoop();
	}

	void App::Update()
	{
		m_camera.Update();
	}

	void App::Tick()
	{
		Update();
		Render();
	}

	void App::Resize(int width, int height)
	{
		m_resources.Resize(width, height);
		m_camera.OnResize(width, height);
	}

	void App::Render()
	{
		auto* pContext = m_resources.GetContext();

		// Unbind shader resources before frame starts for safety
		static std::array<ID3D11ShaderResourceView*, 16> nullSRV;
		pContext->VSSetShaderResources(0, 16, nullSRV.data());
		pContext->PSSetShaderResources(0, 16, nullSRV.data());
		pContext->CSSetShaderResources(0, 16, nullSRV.data());

		for (const auto& pass : m_renderPasses)
		{
			pass->Draw(m_resources, *m_pRegistry, m_helper, m_camera);
		}
		m_resources.Present();
	}
}
