#include "Converter.h"

#include <winrt/base.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>
#include <assimp/material.h>

#include <DirectXTex.h>

#include <iostream>
#include <fstream>

using winrt::check_hresult;

using namespace DirectX;
using namespace dx::importer;
using namespace dx;
using namespace std::string_literals;

namespace
{
	struct AssimpTextureInfo
	{
		std::filesystem::path baseColor;
		std::filesystem::path metalness;
		std::filesystem::path roughness;
		std::filesystem::path occlusion;
		std::filesystem::path normal;
		std::filesystem::path emissive;
	};

	std::string FormatToString(DXGI_FORMAT fmt)
	{
		std::string fmtString;
		switch (fmt)
		{
		case DXGI_FORMAT_BC1_UNORM:
			fmtString = "BC1_UNORM";
			break;
		case DXGI_FORMAT_BC1_UNORM_SRGB:
			fmtString = "BC1_UNORM_SRGB";
			break;
		case DXGI_FORMAT_BC3_UNORM:
			fmtString = "BC3_UNORM";
			break;
		case DXGI_FORMAT_BC3_UNORM_SRGB:
			fmtString = "BC3_UNORM_SRGB";
			break;
		case DXGI_FORMAT_BC5_UNORM:
			fmtString = "BC5_UNORM";
			break;
		case DXGI_FORMAT_BC7_UNORM:
			fmtString = "BC7_UNORM";
			break;
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			fmtString = "BC7_UNORM_SRGB";
			break;
		default:
			fmtString = "UNKNOWN";
		}
		return fmtString;
	}

