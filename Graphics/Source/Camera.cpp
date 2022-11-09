#include "stdafx.h"

#include "Camera.h"

namespace dx
{
    FlyCamera::FlyCamera(unsigned int screenWidth, unsigned int screenHeight, float nearPlane, float farPlane, float fov) :
        m_nearPlane(nearPlane), m_farPlane(farPlane), m_fov(fov),
        m_pos(0.0f, 0.0f, 3.0f), m_front(0.0f, 0.0f, -1.0f),
        m_pitch(0.0f), m_yaw(-90.0f), m_captured(false)
    {
        // Set initial projection matrix
        OnResize(screenWidth, screenHeight);
        // Set initial view matrix
        UpdateViewMatrix();
    }

    void FlyCamera::Update()
    {
        using namespace DirectX;

        // Mouse input
        auto front = XMLoadFloat3(&m_front);
        auto mouse = Mouse::GetInstance().GetState();
        static Mouse::ButtonStateTracker tracker;
        static int lastdx = 0;
        static int lastdy = 0;
        auto [dx, dy] = mouse.delta;
        tracker.Update();
        
        if (tracker.WasPressed(Button::eRight))
        {
            // Toggle capture
            m_captured = !m_captured;
            Mouse::GetInstance().ShowCursor(!m_captured);
            // Set these so the camera doesn't jump when we first capture the cursor.
            lastdx = dx;
            lastdy = dy;
        }
        // Only update the camera if the mouse is captured.
        if (!m_captured)
        {
            return;
        }

        bool mouseDirty = (dx != lastdx) || (dy != lastdy);
        lastdx = dx;
        lastdy = dy;

        if (mouseDirty)
        {
            // Update angles and front vector based on mouse movement
            m_yaw += SENSITIVITY * static_cast<float>(dx);
            m_pitch += SENSITIVITY * static_cast<float>(dy);
            constexpr float maxPitch = XMConvertToRadians(89.0f);
            m_pitch = std::clamp(m_pitch, -maxPitch, maxPitch);
            if (m_yaw > 2.0f * DirectX::XM_PI)
            {
                m_yaw -= 2.0f * DirectX::XM_PI;
            }
            else if (m_yaw < 0.0f)
            {
                m_yaw += 2.0f * DirectX::XM_PI;
            }

            float y = sinf(m_pitch);
            float r = cosf(m_pitch);
            float x = r * cosf(m_yaw);
            float z = r * sinf(m_yaw);
            front = XMVector3Normalize(DirectX::XMVectorSet(x, y, z, 0.0f));
            XMStoreFloat3(&m_front, front);
        }

        // Keyboard input
        auto right = XMVector3Cross(front,
            XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        auto move = XMVectorZero();

        auto kb = Keyboard::GetInstance().GetState();
        bool kbDirty = false;
        if (kb[Key::eW] == KeyState::eDown)
        {
            move = XMVectorAdd(move, front);
            kbDirty = true;
        }
        if (kb[Key::eS] == KeyState::eDown)
        {
            move = XMVectorSubtract(move, front);
            kbDirty = true;
        }
        if (kb[Key::eA] == KeyState::eDown)
        {
            move = XMVectorSubtract(move, right);
            kbDirty = true;
        }
        if (kb[Key::eD] == KeyState::eDown)
        {
            move = XMVectorAdd(move, right);
            kbDirty = true;
        }
        if (kb[Key::ePageUp] == KeyState::eDown)
        {
            move = XMVectorAdd(move, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
            kbDirty = true;
        }
        if (kb[Key::ePageDown] == KeyState::eDown)
        {
            move = XMVectorSubtract(move, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
            kbDirty = true;
        }

        if (kbDirty)
        {
            // Update position based on key presses
            auto currPos = XMLoadFloat3(&m_pos);
            move = XMVectorScale(move, 0.05f);
            XMStoreFloat3(&m_pos, XMVectorAdd(currPos, move));
        }

        // Finally update the view matrix if required
        if (kbDirty || mouseDirty)
        {
            UpdateViewMatrix();
        }
    }

    void FlyCamera::OnResize(unsigned int screenWidth, unsigned int screenHeight)
    {
        using namespace DirectX;

        // Rebuild the projection matrix
        const float aspectRatio = 1.0f * screenWidth / screenHeight;
        const auto proj = XMMatrixPerspectiveFovRH(m_fov, aspectRatio,
            m_nearPlane, m_farPlane);
        XMStoreFloat4x4(&m_proj, proj);
    }

    void FlyCamera::UpdateViewMatrix()
    {
        using namespace DirectX;

        const auto eye = XMLoadFloat3(&m_pos);
        const auto front = XMLoadFloat3(&m_front);
        const auto focus = XMVectorAdd(eye, front);
        XMStoreFloat4x4(&m_view, XMMatrixLookAtRH(eye, focus, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
    }
}