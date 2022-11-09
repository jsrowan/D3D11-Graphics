#pragma once

namespace dx
{
	struct VertexP3N3
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(position, normal);
		}
	};

	struct VertexP3N3U2
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(position, normal, uv);
		}
	};

	struct VertexP3N3U2T3
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 tangent;

		template<typename Archive>
		void serialize(Archive& ar)
		{
			ar(position, normal, uv, tangent);
		}
	};

	enum class VertexType
	{
		eP3N3,
		eP3N3U2,
		eP3N3U2T3
	};
}