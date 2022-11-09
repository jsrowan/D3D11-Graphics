#pragma once

#include "D3DHelper.h"

namespace dx
{
	struct Effect
	{
		virtual ~Effect() = default;

		virtual void Bind(ID3D11DeviceContext* pContext) const = 0;
	};
}