#pragma once
#include <DirectXMath.h>
namespace component {

    class Transform
    {
    public:
        Transform() = default;
        Transform(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& position)
            : m_Scale(scale), m_Position(position)
        {
            SetRotation(rotation);
        }
        ~Transform() = default;

        Transform(const Transform&) = default;
        Transform& operator=(const Transform&) = default;

        Transform(Transform&&) = default;
        Transform& operator=(Transform&&) = default;

        // ��ȡ�������ű���
        DirectX::XMFLOAT3 GetScale() const { return m_Scale; }
        // ��ȡ�������ű���
        DirectX::XMVECTOR GetScaleXM() const { return XMLoadFloat3(&m_Scale); }

        // ��ȡ����ŷ����(������)
        // ������Z-X-Y��˳����ת
        DirectX::XMFLOAT3 GetRotation() const
        {
            float sinX = 2 * (m_Rotation.w * m_Rotation.x - m_Rotation.y * m_Rotation.z);
            float sinY_cosX = 2 * (m_Rotation.w * m_Rotation.y + m_Rotation.x * m_Rotation.z);
            float cosY_cosX = 1 - 2 * (m_Rotation.x * m_Rotation.x + m_Rotation.y * m_Rotation.y);
            float sinZ_cosX = 2 * (m_Rotation.w * m_Rotation.z + m_Rotation.x * m_Rotation.y);
            float cosZ_cosX = 1 - 2 * (m_Rotation.x * m_Rotation.x + m_Rotation.z * m_Rotation.z);

            DirectX::XMFLOAT3 rotation;
            if (fabs(sinX) >= 1.0f)
                rotation.x = copysignf(DirectX::XM_PI / 2, sinX);
            else
                rotation.x = asinf(sinX);
            rotation.y = atan2f(sinY_cosX, cosY_cosX);
            rotation.z = atan2f(sinZ_cosX, cosZ_cosX);

            return rotation;
        }
        // ��ȡ������ת��Ԫ��
        DirectX::XMFLOAT4 GetRotationQuat() const { return m_Rotation; }
        // ��ȡ����ŷ����(������)
        // ������Z-X-Y��˳����ת
        DirectX::XMVECTOR GetRotationXM() const { auto rot = GetRotation(); return XMLoadFloat3(&rot); }
        // ��ȡ������ת��Ԫ��
        DirectX::XMVECTOR GetRotationQuatXM() const { return XMLoadFloat4(&m_Rotation); }

        // ��ȡ����λ��
        DirectX::XMFLOAT3 GetPosition() const { return m_Position; }
        // ��ȡ����λ��
        DirectX::XMVECTOR GetPositionXM() const { return XMLoadFloat3(&m_Position); }

