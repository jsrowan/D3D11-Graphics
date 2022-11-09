#pragma once

#include "Buffers.h"

namespace dx
{
	struct Geometry
	{
		VertexBuffer vertices;
		IndexBuffer indices;

		void Bind(ID3D11DeviceContext* pContext) const
		{
			vertices.Bind(pContext, 0);
			indices.Bind(pContext);
		}
	};

	struct Light
	{
		enum class Type
		{
			ePoint,
			eDirectional,
			eSpot
		};

		struct Data
		{
			DirectX::XMFLOAT3 position;
			float intensity;
			DirectX::XMFLOAT3 direction;
			float innerAngle;
			DirectX::XMFLOAT3 color;
			float outerAngle;
		};

		Data data;
		Type type;
		bool castsShadows;
	};
}