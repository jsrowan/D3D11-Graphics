#pragma once

namespace dx
{
	template<typename T>
	class StructuredBuffer
	{
	public:
		template<typename Container>
		StructuredBuffer(ID3D11Device* pDevice, D3D11_USAGE usage, const Container& container) :
			m_usage(usage), m_data(std::begin(container), std::end(container))
		{
			D3D11_BUFFER_DESC desc{};
			desc.Usage = usage;
			desc.ByteWidth = static_cast<unsigned int>(std::size(container) * sizeof(T));
			desc.StructureByteStride = sizeof(T);
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			if (usage == D3D11_USAGE_DYNAMIC)
			{
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}
			D3D11_SUBRESOURCE_DATA data{};
			data.pSysMem = std::data(container);
			winrt::check_hresult(pDevice->CreateBuffer(&desc, &data, m_pBuffer.put()));
		}

		StructuredBuffer(ID3D11Device* pDevice, D3D11_USAGE usage, int size) :
			StructuredBuffer(pDevice, usage, std::vector<T>(size))
		{
		}

		void Update(ID3D11DeviceContext* pContext)
		{
			assert(m_usage != D3D11_USAGE_IMMUTABLE);

			if (m_usage == D3D11_USAGE_DYNAMIC)
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource{};
				winrt::check_hresult(pContext->Map(m_pBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
				memcpy(mappedResource.pData, m_data.data(), m_data.size() * sizeof(T));
				pContext->Unmap(m_pBuffer.get(), 0);
			}
			else
			{
				// Created with D3D11_USAGE_DEFAULT
				pContext->UpdateSubresource(m_pBuffer.get(), 0, nullptr, m_data.data(), 0, 0);
			}
		}

		const T& operator[](unsigned int index) const
		{
			return m_data[index];
		}

		T& operator[](unsigned int index) 
		{ 
			return m_data[index];
		}

		const T* Data() const
		{
			return m_data.data();
		}

		T* Data()
		{
			return m_data.data();
		}

		unsigned int Size() const
		{
			return m_data.size();
		}

		ID3D11Buffer* GetBuffer() const
		{
			return m_pBuffer.get();
		}

	private:
		D3D11_USAGE m_usage;
		std::vector<T> m_data;
		winrt::com_ptr<ID3D11Buffer> m_pBuffer;
	};

	template<typename T>
	class ConstantBuffer
	{
	public:
		static_assert(sizeof(T) % 16 == 0, "Constant Buffer data must be 16-bytes aligned");

		explicit ConstantBuffer(ID3D11Device* pDevice, const T& initData = T()) : data(initData)
		{
			D3D11_BUFFER_DESC desc{};
			desc.ByteWidth = sizeof(T);
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			winrt::check_hresult(pDevice->CreateBuffer(&desc, nullptr, m_pBuffer.put()));
		}

		T alignas(16) data;

		void Update(ID3D11DeviceContext* pContext) const
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource{};
			winrt::check_hresult(pContext->Map(m_pBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			memcpy(mappedResource.pData, &data, sizeof(data));
			pContext->Unmap(m_pBuffer.get(), 0);
		}

		void BindPS(ID3D11DeviceContext* pContext, unsigned int slot) const
		{
			auto* ptr = m_pBuffer.get();
			pContext->PSSetConstantBuffers(slot, 1, &ptr);
		}
		void BindVS(ID3D11DeviceContext* pContext, unsigned int slot) const
		{
			auto* ptr = m_pBuffer.get();
			pContext->VSSetConstantBuffers(slot, 1, &ptr);
		}
		void BindCS(ID3D11DeviceContext* pContext, unsigned int slot) const
		{
			auto* ptr = m_pBuffer.get();
			pContext->CSSetConstantBuffers(slot, 1, &ptr);
		}

		ID3D11Buffer* GetBuffer() const
		{
			return m_pBuffer.get();
		}

	private:
		winrt::com_ptr<ID3D11Buffer> m_pBuffer;
	};

	class VertexBuffer
	{
	public:
		template<typename Container>
		VertexBuffer(ID3D11Device* pDevice, const Container& container) :
			m_vertexCount(static_cast<unsigned int>(std::size(container))),
			m_stride(sizeof(*std::data(container)))
		{
			D3D11_BUFFER_DESC desc{};
			desc.ByteWidth = m_vertexCount * m_stride;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			D3D11_SUBRESOURCE_DATA data{};
			data.pSysMem = std::data(container);
			winrt::check_hresult(pDevice->CreateBuffer(&desc, &data, m_pBuffer.put()));
		}

		void Bind(ID3D11DeviceContext* pContext, unsigned int slot) const
		{
			auto* ptr = m_pBuffer.get();
			unsigned int offset = 0;
			pContext->IASetVertexBuffers(slot, 1, &ptr, &m_stride, &offset);
		}

		unsigned int GetVertexCount() const { return m_vertexCount; }

		ID3D11Buffer* GetBuffer() const
		{
			return m_pBuffer.get();
		}

	private:
		winrt::com_ptr<ID3D11Buffer> m_pBuffer;
		unsigned int m_vertexCount;
		unsigned int m_stride;
	};

	class IndexBuffer
	{
	public:
		template<typename Container>
		IndexBuffer(ID3D11Device* pDevice, const Container& container) :
			m_indexCount(static_cast<unsigned int>(std::size(container)))
		{
			using Type = std::decay_t<decltype(*std::data(container))>;
			if constexpr (std::is_same_v<Type, unsigned int>)
			{
				m_format = DXGI_FORMAT_R32_UINT;
			}
			else if constexpr (std::is_same_v<Type, unsigned short>)
			{
				m_format = DXGI_FORMAT_R16_UINT;
			}
			else
			{
				static_assert(!std::is_same_v<Type, Type>,
					"Index buffers must be filled with 16 or 32 bit unsigned integers");
			}

			D3D11_BUFFER_DESC desc{};
			desc.ByteWidth = m_indexCount * sizeof(*std::data(container));
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			D3D11_SUBRESOURCE_DATA data{};
			data.pSysMem = std::data(container);
			winrt::check_hresult(pDevice->CreateBuffer(&desc, &data, m_pBuffer.put()));
		}

		void Bind(ID3D11DeviceContext* pContext) const
		{
			pContext->IASetIndexBuffer(m_pBuffer.get(), DXGI_FORMAT_R32_UINT, 0);
		}

		unsigned int GetIndexCount() const { return m_indexCount; }

		ID3D11Buffer* GetBuffer() const
		{
			return m_pBuffer.get();
		}

	private:
		DXGI_FORMAT m_format;
		winrt::com_ptr<ID3D11Buffer> m_pBuffer;
		unsigned int m_indexCount;
	};
}