	AssimpTextureInfo GetTextures(const aiMaterial* aiMat, Material& mat, 
		const std::filesystem::path& srcDir)
	{
		AssimpTextureInfo aiInfo{};
		aiString str;

		if (aiMat->GetTexture(aiTextureType_BASE_COLOR, 0, &str) == AI_SUCCESS)
		{
			std::cout << "Base color texture: " << str.C_Str() << "\n";
			aiInfo.baseColor = srcDir / str.C_Str();
			mat.textures |= TextureType::eBaseColor;
		}
		if (aiMat->GetTexture(aiTextureType_METALNESS, 0, &str) == AI_SUCCESS)
		{
			std::cout << "Metalness texture: " << str.C_Str() << "\n";
			aiInfo.metalness = srcDir / str.C_Str();
			mat.textures |= TextureType::eMetalness;
		}
		if (aiMat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &str) == AI_SUCCESS)
		{
			std::cout << "Roughness texture: " << str.C_Str() << "\n";
			aiInfo.roughness = srcDir / str.C_Str();
			mat.textures |= TextureType::eRoughness;
		}
		if (aiMat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &str) == AI_SUCCESS)
		{
			std::cout << "Occlusion texture: " << str.C_Str() << "\n";
			aiInfo.occlusion = srcDir / str.C_Str();
			mat.textures |= TextureType::eOcclusion;
		}
		if (aiMat->GetTexture(aiTextureType_NORMALS, 0, &str) == AI_SUCCESS)
		{
			std::cout << "Normal texture: " << str.C_Str() << "\n";
			aiInfo.normal = srcDir / str.C_Str();
			mat.textures |= TextureType::eNormal;
		}
		if (aiMat->GetTexture(aiTextureType_EMISSIVE, 0, &str) == AI_SUCCESS)
		{
			std::cout << "Emissive texture: " << str.C_Str() << "\n";
			aiInfo.emissive = srcDir / str.C_Str();
			mat.textures |= TextureType::eEmissive;
		}
		std::cout << "\n";
		return aiInfo;
	}

	void ProcessAndSaveTexture(const DirectX::ScratchImage& image, DXGI_FORMAT fmt,
		const std::filesystem::path& dst, 
		AlphaMode alpha = AlphaMode::eOpaque, float alphaRef = 0.5f)
	{
		ScratchImage premultiplied;
		premultiplied.InitializeFromImage(*image.GetImage(0, 0, 0));
		if (alpha != AlphaMode::eOpaque)
		{
			std::cout << "\tPremultiplying alpha\n";
			check_hresult(PremultiplyAlpha(*image.GetImage(0, 0, 0), TEX_PMALPHA_DEFAULT, premultiplied));
		}

		ScratchImage mipchain;
		std::cout << "\tGenerating mipmaps\n";
		check_hresult(GenerateMipMaps(*premultiplied.GetImage(0, 0, 0), TEX_FILTER_DEFAULT, 0, mipchain));

		if (alpha == AlphaMode::eMask)
		{
			ScratchImage scaled;
			auto& info = mipchain.GetMetadata();
			check_hresult(scaled.Initialize(info));
			std::cout << "\tScaling mipmaps for coverage\n";
			check_hresult(ScaleMipMapsAlphaForCoverage(mipchain.GetImages(), info.mipLevels, info,
				0, alphaRef, scaled));
			mipchain = std::move(scaled);
		}

		ScratchImage compressed;
		std::string fmtString = FormatToString(fmt);
		std::cout << "\tCompressing to format " << fmtString << "\n";
		check_hresult(Compress(mipchain.GetImages(), mipchain.GetImageCount(), mipchain.GetMetadata(), fmt,
			TEX_COMPRESS_DEFAULT, TEX_THRESHOLD_DEFAULT, compressed));
		
		std::wcout << L"\tSaving to " << dst << "\n";
		check_hresult(SaveToDDSFile(compressed.GetImages(), compressed.GetImageCount(), compressed.GetMetadata(),
			DDS_FLAGS_NONE, dst.c_str()));
	}

	void ConvertTexture(const std::filesystem::path& src, WIC_FLAGS flags, 
		DXGI_FORMAT fmt, const std::filesystem::path& dst, AlphaMode alpha = AlphaMode::eOpaque,
		float alphaRef = 0.5f)
	{
		ScratchImage out;
		check_hresult(LoadFromWICFile(src.c_str(), flags, nullptr, out));
		ProcessAndSaveTexture(out, fmt, dst, alpha, alphaRef);
	}

	void PackTextures(Material& material, const AssimpTextureInfo& aiInfo, const std::filesystem::path& targetDir)
	{
		static int textureID = 0;

		// Pack Occlusion-Roughness-Metalness map if required
		if ((!aiInfo.occlusion.empty()) || (!aiInfo.roughness.empty()) || (!aiInfo.metalness.empty()))
		{
			TexMetadata md{};
			ScratchImage out;
			ScratchImage occlusion;
			ScratchImage roughness;
			ScratchImage metalness;
			if (!aiInfo.occlusion.empty())
			{
				check_hresult(LoadFromWICFile(aiInfo.occlusion.c_str(), WIC_FLAGS_FORCE_LINEAR, &md, occlusion));
				if (out.GetImageCount() == 0)
				{
					check_hresult(out.Initialize(md));
				}
			}
			if (!aiInfo.roughness.empty())
			{
				check_hresult(LoadFromWICFile(aiInfo.roughness.c_str(), WIC_FLAGS_FORCE_LINEAR, &md, roughness));
				if (out.GetImageCount() == 0)
				{
					check_hresult(out.Initialize(md));
				}
			}
			if (!aiInfo.metalness.empty())
			{
				check_hresult(LoadFromWICFile(aiInfo.metalness.c_str(), WIC_FLAGS_FORCE_LINEAR, &md, metalness));
				if (out.GetImageCount() == 0)
				{
					check_hresult(out.Initialize(md));
				}
			}

			if (occlusion.GetImageCount() != 0)
			{
				check_hresult(TransformImage(*occlusion.GetImage(0, 0, 0), [](XMVECTOR* outPixels,
					const XMVECTOR* inPixels,
					size_t width, [[maybe_unused]] size_t y)
					{
						for (size_t i = 0; i < width; i++)
						{
							assert(i < width);
							float r = XMVectorGetX(inPixels[i]);
							outPixels[i] = XMVectorSetX(inPixels[i], r);
						}
					}, out));
			}
			if (roughness.GetImageCount() != 0)
			{
				check_hresult(TransformImage(*roughness.GetImage(0, 0, 0), [](XMVECTOR* outPixels,
					const XMVECTOR* inPixels,
					size_t width, [[maybe_unused]] size_t y)
					{
						for (size_t i = 0; i < width; i++)
						{
							assert(i < width);
							float g = XMVectorGetY(inPixels[i]);
							outPixels[i] = XMVectorSetY(inPixels[i], g);
						}
					}, out));
			}
			if (metalness.GetImageCount() != 0)
			{
				check_hresult(TransformImage(*metalness.GetImage(0, 0, 0), [](XMVECTOR* outPixels,
					const XMVECTOR* inPixels,
					size_t width, [[maybe_unused]] size_t y)
					{
						for (size_t i = 0; i < width; i++)
						{
							assert(i < width);
							float b = XMVectorGetZ(inPixels[i]);
							outPixels[i] = XMVectorSetZ(inPixels[i], b);
						}
					}, out));
			}
			std::cout << "Processing occlusion-roughness-metalness map\n";
			std::string filename = "occlusionRoughnessMetalness" + std::to_string(textureID) + ".dds";
			ProcessAndSaveTexture(out, DXGI_FORMAT_BC1_UNORM, targetDir / filename);
			material.occlusionRoughnessMetalness = filename;
		}
		if (!aiInfo.baseColor.empty())
		{
			std::cout << "Processing base color map\n";
			std::string filename = "baseColor" + std::to_string(textureID) + ".dds";
			DXGI_FORMAT fmt = (material.alphaMode == AlphaMode::eOpaque)
				? DXGI_FORMAT_BC1_UNORM_SRGB : DXGI_FORMAT_BC3_UNORM_SRGB;
			ConvertTexture(aiInfo.baseColor, WIC_FLAGS_FORCE_SRGB, fmt, targetDir / filename, 
				material.alphaMode, material.alphaCutoff);
			material.baseColor = filename;
		}
		if (!aiInfo.emissive.empty())
		{
			std::cout << "Processing emissive map\n";
			std::string filename = "emissive" + std::to_string(textureID) + ".dds";
			ConvertTexture(aiInfo.emissive, WIC_FLAGS_FORCE_SRGB,
				DXGI_FORMAT_BC1_UNORM_SRGB, targetDir / filename);
			material.emissive = filename;
		}
		if (!aiInfo.normal.empty())
		{
			std::cout << "Processing normal map\n";
			std::string filename = "normal" + std::to_string(textureID) + ".dds";
			ConvertTexture(aiInfo.normal, WIC_FLAGS_FORCE_LINEAR,
				DXGI_FORMAT_BC5_UNORM, targetDir / filename);
			material.normal = filename;
		}
		textureID++;
	}

	Material GetMaterial(const aiMaterial* aiMat, const std::filesystem::path& targetDir,
		const std::filesystem::path& srcDir)
	{
		Material mat{};

		// Texture mapping mode, assuming all textures have the same mapping mode
		aiTextureMapMode aiMapMode;
		if (aiMat->Get(AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0), aiMapMode) == AI_SUCCESS)
		{
			switch (aiMapMode)
			{
			case aiTextureMapMode_Clamp:
				mat.addressU = D3D11_TEXTURE_ADDRESS_CLAMP;
				break;
			case aiTextureMapMode_Wrap:
				mat.addressU = D3D11_TEXTURE_ADDRESS_WRAP;
				break;
			}
		}
		if (aiMat->Get(AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0), aiMapMode) == AI_SUCCESS)
		{
			switch (aiMapMode)
			{
			case aiTextureMapMode_Clamp:
				mat.addressV = D3D11_TEXTURE_ADDRESS_CLAMP;
				break;
			case aiTextureMapMode_Wrap:
				mat.addressV = D3D11_TEXTURE_ADDRESS_WRAP;
				break;
			}
		}

		// Determine alpha mode and cutoff
		aiString aiAlphaMode;
		if (aiMat->Get(AI_MATKEY_GLTF_ALPHAMODE, aiAlphaMode) == AI_SUCCESS)
		{
			std::string alphaMode(aiAlphaMode.C_Str());
			if (alphaMode == "OPAQUE")
			{
				mat.alphaMode = AlphaMode::eOpaque;
			}
			else if (alphaMode == "MASK")
			{
				mat.alphaMode = AlphaMode::eMask;
			}
			else if (alphaMode == "BLEND")
			{
				mat.alphaMode = AlphaMode::eBlend;
			}
			std::cout << "Alpha mode: " << alphaMode << "\n";
		}
		float aiAlphaCutoff;
		if (aiMat->Get(AI_MATKEY_GLTF_ALPHACUTOFF, aiAlphaCutoff) == AI_SUCCESS)
		{
			mat.alphaCutoff = aiAlphaCutoff;
			std::cout << "Alpha cutoff: " << aiAlphaCutoff << "\n";
		}

		// Get constants if present
		aiColor4D aiBaseColor;
		if (aiMat->Get(AI_MATKEY_BASE_COLOR, aiBaseColor) == AI_SUCCESS)
		{
			mat.baseColorFactor = DirectX::XMFLOAT4(aiBaseColor.r, aiBaseColor.g, aiBaseColor.b, aiBaseColor.a);
			std::cout << "Base color factor: " << "[" << aiBaseColor.r << ", " << aiBaseColor.g
				<< ", " << aiBaseColor.b << ", " << aiBaseColor.a << "]\n";
		}
		aiColor3D aiEmissiveColor;
		if (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, aiEmissiveColor) == AI_SUCCESS)
		{
			mat.emissiveFactor = DirectX::XMFLOAT3(aiEmissiveColor.r, aiEmissiveColor.g, aiEmissiveColor.b);
			std::cout << "Emissive factor: " << "[" << aiEmissiveColor.r << ", " << aiEmissiveColor.g
				<< ", " << aiEmissiveColor.b << "]\n";
		}
		float aiRoughness;
		if (aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, aiRoughness) == AI_SUCCESS)
		{
			mat.roughnessFactor = aiRoughness;
			std::cout << "Roughness factor: " << aiRoughness << "\n";
		}
		float aiMetalness;
		if (aiMat->Get(AI_MATKEY_METALLIC_FACTOR, aiMetalness) == AI_SUCCESS)
		{
			mat.metallicFactor = aiMetalness;
			std::cout << "Metallic factor: " << aiMetalness << "\n";
		}

		const AssimpTextureInfo info = GetTextures(aiMat, mat, srcDir);
		PackTextures(mat, info, targetDir);
		return mat;
	}

	Mesh ProcessNode(const aiNode* node, const aiScene* scene, const std::filesystem::path& targetDir,
		const std::filesystem::path& srcDir)
	{
		Mesh ret;

		const unsigned int meshIdx = node->mMeshes[0];
		const aiMesh* mesh = scene->mMeshes[meshIdx];
		const unsigned int materialIdx = mesh->mMaterialIndex;
		const aiMaterial* mat = scene->mMaterials[materialIdx];

		// Textures and other material info
		ret.material = GetMaterial(mat, targetDir, srcDir);

		// Index buffer
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
			{
				ret.indices.push_back(mesh->mFaces[i].mIndices[j]);
			}
		}

		// Vertex buffer
		if (ret.material.normal)
		{
			// Normal mapping requires position, normals, tangents and uvs
			std::vector<VertexP3N3U2T3> vertices;
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				VertexP3N3U2T3 v = {
					XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
					XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z),
					XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y),
					XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z)
				};
				vertices.push_back(v);
			}
			ret.vertices = vertices;
			ret.vertexType = VertexType::eP3N3U2T3;
		}
		else if (mesh->HasTextureCoords(0))
		{
			// No tangents needed
			std::vector<VertexP3N3U2> vertices;
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				VertexP3N3U2 v = {
					XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
					XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z),
					XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
				};
				vertices.push_back(v);
			}
			ret.vertices = vertices;
			ret.vertexType = VertexType::eP3N3U2;
		}
		else
		{
			// Just positions and normals
			std::vector<VertexP3N3> vertices;
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				VertexP3N3 v = {
					XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
					XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
				};
				vertices.push_back(v);
			}
			ret.vertices = vertices;
			ret.vertexType = VertexType::eP3N3;
		}
		return ret;
	}

	StaticModel LoadModel(const std::filesystem::path& src, const std::filesystem::path& targetDir)
	{
		std::cout << "Loading model: " << src << "\n";

		Assimp::Importer importer;
		StaticModel ret;

		constexpr auto flags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_TransformUVCoords
			| aiProcess_FixInfacingNormals | aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_PreTransformVertices;
		const aiScene* scene = importer.ReadFile(src.string(), flags);
		if (!scene)
		{
			throw std::runtime_error("Import failed with error "s + importer.GetErrorString());
		}

		const auto srcDir = src.parent_path();
		int nodeIdx = 0;
		const aiNode* root = scene->mRootNode;
		if (root->mNumMeshes > 0)
		{
			std::cout << "\nProcessing node " << nodeIdx++ << "\n";
			ret.meshes.push_back(ProcessNode(root, scene, targetDir, srcDir));
		}
		for (unsigned int i = 0; i < root->mNumChildren; i++)
		{
			aiNode* node = root->mChildren[i];
			if (node->mNumMeshes > 0)
			{
				std::cout << "\nProcessing node " << nodeIdx++ << "\n";
				ret.meshes.push_back(ProcessNode(node, scene, targetDir, srcDir));
			}
		}
		std::cout << "Finished processing model\n";
		return ret;
	}

	void PrintUsage()
	{
		std::cerr << "Usage: convert [filename] [output folder]\n";
	}
}

int wmain(int argc, wchar_t** argv)
{
	check_hresult(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

	if (argc != 3)
	{
		PrintUsage();
		return 0;
	}

	std::filesystem::path target(argv[2]);
	if ((!std::filesystem::exists(target)) && (!std::filesystem::create_directory(target)))
	{
		std::cerr << "Failed to create target directory " << target << "\n";
		return 1;
	}
	const auto model = LoadModel(argv[1], target);

	std::filesystem::path filename = std::filesystem::path(argv[2]).stem();
	filename += ".mdl";
	std::filesystem::path dst = target / filename;
	std::ofstream ofs(dst, std::ios::binary);
	if (!ofs.good())
	{
		std::cerr << "Failed to open file " << dst << " for output\n";
		return 1;
	}
	{
		cereal::BinaryOutputArchive oarchive(ofs);
		oarchive(model);
	}
	std::cout << "Saved to " << dst << "\n";
	return 0;
}
