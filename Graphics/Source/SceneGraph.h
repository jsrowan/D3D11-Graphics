#pragma once

#include "D3DCache.h"

namespace dx
{
	class Transform
	{
	public:
		struct Data
		{
			DirectX::XMFLOAT3 translation;
			DirectX::XMFLOAT4 rotation;
			DirectX::XMFLOAT3 scale;

			Data(const DirectX::XMFLOAT3& t, const DirectX::XMFLOAT4& r,
				const DirectX::XMFLOAT3& s) :
				translation(t), rotation(r), scale(s) { }
		};

		Transform(const DirectX::XMFLOAT3& t, const DirectX::XMFLOAT4& r,
			const DirectX::XMFLOAT3& s, entt::entity o, Transform* p) :
			local(t, r, s), global(t, r, s), owner(o), pParent(p), dirty(true) { }

		friend class SceneGraph;

	private:
		Data local;				// Local transform
		mutable Data global;	// Pre-cached global transform

		entt::entity owner;

		Transform* pParent;
		std::vector<Transform*> pChildren;

		mutable bool dirty;
	};

	class SceneGraph
	{
	public:
		SceneGraph(std::shared_ptr<entt::registry> registry);

		// Add a node to the scenegraph
		void AddNode(entt::entity node, 
			const DirectX::XMFLOAT3& localTranslation = {0.0f, 0.0f, 0.0f},
			const DirectX::XMFLOAT4& localRotation = {0.0f, 0.0f, 0.0f, 1.0f}, 
			const DirectX::XMFLOAT3& localScale = {1.0f, 1.0f, 1.0f},
			entt::entity parent = entt::null);
		void AddNode(entt::entity node,
			const DirectX::XMFLOAT3& localTranslation = { 0.0f, 0.0f, 0.0f },
			const DirectX::XMFLOAT4& localRotation = { 0.0f, 0.0f, 0.0f, 1.0f },
			const DirectX::XMFLOAT3& localScale = { 1.0f, 1.0f, 1.0f },
			Transform* parent = nullptr);

		// Destroy a node
		void DestroyNode(entt::entity node);
		void DestroyNode(Transform* t);

		// Get the global transform, recursively performing any calculations necessary
		const Transform::Data& GetGlobalTransform(entt::entity node); 
		const Transform::Data& GetGlobalTransform(Transform* t) const;

		// Set a new local transform and set dirty flags on the node and its children
		void SetTransform(entt::entity node, const DirectX::XMFLOAT3& localTranslation,
			const DirectX::XMFLOAT4& localRotation, const DirectX::XMFLOAT3& localScale);
		void SetTransform(Transform* t, const DirectX::XMFLOAT3& localTranslation,
			const DirectX::XMFLOAT4& localRotation, const DirectX::XMFLOAT3& localScale);

		void LoadModel(ID3D11Device* pDevice, D3DCache& cache, const std::string& path,
			const DirectX::XMFLOAT3& localTranslation = { 0.0f, 0.0f, 0.0f },
			const DirectX::XMFLOAT4& localRotation = { 0.0f, 0.0f, 0.0f, 1.0f },
			const DirectX::XMFLOAT3& localScale = { 1.0f, 1.0f, 1.0f },
			entt::entity parent = entt::null);

	private:
		Transform* m_pRootTransform;
		entt::entity m_root;
		std::shared_ptr<entt::registry> m_pRegistry;
	};
}