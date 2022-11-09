#pragma once

namespace dx
{
	class VertexShader
	{
	public:
		VertexShader(ID3D11Device* pDevice, const std::string& filename,
			uint64_t optionsKey, const std::vector<std::pair<std::string, std::string>>& defines);

		void Bind(ID3D11DeviceContext* pContext) const
		{
			pContext->IASetInputLayout(m_pLayout.get());
			pContext->VSSetShader(m_pShader.get(), nullptr, 0);
		}

	private:
		winrt::com_ptr<ID3D11InputLayout> m_pLayout;
		winrt::com_ptr<ID3D11VertexShader> m_pShader;
	};

	class PixelShader
	{
	public:
		PixelShader(ID3D11Device* pDevice, const std::string& filename,
			uint64_t optionsKey, const std::vector<std::pair<std::string, std::string>>& defines);

		void Bind(ID3D11DeviceContext* pContext) const
		{
			pContext->PSSetShader(m_pShader.get(), nullptr, 0);
		}

	private:
		winrt::com_ptr<ID3D11PixelShader> m_pShader;
	};

	class ComputeShader
	{
	public:
		ComputeShader(ID3D11Device* pDevice, const std::string& filename,
			uint64_t optionsKey, const std::vector<std::pair<std::string, std::string>>& defines);

		void Bind(ID3D11DeviceContext* pContext) const
		{
			pContext->CSSetShader(m_pShader.get(), nullptr, 0);
		}

	private:
		winrt::com_ptr<ID3D11ComputeShader> m_pShader;
	};
}