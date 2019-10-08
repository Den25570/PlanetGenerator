#include "pch.h"
#include "PlanetGenMain.h"
#include "Common\DirectXHelper.h"

using namespace PlanetGen;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Документацию по шаблону "Приложение DirectX 12" см. по адресу https://go.microsoft.com/fwlink/?LinkID=613670&clcid=0x419

// Загружает и инициализирует ресурсы приложения во время его загрузки.
PlanetGenMain::PlanetGenMain()
{
	// TODO: измените настройки таймера, если требуется режим, отличный от режима по умолчанию с переменным шагом по времени.
	// например, для логики обновления с фиксированным временным шагом 60 кадров в секунду вызовите:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

// Создает и инициализирует визуализаторов.
void PlanetGenMain::CreateRenderers(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
	// TODO: замените это инициализацией содержимого своего приложения.
	m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(deviceResources));

	OnWindowSizeChanged();
}

// Обновляет состояние приложения один раз за кадр.
void PlanetGenMain::Update()
{
	// Обновление объектов сцены.
	m_timer.Tick([&]()
	{
		// TODO: замените это функциями обновления содержимого своего приложения.
		m_sceneRenderer->Update(m_timer);
	});
}

// Отрисовывает текущий кадр в соответствии с текущим состоянием приложения.
// Возвращает значение true, если кадр отрисован и готов к отображению.
bool PlanetGenMain::Render()
{
	// Не пытаться выполнять какую-либо отрисовку до первого обновления.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	// Отрисовка объектов сцены.
	// TODO: замените это функциями отрисовки содержимого своего приложения.
	return m_sceneRenderer->Render();
}

// Обновляет состояние приложения при изменении размера окна, например при изменении ориентации устройства
void PlanetGenMain::OnWindowSizeChanged()
{
	// TODO: замените это инициализацией содержимого вашего приложения в зависимости от размера.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

// Уведомляет приложение о его приостановке.
void PlanetGenMain::OnSuspending()
{
	// TODO: замените это логикой приостановки вашего приложения.

	// Управление жизненным циклом процесса может завершить работу приостановленных приложений, поэтому
	// рекомендуется сохранять любое состояние, позволяющее приложению запуститься с места остановки.

	m_sceneRenderer->SaveState();

	// Если приложение использует выделения видеопамяти, которые просто создать повторно,
	// рекомендуется освободить эту память, чтобы она была доступна для других приложений.
}

// Уведомляет приложение о том, что оно более не приостановлено.
void PlanetGenMain::OnResuming()
{
	// TODO: замените это на логику возобновления работы вашего приложения.
}

// Уведомляет визуализаторы о том, что ресурсы устройства необходимо освободить.
void PlanetGenMain::OnDeviceRemoved()
{
	// TODO: сохраните все состояния визуализатора или необходимые приложения и освободите визуализатор
	// и его ресурсы, которые более недействительны.
	m_sceneRenderer->SaveState();
	m_sceneRenderer = nullptr;
}
