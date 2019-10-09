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

// ������������ �� ������� "���������� DirectX 12" ��. �� ������ https://go.microsoft.com/fwlink/?LinkID=613670&clcid=0x419

// ������� main ������������ ������ ��� ������������� ������ IFrameworkView.
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

// ������ �����, ��������� ��� �������� IFrameworkView.
void App::Initialize(CoreApplicationView^ applicationView)
{
	// ��������������� ����������� ������� ��� ���������� ����� ����������. ���� ������ �������� ������� Activated, �����
	// ����� ���� ������� ������ CoreWindow �������� � ��������� ��������� � ����.
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &App::OnResuming);
}

// ���������� ��� �������� (��� ��������� ��������) ������� CoreWindow.
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

// �������������� ������� ����� ��� ��������� ����� ����������� ��������� ����������.
void App::Load(Platform::String^ entryPoint)
{
	if (m_main == nullptr)
	{
		m_main = std::unique_ptr<PlanetGenMain>(new PlanetGenMain());
	}
}

// ���� ����� ���������� ����� ����, ��� ���� ���������� ��������.
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

// ��������� ��� IFrameworkView.
// ������� Terminate �� �������� � ������ ������ Uninitialize. ���� ����� ����������, ���� ����� IFrameworkView
// ������������, ���� ���������� ��������� �� �������� �����.
void App::Uninitialize()
{
}

// ����������� ������� ���������� ����� ����������.

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	// Run() �� �����������, ���� �� ����� ����������� ������ CoreWindow.
	CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// ����������� ���������� ��������� ���������� ����� ������� ��������. ��������� ��������
	// ��������, ��� ���������� ������ ����������� ������������ ��������. �������� ��������,
	// ��� �������� ���������� ���������� � ������� ������� �������. �������� ����� ����
	// ������ ���������� �������������� ����� �� ����������.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		m_main->OnSuspending();
		deferral->Complete();
	});
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// �������������� ���� ������ � ���������, ������� ���� ��������� ��� ������������ ��������. �� ��������� ������
	// � ��������� ����������� ��� ������������� ������. �������� ��������, ��� �����
	// �� ����������, ���� ������ ���������� ���� ����������� �����.

	m_main->OnResuming();
}

// ����������� ������� ����.

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

// ����������� ������� DisplayInformation

void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	// ����������. �������� LogicalDpi, ���������� �����, ����� �� ��������������� ������������ DPI ����������,
	// ���� ��� ������� ���������� ��� ��������� � ������� �������� ����������. ����� ��������� DPI � DeviceResources,
	// ������ ������� �������� ��� � ������� ������ GetDpi.
	// ��. DeviceResources.cpp ��� ��������� �������������� ��������.
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
		// ��� ������ �� ������������ ���������� D3D ������ ���� ����������� ������, ��� ���������
		// ����� ����������.

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