        // ��ȡ�ҷ�����
        DirectX::XMFLOAT3 GetRightAxis() const
        {
            using namespace DirectX;
            XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&m_Rotation));
            XMFLOAT3 right;
            XMStoreFloat3(&right, R.r[0]);
            return right;
        }
        // ��ȡ�ҷ�����
        DirectX::XMVECTOR GetRightAxisXM() const
        {
            DirectX::XMFLOAT3 right = GetRightAxis();
            return DirectX::XMLoadFloat3(&right);
        }

        // ��ȡ�Ϸ�����
        DirectX::XMFLOAT3 GetUpAxis() const
        {
            using namespace DirectX;
            XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&m_Rotation));
            XMFLOAT3 up;
            XMStoreFloat3(&up, R.r[1]);
            return up;
        }
        // ��ȡ�Ϸ�����
        DirectX::XMVECTOR GetUpAxisXM() const
        {
            DirectX::XMFLOAT3 up = GetUpAxis();
            return DirectX::XMLoadFloat3(&up);
        }

        // ��ȡǰ������
        DirectX::XMFLOAT3 GetForwardAxis() const
        {
            using namespace DirectX;
            XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&m_Rotation));
            XMFLOAT3 forward;
            XMStoreFloat3(&forward, R.r[2]);
            return forward;
        }
        // ��ȡǰ������
        DirectX::XMVECTOR GetForwardAxisXM() const
        {
            DirectX::XMFLOAT3 forward = GetForwardAxis();
            return DirectX::XMLoadFloat3(&forward);
        }

        // ��ȡ����任����
        DirectX::XMFLOAT4X4 GetLocalToWorldMatrix() const
        {
            DirectX::XMFLOAT4X4 res;
            DirectX::XMStoreFloat4x4(&res, GetLocalToWorldMatrixXM());
            return res;
        }
        // ��ȡ����任����
        DirectX::XMMATRIX GetLocalToWorldMatrixXM() const
        {
            using namespace DirectX;
            DirectX::XMVECTOR scaleVec = XMLoadFloat3(&m_Scale);
            DirectX::XMVECTOR quateration = XMLoadFloat4(&m_Rotation);
            DirectX::XMVECTOR positionVec = XMLoadFloat3(&m_Position);
            DirectX::XMMATRIX World = XMMatrixAffineTransformation(scaleVec, g_XMZero, quateration, positionVec);
            return World;
        }

        // ��ȡ����任����
        DirectX::XMFLOAT4X4 GetWorldToLocalMatrix() const
        {
            DirectX::XMFLOAT4X4 res;
            DirectX::XMStoreFloat4x4(&res, GetWorldToLocalMatrixXM());
            return res;
        }
        // ��ȡ������任����
        DirectX::XMMATRIX GetWorldToLocalMatrixXM() const
        {
            DirectX::XMMATRIX InvWorld = DirectX::XMMatrixInverse(nullptr, GetLocalToWorldMatrixXM());
            return InvWorld;
        }

        // ���ö������ű���
        void SetScale(const DirectX::XMFLOAT3& scale) { m_Scale = scale; }
        // ���ö������ű���
        void SetScale(float x, float y, float z) { m_Scale = DirectX::XMFLOAT3(x, y, z); }

        // ���ö���ŷ����(������)
        // ������Z-X-Y��˳����ת
        void SetRotation(const DirectX::XMFLOAT3& eulerAnglesInRadian)
        {
            auto quat = DirectX::XMQuaternionRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&eulerAnglesInRadian));
            DirectX::XMStoreFloat4(&m_Rotation, quat);
        }
        // ���ö���ŷ����(������)
        // ������Z-X-Y��˳����ת
        void SetRotation(float x, float y, float z)
        {
            auto quat = DirectX::XMQuaternionRotationRollPitchYaw(x, y, z);
            DirectX::XMStoreFloat4(&m_Rotation, quat);
        }

        // ���ö���λ��
        void SetPosition(const DirectX::XMFLOAT3& position) { m_Position = position; }
        // ���ö���λ��
        void SetPosition(float x, float y, float z) { m_Position = DirectX::XMFLOAT3(x, y, z); }

        // ָ��ŷ������ת����
        void Rotate(const DirectX::XMFLOAT3& eulerAnglesInRadian)
        {
            using namespace DirectX;
            auto newQuat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&eulerAnglesInRadian));
            auto quat = XMLoadFloat4(&m_Rotation);
            XMStoreFloat4(&m_Rotation, XMQuaternionMultiply(quat, newQuat));
        }
        // ָ����ԭ��Ϊ����������ת
        void RotateAxis(const DirectX::XMFLOAT3& axis, float radian)
        {
            using namespace DirectX;
            auto newQuat = XMQuaternionRotationAxis(XMLoadFloat3(&axis), radian);
            auto quat = XMLoadFloat4(&m_Rotation);
            XMStoreFloat4(&m_Rotation, XMQuaternionMultiply(quat, newQuat));
        }
        // ָ����pointΪ��ת����������ת
        void RotateAround(const DirectX::XMFLOAT3& point, const DirectX::XMFLOAT3& axis, float radian)
        {
            using namespace DirectX;
            XMVECTOR quat = XMLoadFloat4(&m_Rotation);
            XMVECTOR positionVec = XMLoadFloat3(&m_Position);
            XMVECTOR centerVec = XMLoadFloat3(&point);

            // ��point��Ϊԭ�������ת
            XMMATRIX RT = XMMatrixRotationQuaternion(quat) * XMMatrixTranslationFromVector(positionVec - centerVec);
            RT *= XMMatrixRotationAxis(XMLoadFloat3(&axis), radian);
            RT *= XMMatrixTranslationFromVector(centerVec);
            XMStoreFloat4(&m_Rotation, XMQuaternionRotationMatrix(RT));
            XMStoreFloat3(&m_Position, RT.r[3]);
        }
        // ����ĳһ����ƽ��
        void Translate(const DirectX::XMFLOAT3& direction, float magnitude)
        {
            using namespace DirectX;
            XMVECTOR directionVec = XMVector3Normalize(XMLoadFloat3(&direction));
            XMVECTOR newPosition = XMVectorMultiplyAdd(XMVectorReplicate(magnitude), directionVec, XMLoadFloat3(&m_Position));
            XMStoreFloat3(&m_Position, newPosition);
        }

        // �۲�ĳһ��
        void LookAt(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up = { 0.0f, 1.0f, 0.0f })
        {
            using namespace DirectX;
            XMMATRIX View = XMMatrixLookAtLH(XMLoadFloat3(&m_Position), XMLoadFloat3(&target), XMLoadFloat3(&up));
            XMMATRIX InvView = XMMatrixInverse(nullptr, View);
            XMStoreFloat4(&m_Rotation, XMQuaternionRotationMatrix(InvView));
        }
        // ����ĳһ����۲�
        void LookTo(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& up = { 0.0f, 1.0f, 0.0f })
        {
            using namespace DirectX;
            XMMATRIX View = XMMatrixLookToLH(XMLoadFloat3(&m_Position), XMLoadFloat3(&direction), XMLoadFloat3(&up));
            XMMATRIX InvView = XMMatrixInverse(nullptr, View);
            XMStoreFloat4(&m_Rotation, XMQuaternionRotationMatrix(InvView));
        }
    private:
        // ����ת�����ȡ��תŷ����
        DirectX::XMFLOAT3 GetEulerAnglesFromRotationMatrix(const DirectX::XMFLOAT4X4& rotationMatrix)
        {
            // ͨ����ת������ŷ����
            float c = sqrtf(1.0f - rotationMatrix(2, 1) * rotationMatrix(2, 1));
            // ��ֹr[2][1]���ִ���1�����
            if (isnan(c))
                c = 0.0f;
            DirectX::XMFLOAT3 rotation{};
            rotation.z = atan2f(rotationMatrix(0, 1), rotationMatrix(1, 1));
            rotation.x = atan2f(-rotationMatrix(2, 1), c);
            rotation.y = atan2f(rotationMatrix(2, 0), rotationMatrix(2, 2));
            return rotation;
        }

    private:
        DirectX::XMFLOAT3 m_Scale = { 1.0f, 1.0f, 1.0f };				// ����
        DirectX::XMFLOAT4 m_Rotation = { 0.0f, 0.0f, 0.0f, 1.0f };		// ��ת��Ԫ��
        DirectX::XMFLOAT3 m_Position = {};								// λ��
    };

}