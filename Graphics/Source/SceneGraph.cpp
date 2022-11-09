#include "stdafx.h"

#include "SceneGraph.h"

#include "Converter.h"
#include "Components.h"
#include "RenderPass.h"
#include "PBREffect.h"
#include "ShadowMapEffect.h"

namespace dx
{
	SceneGraph::SceneGraph(std::shared_ptr<entt::registry> pRegistry)
	{
		m_pRegistry = std::move(pRegistry);

		// Create the root node and get a pointer to its transform
		m_root = m_pRegistry->create();
		m_pRootTransform = &m_pRegistry->emplace<Transform>(m_root, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
			DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), m_root, nullptr);
	}

	void SceneGraph::AddNode(entt::entity node, const DirectX::XMFLOAT3& localTranslation, const DirectX::XMFLOAT4& localRotation, 
		const DirectX::XMFLOAT3& localScale, entt::entity parent)
	{
		Transform* pParentTransform = nullptr;
		if (parent != entt::null)
		{
			pParentTransform = &m_pRegistry->get<Transform>(parent);
		}

		AddNode(node, localTranslation, localRotation, localScale, pParentTransform);
	}

	void SceneGraph::AddNode(entt::entity node, const DirectX::XMFLOAT3& localTranslation, const DirectX::XMFLOAT4& localRotation,
		const DirectX::XMFLOAT3& localScale, Transform* parent)
	{
		if (!parent)
		{
			parent = m_pRootTransform;
		}

		auto& childTransform = m_pRegistry->emplace<Transform>(node, localTranslation, localRotation, localScale, node, parent);
		parent->pChildren.push_back(&childTransform);

	}

	void SceneGraph::DestroyNode(entt::entity node)
	{
		auto& transform = m_pRegistry->get<Transform>(node);
		DestroyNode(&transform);
	}

	void SceneGraph::DestroyNode(Transform* t)
	{
		for (const auto* child : t->pChildren)
		{
			m_pRegistry->destroy(child->owner);
		}
		m_pRegistry->destroy(t->owner);
	}

	const Transform::Data& SceneGraph::GetGlobalTransform(entt::entity node)
	{
		auto& transform = m_pRegistry->get<Transform>(node);
		return GetGlobalTransform(&transform);
	}

	const Transform::Data& SceneGraph::GetGlobalTransform(Transform* t) const
	{
		using namespace DirectX;

		if (t->dirty)
		{
			// Recursively calculate transforms of the parent
			const auto& parentTransform = GetGlobalTransform(t->pParent);

			// Combine transforms with parent
			auto global = XMLoadFloat3(&parentTransform.translation);
			auto local = XMLoadFloat3(&t->local.translation);
			XMStoreFloat3(&t->global.translation, XMVectorAdd(local, global));
			
			global = XMLoadFloat4(&parentTransform.rotation);
			local = XMLoadFloat4(&t->local.rotation);
			XMStoreFloat4(&t->global.rotation, XMQuaternionMultiply(local, global));

			global = XMLoadFloat3(&parentTransform.scale);
			local = XMLoadFloat3(&t->local.scale);
			XMStoreFloat3(&t->global.scale, XMVectorMultiply(local, global));
		}
		return t->global;
	}

	void SceneGraph::SetTransform(entt::entity node, const DirectX::XMFLOAT3& localTranslation, 
		const DirectX::XMFLOAT4& localRotation, const DirectX::XMFLOAT3& localScale)
	{
		auto& transform = m_pRegistry->get<Transform>(node);
		SetTransform(&transform, localTranslation, localRotation, localScale);
	}

	void SceneGraph::SetTransform(Transform* t, const DirectX::XMFLOAT3& localTranslation,
		const DirectX::XMFLOAT4& localRotation, const DirectX::XMFLOAT3& localScale)
	{
		t->local.translation = localTranslation;
		t->local.rotation = localRotation;
		t->local.scale = localScale;
		t->dirty = true;

		for (auto* child : t->pChildren)
		{
			child->dirty = true;
		}
	}

	// Convenience function to load a PBR model
	void SceneGraph::LoadModel(ID3D11Device* pDevice, D3DCache& cache, const std::string& path, 
		const DirectX::XMFLOAT3& localTranslation, const DirectX::XMFLOAT4& localRotation, 
		const DirectX::XMFLOAT3& localScale, entt::entity parent)
	{
		using namespace importer;
		using namespace entt::literals;

		std::ifstream ifs(path, std::ios::binary);
		if (!ifs.good())
		{
			throw std::runtime_error("Failed to open model " + path);
		}

		StaticModel model;
		{
			cereal::BinaryInputArchive ar(ifs);
			ar(model);
		}

		for (const auto& mesh : model.meshes)
		{
			// Chose correct shader options based on material info
			PBREffect::Options pbrOptions{};
			ShadowMapEffect::Options shadowOptions{};
			if ((mesh.vertexType == VertexType::eP3N3U2) || (mesh.vertexType == VertexType::eP3N3U2T3))
			{
				pbrOptions.bits.hasTexcoords = true;
				shadowOptions.bits.hasTangents = true;
			}
			if (mesh.vertexType == VertexType::eP3N3U2T3)
			{
				pbrOptions.bits.hasTangents = true;
				shadowOptions.bits.hasTangents = true;
			}
			if (Any(mesh.material.textures & TextureType::eBaseColor))
			{
				pbrOptions.bits.useColorMap = true;
			}
			if (Any(mesh.material.textures & TextureType::eEmissive))
			{
				//options.bits.useEmissiveMap = true;
			}
			if (Any(mesh.material.textures & TextureType::eMetalness))
			{
				pbrOptions.bits.useMetalnessMap = true;
			}
			if (Any(mesh.material.textures & TextureType::eRoughness))
			{
				pbrOptions.bits.useRoughnessMap = true;
			}
			if (Any(mesh.material.textures & TextureType::eOcclusion))
			{
				pbrOptions.bits.useOcclusionMap = true;
			}
			if (Any(mesh.material.textures & TextureType::eNormal))
			{
				pbrOptions.bits.useNormalMap = true;
			}
			PBREffect pbrEffect(pDevice, pbrOptions);
			ShadowMapEffect shadowEffect(pDevice, shadowOptions);

			// Create resources for the effect
			pbrEffect.resources.lights = cache.GetShaderResourceView("LightsBuffer");
			// Load textures from disk
			std::filesystem::path dir = std::filesystem::path(path).parent_path();
			if (pbrOptions.bits.useColorMap)
			{
				auto texturePath = dir / *mesh.material.baseColor;
				pbrEffect.resources.color = CreateTexture(pDevice, texturePath.string());
			}
			if (pbrOptions.bits.useOcclusionMap || pbrOptions.bits.useRoughnessMap || pbrOptions.bits.useMetalnessMap)
			{
				auto texturePath = dir / *mesh.material.occlusionRoughnessMetalness;
				pbrEffect.resources.orm = CreateTexture(pDevice, texturePath.string());
			}
			if (pbrOptions.bits.useNormalMap)
			{
				auto texturePath = dir / *mesh.material.normal;
				pbrEffect.resources.normal = CreateTexture(pDevice, texturePath.string());
			}

			// Set constants
			auto& cbuffer = pbrEffect.GetConstants();
			cbuffer.metallicFactor = mesh.material.metallicFactor;
			cbuffer.roughnessFactor = mesh.material.roughnessFactor;
			cbuffer.baseColorFactor = mesh.material.baseColorFactor;

			auto entity = m_pRegistry->create();
			m_pRegistry->emplace<PBREffect>(entity, std::move(pbrEffect));
			m_pRegistry->emplace<ShadowMapEffect>(entity, std::move(shadowEffect));
			
			IndexBuffer indices(pDevice, mesh.indices);
			switch (mesh.vertexType)
			{
			case VertexType::eP3N3:
			{
				Geometry geometry{ VertexBuffer(pDevice, std::get<std::vector<VertexP3N3>>(mesh.vertices)), std::move(indices) };
				m_pRegistry->emplace<Geometry>(entity, std::move(geometry));
				break;
			}
			case VertexType::eP3N3U2:
			{
				Geometry geometry{ VertexBuffer(pDevice, std::get<std::vector<VertexP3N3U2>>(mesh.vertices)), std::move(indices) };
				m_pRegistry->emplace<Geometry>(entity, std::move(geometry));
				break;
			}
			case VertexType::eP3N3U2T3:
			{
				Geometry geometry{ VertexBuffer(pDevice, std::get<std::vector<VertexP3N3U2T3>>(mesh.vertices)), std::move(indices) };
				m_pRegistry->emplace<Geometry>(entity, std::move(geometry));
				break;
			}
			}
			AddNode(entity, localTranslation, localRotation, localScale, parent);
		}
	}
}