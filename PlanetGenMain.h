#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"

// Отрисовывает содержимое Direct3D на экране.
namespace PlanetGen
{
	class PlanetGenMain
	{
	public:
		PlanetGenMain();
		void CreateRenderers(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void Update();
		bool Render();

		void OnWindowSizeChanged();
		void OnSuspending();
		void OnResuming();
		void OnDeviceRemoved();

	private:
		// TODO: замените это собственными визуализаторами содержимого.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;

		// Таймер цикла отрисовки.
		DX::StepTimer m_timer;
	};
}