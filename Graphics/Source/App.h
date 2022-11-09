#pragma once

#include "DeviceResources.h"
#include "Window.h"
#include "D3DCache.h"
#include "Camera.h"
#include "RenderPass.h"
#include "SceneGraph.h"
#include "D3DHelper.h"

namespace dx
{
	class App
	{
	public:
		App();

		int Run();

		static constexpr int WIDTH = 1280;
		static constexpr int HEIGHT = 720;

	private:
		Win32Window m_window;
		DeviceResources m_resources;
		D3DCache m_cache;
		D3DHelper m_helper;
		FlyCamera m_camera;
		std::vector<std::unique_ptr<RenderPass>> m_renderPasses;
		std::shared_ptr<entt::registry> m_pRegistry;
		SceneGraph m_sceneGraph;

		void Render();
		void Update();
		void Tick();
		void Resize(int width, int height);
	};
}