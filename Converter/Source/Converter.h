#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include <vector>
#include <optional>
#include <variant>
#include <filesystem>
#include <functional>

#include <cereal/archives/binary.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/string.hpp>

#include "VertexTypes.h"
#include "Util.h"

namespace cereal
{
	template<typename Archive>
	void serialize(Archive& ar, DirectX::XMFLOAT2& v)
	{
		ar(v.x, v.y);
	}

	template<typename Archive>
	void serialize(Archive& ar, DirectX::XMFLOAT3& v)
	{
		ar(v.x, v.y, v.z);
	}

	template<typename Archive>
	void serialize(Archive& ar, DirectX::XMFLOAT4& v)
	{
		ar(v.x, v.y, v.z, v.w);
	}
}

namespace dx::importer
{
	enum class AlphaMode
	{
		eBlend,
		eMask,
		eOpaque
	};

	enum class TextureType
	{
		eBaseColor = 1 << 0,
		eMetalness = 1 << 1,
		eRoughness = 1 << 2,
		eOcclusion = 1 << 3,
		eNormal = 1 << 4,
		eEmissive = 1 << 5
	};

	struct Material
	{
		// States which textures are present
		TextureType textures;

		// Constants
		DirectX::XMFLOAT4 baseColorFactor;	// Default = [1, 1, 1, 1]
		float metallicFactor;				// Default = 1
		float roughnessFactor;				// Default = 1
		DirectX::XMFLOAT3 emissiveFactor;	// Default = [0, 0, 0]
		float alphaCutoff;

		// Flags
		AlphaMode alphaMode;
		D3D11_TEXTURE_ADDRESS_MODE addressU;
		D3D11_TEXTURE_ADDRESS_MODE addressV;

		// Textures
		std::optional<std::string> baseColor;
		std::optional<std::string> occlusionRoughnessMetalness;
		std::optional<std::string> normal;
		std::optional<std::string> emissive;

		// Constructor using default values from the GLTF spec
		Material() : textures(), baseColorFactor(1.0f, 1.0f, 1.0f, 1.0f), metallicFactor(1.0f),
			roughnessFactor(1.0f), emissiveFactor(0.0f, 0.0f, 0.0f), alphaCutoff(0.5f),
			alphaMode(AlphaMode::eOpaque), 
			addressU(D3D11_TEXTURE_ADDRESS_CLAMP), addressV(D3D11_TEXTURE_ADDRESS_CLAMP) { }

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(textures, baseColorFactor, metallicFactor, roughnessFactor, emissiveFactor,
				alphaCutoff, alphaMode, addressU, addressV,
				baseColor, occlusionRoughnessMetalness, normal, emissive);
		}
	};

	using VertexArray = std::variant<std::vector<VertexP3N3>, std::vector<VertexP3N3U2>,
		std::vector<VertexP3N3U2T3>>;
	using Index = uint32_t;

	struct Mesh
	{
		// Geometry
		VertexType vertexType;
		VertexArray vertices;
		std::vector<Index> indices;

		// Contains paths to textures
		Material material;

		Mesh() : vertexType() { }

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(vertexType, vertices, indices, material);
		}
	};

	// Static model that cannot be transfomed further, simply consisting of
	// an array of meshes. Each mesh uses a separate material.
	// No transform calculations are required before rendering.
	struct StaticModel
	{
		std::vector<Mesh> meshes;

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(meshes);
		}
	};
}

namespace dx
{
	template<>
	struct BitmaskEnable<importer::TextureType>
	{
		static constexpr bool enabled = true;
	};
}