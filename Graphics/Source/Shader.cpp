#include "stdafx.h"

#include "Shader.h"

#include "Util.h"

using winrt::com_ptr;
using winrt::check_hresult;
using winrt::hresult;

namespace
{
	namespace fs = std::filesystem;

	const std::filesystem::path g_cachePath = "ShaderCache";

	enum class ShaderStage
	{
		eVertex,
		ePixel,
		eCompute
	};

	fs::path CreateBytecodeFilename(const fs::path& source, uint64_t optionsKey)
	{
		std::stringstream filename;
		std::string stem = source.stem().string();
		std::replace(stem.begin(), stem.end(), '.', '_');
		filename << stem << "_" << optionsKey << "_" << _DEBUG << ".blob";
		return g_cachePath / filename.str();
	}

	com_ptr<ID3DBlob> RetrieveShaderFromCache(const std::string& path, uint64_t optionsKey)
	{
		// Check if the cache is created already
		if (!fs::exists(g_cachePath))
		{
			fs::create_directory(g_cachePath);
			return nullptr;
		}

		// Create the target filename and path
		fs::path targetPath = CreateBytecodeFilename(path, optionsKey);
		if (fs::exists(targetPath))
		{
			// Check if shader recompilation is necessary
			auto sourceWriteTime = fs::directory_entry(path).last_write_time();
			auto targetWriteTime = fs::directory_entry(targetPath).last_write_time();
			if (sourceWriteTime <= targetWriteTime)
			{
				// Up-to-date shader bytecode exists
				com_ptr<ID3DBlob> bytecode;
				check_hresult(D3DReadFileToBlob(targetPath.c_str(), bytecode.put()));
				return bytecode;
			}
		}
		return nullptr;
	}

	void SaveShaderToCache(ID3DBlob* pBlob, const std::string& path, uint64_t optionsKey)
	{
		fs::path targetPath = CreateBytecodeFilename(path, optionsKey);
		check_hresult(D3DWriteBlobToFile(pBlob, targetPath.c_str(), true));
	}

