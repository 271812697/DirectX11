#include "CameraController.h"
#include "../XUtil.h"
#include"../d3dApp.h"
#include <imgui.h>
namespace component {
    using namespace DirectX;
    void FirstPersonCameraController::Update(float deltaTime)
    {
        ImGuiIO& io = ImGui::GetIO();
        float yaw = 0.0f, pitch = 0.0f;
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            yaw += io.MouseDelta.x * m_MouseSensitivityX;
            pitch += io.MouseDelta.y * m_MouseSensitivityY;
        }
        int forward = (
            (ImGui::IsKeyDown('W') ? 1 : 0) +
            (ImGui::IsKeyDown('S') ? -1 : 0)
            );
        int strafe = (
            (ImGui::IsKeyDown('A') ? -1 : 0) +
            (ImGui::IsKeyDown('D') ? 1 : 0)
            );
        int up = (
            (ImGui::IsKeyDown('E') ? -1 : 0) +
            (ImGui::IsKeyDown('Q') ? 1 : 0)
            );
        DirectX::XMFLOAT3 MoveDir{};
        if (forward || strafe || up)
        {
            XMVECTOR dir = m_pCamera->GetLookAxisXM() * (float)forward + m_pCamera->GetRightAxisXM() * (float)strafe + m_pCamera->GetUpAxisXM() * (float)up;
            XMStoreFloat3(&MoveDir, dir);
        }
        m_pCamera->RotateY(yaw);
        m_pCamera->Pitch(pitch);
        m_pCamera->Translate(MoveDir, m_MoveSpeed * deltaTime);
    }

    void FirstPersonCameraController::InitCamera(FirstPersonCamera* pCamera)
    {
        //初始化相机和相机控制器  
        m_pCamera = pCamera;
        auto instance= D3DApp::Get();
        
        m_pCamera->SetViewPort(instance->m_ScreenViewport);
        m_pCamera->SetFrustum(45, instance->AspectRatio(), 0.1, 1000.0);
      
    }

    void FirstPersonCameraController::SetMouseSensitivity(float x, float y)
    {
        m_MouseSensitivityX = x;
        m_MouseSensitivityY = y;
    }

    void FirstPersonCameraController::SetMoveSpeed(float speed)
    {
        m_MoveSpeed = speed;
    }
    void FirstPersonCameraController::OnResize()
    {
       // g_Fcamera.SetViewPort(m_ScreenViewport);
        //g_Fcamera.SetFrustum(45, this->AspectRatio(), 0.1, 1000.0);
    }
}