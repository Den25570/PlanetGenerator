#include "pch.h"
#include "PlanetGenMain.h"
#include "Common\DirectXHelper.h"

using namespace PlanetGen;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// ������������ �� ������� "���������� DirectX 12" ��. �� ������ https://go.microsoft.com/fwlink/?LinkID=613670&clcid=0x419

// ��������� � �������������� ������� ���������� �� ����� ��� ��������.
PlanetGenMain::PlanetGenMain()
{
	// TODO: �������� ��������� �������, ���� ��������� �����, �������� �� ������ �� ��������� � ���������� ����� �� �������.
	// ��������, ��� ������ ���������� � ������������� ��������� ����� 60 ������ � ������� ��������:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

// ������� � �������������� ��������������.
void PlanetGenMain::CreateRenderers(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
	// TODO: �������� ��� �������������� ����������� ������ ����������.
	m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(deviceResources));

	OnWindowSizeChanged();
}

// ��������� ��������� ���������� ���� ��� �� ����.
void PlanetGenMain::Update()
{
	// ���������� �������� �����.
	m_timer.Tick([&]()
	{
		// TODO: �������� ��� ��������� ���������� ����������� ������ ����������.
		m_sceneRenderer->Update(m_timer);
	});
}

// ������������ ������� ���� � ������������ � ������� ���������� ����������.
// ���������� �������� true, ���� ���� ��������� � ����� � �����������.
bool PlanetGenMain::Render()
{
	// �� �������� ��������� �����-���� ��������� �� ������� ����������.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	// ��������� �������� �����.
	// TODO: �������� ��� ��������� ��������� ����������� ������ ����������.
	return m_sceneRenderer->Render();
}

// ��������� ��������� ���������� ��� ��������� ������� ����, �������� ��� ��������� ���������� ����������
void PlanetGenMain::OnWindowSizeChanged()
{
	// TODO: �������� ��� �������������� ����������� ������ ���������� � ����������� �� �������.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

// ���������� ���������� � ��� ������������.
void PlanetGenMain::OnSuspending()
{
	// TODO: �������� ��� ������� ������������ ������ ����������.

	// ���������� ��������� ������ �������� ����� ��������� ������ ���������������� ����������, �������
	// ������������� ��������� ����� ���������, ����������� ���������� ����������� � ����� ���������.

	m_sceneRenderer->SaveState();

	// ���� ���������� ���������� ��������� �����������, ������� ������ ������� ��������,
	// ������������� ���������� ��� ������, ����� ��� ���� �������� ��� ������ ����������.
}

// ���������� ���������� � ���, ��� ��� ����� �� ��������������.
void PlanetGenMain::OnResuming()
{
	// TODO: �������� ��� �� ������ ������������� ������ ������ ����������.
}

// ���������� ������������� � ���, ��� ������� ���������� ���������� ����������.
void PlanetGenMain::OnDeviceRemoved()
{
	// TODO: ��������� ��� ��������� ������������� ��� ����������� ���������� � ���������� ������������
	// � ��� �������, ������� ����� ���������������.
	m_sceneRenderer->SaveState();
	m_sceneRenderer = nullptr;
}
