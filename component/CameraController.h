#pragma once

#include "Camera.h"

namespace component {
    class CameraController
    {
    public:
        CameraController() = default;
        CameraController& operator=(const CameraController&) = default;
        virtual ~CameraController() {}
        virtual void Update(float deltaTime) = 0;
    };

    class FirstPersonCameraController : public CameraController
    {
    public:

        ~FirstPersonCameraController() override {};
        void Update(float deltaTime) override;

        void InitCamera(FirstPersonCamera* pCamera);

        void SetMouseSensitivity(float x, float y);
        void SetMoveSpeed(float speed);
        void OnResize();

    private:
        FirstPersonCamera* m_pCamera = nullptr;

        float m_MoveSpeed = 5.0f;
        float m_MouseSensitivityX = 0.005f;
        float m_MouseSensitivityY = 0.005f;

        float m_CurrentYaw = 0.0f;
        float m_CurrentPitch = 0.0f;

        DirectX::XMFLOAT3 m_MoveDir{};
        float m_MoveVelocity = 0.0f;
        float m_VelocityDrag = 0.0f;
        float m_TotalDragTimeToZero = 0.25f;
        float m_DragTimer = 0.0f;
    };
}