	// Helper methods for shader creation
	com_ptr<ID3DBlob> CompileShader(const std::string& path, uint64_t optionsKey,
		const std::vector<std::pair<std::string, std::string>>& defines, ShaderStage type)
	{
		// Check if shader is already cached
		com_ptr<ID3DBlob> shaderBlob = RetrieveShaderFromCache(path, optionsKey);
		if (shaderBlob)
		{
			return shaderBlob;
		}

		unsigned int flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		std::vector<D3D_SHADER_MACRO> macros;
		for (const auto& d : defines)
		{
			macros.push_back(D3D_SHADER_MACRO{ d.first.c_str(), d.second.c_str() });
		}
		macros.push_back(D3D_SHADER_MACRO{ nullptr, nullptr });

		std::string target;
		switch (type)
		{
		case ShaderStage::eVertex:
			target = "vs_5_0";
			break;
		case ShaderStage::ePixel:
			target = "ps_5_0";
			break;
		case ShaderStage::eCompute:
			target = "cs_5_0";
			break;
		}

		com_ptr<ID3DBlob> errorBlob;
		std::cout << "Compiling shader " << path << std::endl;
		hresult hr = D3DCompileFromFile(dx::StringToWstring(path).c_str(), macros.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main", target.c_str(), flags, 0, shaderBlob.put(), errorBlob.put());

		if (errorBlob)
		{
			// Output specific shader compilation errors
			std::cerr << "Shader compilation warning: " + std::string(static_cast<char*>(errorBlob->GetBufferPointer()));
		}
		// Throw if shader compilation failed
		check_hresult(hr);
		SaveShaderToCache(shaderBlob.get(), path, optionsKey);
		return shaderBlob;
	}

	void CreateInputLayout(ID3D11Device* pDevice, ID3DBlob* pBytecode, ID3D11InputLayout** ppLayout)
	{
		com_ptr<ID3D11ShaderReflection> m_pReflect;
		check_hresult(D3DReflect(pBytecode->GetBufferPointer(), pBytecode->GetBufferSize(),
			IID_PPV_ARGS(m_pReflect.put())));

		D3D11_SHADER_DESC shaderDesc{};
		m_pReflect->GetDesc(&shaderDesc);

		std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDesc;
		for (unsigned int i = 0; i < shaderDesc.InputParameters; i++)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc{};
			m_pReflect->GetInputParameterDesc(i, &paramDesc);

			D3D11_INPUT_ELEMENT_DESC elementDesc{};
			elementDesc.SemanticName = paramDesc.SemanticName;
			elementDesc.SemanticIndex = paramDesc.SemanticIndex;
			elementDesc.InputSlot = 0;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;
			elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

			if (paramDesc.Mask == 1)
			{
				switch (paramDesc.ComponentType)
				{
				case D3D_REGISTER_COMPONENT_UINT32:
					elementDesc.Format = DXGI_FORMAT_R32_UINT;
					break;
				case D3D_REGISTER_COMPONENT_SINT32:
					elementDesc.Format = DXGI_FORMAT_R32_SINT;
					break;
				case D3D_REGISTER_COMPONENT_FLOAT32:
					elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
					break;
				}
			}
			else if (paramDesc.Mask < 4)
			{
				switch (paramDesc.ComponentType)
				{
				case D3D_REGISTER_COMPONENT_UINT32:
					elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
					break;
				case D3D_REGISTER_COMPONENT_SINT32:
					elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
					break;
				case D3D_REGISTER_COMPONENT_FLOAT32:
					elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
					break;
				}
			}
			else if (paramDesc.Mask < 8)
			{
				switch (paramDesc.ComponentType)
				{
				case D3D_REGISTER_COMPONENT_UINT32:
					elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
					break;
				case D3D_REGISTER_COMPONENT_SINT32:
					elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
					break;
				case D3D_REGISTER_COMPONENT_FLOAT32:
					elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
					break;
				}
			}
			else if (paramDesc.Mask < 16)
			{
				switch (paramDesc.ComponentType)
				{
				case D3D_REGISTER_COMPONENT_UINT32:
					elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
					break;
				case D3D_REGISTER_COMPONENT_SINT32:
					elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
					break;
				case D3D_REGISTER_COMPONENT_FLOAT32:
					elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					break;
				}
			}
			inputElementDesc.push_back(elementDesc);
		}
		check_hresult(pDevice->CreateInputLayout(inputElementDesc.data(),
			static_cast<unsigned int>(inputElementDesc.size()),
			pBytecode->GetBufferPointer(), pBytecode->GetBufferSize(), ppLayout));
	}
}

namespace dx
{
	VertexShader::VertexShader(ID3D11Device* pDevice, const std::string& filename,
		uint64_t optionsKey, const std::vector<std::pair<std::string, std::string>>& defines)
	{
		// Create shader
		auto blob = CompileShader(filename, optionsKey, defines, ShaderStage::eVertex);
		check_hresult(pDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(),
			nullptr, m_pShader.put()));

		CreateInputLayout(pDevice, blob.get(), m_pLayout.put());
	}

	PixelShader::PixelShader(ID3D11Device* pDevice, const std::string& filename,
		uint64_t optionsKey, const std::vector<std::pair<std::string, std::string>>& defines)
	{
		auto blob = CompileShader(filename, optionsKey, defines, ShaderStage::ePixel);
		check_hresult(pDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(),
			nullptr, m_pShader.put()));
	}

	ComputeShader::ComputeShader(ID3D11Device* pDevice, const std::string& filename,
		uint64_t optionsKey, const std::vector<std::pair<std::string, std::string>>& defines)
	{
		auto blob = CompileShader(filename, optionsKey, defines, ShaderStage::eCompute);
		check_hresult(pDevice->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(),
			nullptr, m_pShader.put()));
	}
}
