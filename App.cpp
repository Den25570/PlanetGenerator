#include "pch.h"
#include "App.h"

#include <ppltasks.h>

using namespace PlanetGen;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

using Microsoft::WRL::ComPtr;

// Документацию по шаблону "Приложение DirectX 12" см. по адресу https://go.microsoft.com/fwlink/?LinkID=613670&clcid=0x419

// Функция main используется только для инициализации класса IFrameworkView.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new App();
}

App::App() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

// Первый метод, вызванный при создании IFrameworkView.
void App::Initialize(CoreApplicationView^ applicationView)
{
	// Зарегистрируйте обработчики событий для жизненного цикла приложения. Этот пример включает событие Activated, чтобы
	// можно было сделать объект CoreWindow активным и запустить отрисовку в окне.
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &App::OnResuming);
}

// Вызывается при создании (или повторном создании) объекта CoreWindow.
void App::SetWindow(CoreWindow^ window)
{
	/*window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);

	PointerVisualizationSettings^ visualizationSettings = PointerVisualizationSettings::GetForCurrentView();
	visualizationSettings->IsContactFeedbackEnabled = false;
	visualizationSettings->IsBarrelButtonFeedbackEnabled = false;*/

	window->SizeChanged += 
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

	window->Closed += 
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

	window->KeyDown +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &App::OnKeyPressed);

	//window->PointerPressed +=
	//	ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerPressed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

	//currentDisplayInformation->StereoEnabledChanged +=
	//	ref new TypedEventHandler<DisplayInformation^, Platform::Object^>(this, &App::OnStereoEnabledChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDisplayContentsInvalidated);
}

// Инициализирует ресурсы сцены или загружает ранее сохраненное состояние приложения.
void App::Load(Platform::String^ entryPoint)
{
	if (m_main == nullptr)
	{
		m_main = std::unique_ptr<PlanetGenMain>(new PlanetGenMain());
	}
}

// Этот метод вызывается после того, как окно становится активным.
void App::Run()
{
	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			auto commandQueue = GetDeviceResources()->GetCommandQueue();
			PIXBeginEvent(commandQueue, 0, L"Update");
			{
				m_main->Update();
			}
			PIXEndEvent(commandQueue);

			PIXBeginEvent(commandQueue, 0, L"Render");
			{
				if (m_main->Render())
				{
					GetDeviceResources()->Present();
				}
			}
			PIXEndEvent(commandQueue);
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

// Требуется для IFrameworkView.
// События Terminate не приводят к вызову метода Uninitialize. Этот метод вызывается, если класс IFrameworkView
// уничтожается, пока приложение находится на переднем плане.
void App::Uninitialize()
{
}

// Обработчики событий жизненного цикла приложения.

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	// Run() не запускается, пока не будет активирован объект CoreWindow.
	CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Асинхронное сохранение состояния приложения после запроса задержки. Удержание задержки
	// означает, что приложение занято выполнением приостановки операций. Обратите внимание,
	// что задержку невозможно удерживать в течение долгого времени. Примерно через пять
	// секунд произойдет принудительный выход из приложения.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		m_main->OnSuspending();
		deferral->Complete();
	});
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Восстановление всех данных и состояний, которые были выгружены при приостановке действия. По умолчанию данные
	// и состояние сохраняются при возобновлении работы. Обратите внимание, что этого
	// не происходит, если работа приложения была остановлена ранее.

	m_main->OnResuming();
}

// Обработчики событий окна.

void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	GetDeviceResources()->SetLogicalSize(Size(sender->Bounds.Width, sender->Bounds.Height));
	m_main->OnWindowSizeChanged();
}

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

// Обработчики события DisplayInformation

void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	// Примечание. Значение LogicalDpi, полученное здесь, может не соответствовать фактическому DPI приложения,
	// если его масштаб изменяется для устройств с экраном высокого разрешения. После установки DPI в DeviceResources,
	// всегда следует получать его с помощью метода GetDpi.
	// См. DeviceResources.cpp для получения дополнительных сведений.
	GetDeviceResources()->SetDpi(sender->LogicalDpi);
	m_main->OnWindowSizeChanged();
}

void App::OnKeyPressed(CoreWindow^ sender, KeyEventArgs^ args)
{
	if (args->VirtualKey == Windows::System::VirtualKey::Escape)
	{
		App::Uninitialize();
	}
}

void App::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
{
	// Allow the user to interact with the holographic world using the mouse.
	if (m_main != nullptr)
	{
	/*	m_main->OnPointerPressed();*/
	}
}

void App::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	GetDeviceResources()->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->OnWindowSizeChanged();
}

void App::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	GetDeviceResources()->ValidateDevice();
}

/*void App::OnPointerPressed()
{
	m_pointerPressed = true;
}*/

std::shared_ptr<DX::DeviceResources> App::GetDeviceResources()
{
	if (m_deviceResources != nullptr && m_deviceResources->IsDeviceRemoved())
	{
		// Все ссылки на существующее устройство D3D должно быть освобождено прежде, чем создавать
		// новое устройство.

		m_deviceResources = nullptr;
		m_main->OnDeviceRemoved();

#if defined(_DEBUG)
		ComPtr<IDXGIDebug1> dxgiDebug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
		{
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}
#endif
	}

	if (m_deviceResources == nullptr)
	{
		m_deviceResources = std::make_shared<DX::DeviceResources>();
		m_deviceResources->SetWindow(CoreWindow::GetForCurrentThread());
		m_main->CreateRenderers(m_deviceResources);
	}
	return m_deviceResources;
}
