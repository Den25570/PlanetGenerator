#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "mesh.h"

namespace PlanetGen
{
	//2^n = SPHERE_SUBDIVISIONS_COUNT
	const int SPHERE_SUBDIVISIONS_COUNT = 8;

	// Этот пример визуализатора создает экземпляр базового конвейера отрисовки.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~Sample3DSceneRenderer();
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void Update(DX::StepTimer const& timer);
		bool Render();
		void SaveState();

		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking() { return m_tracking; }

	private:
		void LoadState();
		void Rotate(float radians);

		VertexPositionColor extractVertexes(Mesh* mesh);
		unsigned short extractIndexes(Mesh* mesh);

	private:
		// Буферы констант должны быть выравнены до 256 байт.
		static const UINT c_alignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;

		// Кэшированный указатель на ресурсы устройства.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Ресурсы Direct3D для геометрии куба.
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_commandList;
		Microsoft::WRL::ComPtr<ID3D12RootSignature>			m_rootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState>			m_pipelineState;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>		m_cbvHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource>				m_constantBuffer;
		ModelViewProjectionConstantBuffer					m_constantBufferData;
		UINT8*												m_mappedConstantBuffer;
		UINT												m_cbvDescriptorSize;
		D3D12_RECT											m_scissorRect;
		std::vector<byte>									m_vertexShader;
		std::vector<byte>									m_pixelShader;
		D3D12_VERTEX_BUFFER_VIEW							m_vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW								m_indexBufferView;

		// Переменные, используемые с циклом отрисовки.
		bool	m_loadingComplete;
		float	m_radiansPerSecond;
		float	m_angle;
		bool	m_tracking;
	};

	
}

