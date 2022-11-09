#pragma once

#include "Keyboard.h"
#include "Mouse.h"

namespace dx
{
    class FlyCamera
    {
    public:
        FlyCamera(unsigned int screenWidth, unsigned int screenHeight, float nearPlane = DEFAULT_NEAR_PLANE,
            float farPlane = DEFAULT_FAR_PLANE, float fov = DEFAULT_FOV);

        void Update();
        void OnResize(unsigned int screenWidth, unsigned int screenHeight);

        DirectX::XMMATRIX GetViewMatrix() const
        {
            return DirectX::XMLoadFloat4x4(&m_view);
        }

        DirectX::XMMATRIX GetProjectionMatrix() const
        {
            return DirectX::XMLoadFloat4x4(&m_proj);
        }

        DirectX::XMMATRIX GetViewProjectionMatrix() const
        {
            return DirectX::XMMatrixMultiply(GetViewMatrix(), GetProjectionMatrix());
        }

        DirectX::XMVECTOR GetEyePosition() const
        {
            return DirectX::XMLoadFloat3(&m_pos);
        }

        std::pair<float, float> GetClipPlanes() const
        {
            return std::pair{ m_nearPlane, m_farPlane };
        }

        void SetEyePosition(const DirectX::XMFLOAT3& pos)
        {
            m_pos = pos;
            UpdateViewMatrix();
        }

        void LookAt(const DirectX::XMFLOAT3& focus)
        {
            auto v = DirectX::XMLoadFloat3(&focus);
            auto front = DirectX::XMVectorSubtract(v, GetEyePosition());
            front = DirectX::XMVector3Normalize(front);
            DirectX::XMStoreFloat3(&m_front, front);
            UpdateViewMatrix();
        }

        void LookTo(const DirectX::XMFLOAT3& dir)
        {
            m_front = dir;
            UpdateViewMatrix();
        }

        static constexpr float DEFAULT_NEAR_PLANE = 0.2f;
        static constexpr float DEFAULT_FAR_PLANE = 25.0f;
        static constexpr float DEFAULT_FOV = DirectX::XMConvertToRadians(50.0f);

    private:
        // Perspective projection values
        DirectX::XMFLOAT4X4 m_proj;
        float m_nearPlane;
        float m_farPlane;
        float m_fov;

        DirectX::XMFLOAT3 m_pos;
        DirectX::XMFLOAT3 m_front;
        DirectX::XMFLOAT4X4 m_view;
        float m_pitch;
        float m_yaw;

        // Is the mouse captured and hidden (for FPS controls)?
        bool m_captured;

        static constexpr float SENSITIVITY = 0.01f;

        void UpdateViewMatrix();
    };
}
