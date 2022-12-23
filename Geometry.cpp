#include "Geometry.h"

namespace Geometry
{
    constexpr float PI = 3.1415926f;
    //
    // �����巽����ʵ��
    //

    GeometryData CreateSphere(float radius, uint32_t levels, uint32_t slices)
    {
        using namespace DirectX;

        GeometryData vertices;

        uint32_t vertexCount = 2 + (levels - 1) * (slices + 1);
        uint32_t indexCount = 6 * (levels - 1) * slices;
        vertices.vertices.resize(vertexCount);
        vertices.normals.resize(vertexCount);
        vertices.texcoords.resize(vertexCount);
        vertices.tangents.resize(vertexCount);
        if (indexCount > 65535)
            vertices.indices32.resize(indexCount);
        else
            vertices.indices16.resize(indexCount);

        uint32_t vIndex = 0, iIndex = 0;

        float phi = 0.0f, theta = 0.0f;
        float per_phi = PI / levels;
        float per_theta = 2 * PI / slices;
        float x, y, z;

        // ���붥�˵�
        vertices.vertices[vIndex] = XMFLOAT3(0.0f, radius, 0.0f);
        vertices.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
        vertices.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
        vertices.texcoords[vIndex++] = XMFLOAT2(0.0f, 0.0f);


        for (uint32_t i = 1; i < levels; ++i)
        {
            phi = per_phi * i;
            // ��Ҫslices + 1����������Ϊ �����յ���Ϊͬһ�㣬����������ֵ��һ��
            for (uint32_t j = 0; j <= slices; ++j)
            {
                theta = per_theta * j;
                x = radius * sinf(phi) * cosf(theta);
                y = radius * cosf(phi);
                z = radius * sinf(phi) * sinf(theta);
                // ������ֲ����ꡢ��������Tangent��������������
                XMFLOAT3 pos = XMFLOAT3(x, y, z);

                vertices.vertices[vIndex] = pos;
                XMStoreFloat3(&vertices.normals[vIndex], XMVector3Normalize(XMLoadFloat3(&pos)));
                vertices.tangents[vIndex] = XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f);
                vertices.texcoords[vIndex++] = XMFLOAT2(theta / 2 / PI, phi / PI);
            }
        }

        // ����׶˵�
        vertices.vertices[vIndex] = XMFLOAT3(0.0f, -radius, 0.0f);
        vertices.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
        vertices.tangents[vIndex] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
        vertices.texcoords[vIndex++] = XMFLOAT2(0.0f, 1.0f);


        // ��������
        if (levels > 1)
        {
            for (uint32_t j = 1; j <= slices; ++j)
            {
                if (indexCount > 65535)
                {
                    vertices.indices32[iIndex++] = 0;
                    vertices.indices32[iIndex++] = j % (slices + 1) + 1;
                    vertices.indices32[iIndex++] = j;
                }
                else
                {
                    vertices.indices16[iIndex++] = 0;
                    vertices.indices16[iIndex++] = j % (slices + 1) + 1;
                    vertices.indices16[iIndex++] = j;
                }
            }
        }


        for (uint32_t i = 1; i < levels - 1; ++i)
        {
            for (uint32_t j = 1; j <= slices; ++j)
            {
                if (indexCount > 65535)
                {
                    vertices.indices32[iIndex++] = (i - 1) * (slices + 1) + j;
                    vertices.indices32[iIndex++] = (i - 1) * (slices + 1) + j % (slices + 1) + 1;
                    vertices.indices32[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;

                    vertices.indices32[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;
                    vertices.indices32[iIndex++] = i * (slices + 1) + j;
                    vertices.indices32[iIndex++] = (i - 1) * (slices + 1) + j;
                }
                else
                {
                    vertices.indices16[iIndex++] = (i - 1) * (slices + 1) + j;
                    vertices.indices16[iIndex++] = (i - 1) * (slices + 1) + j % (slices + 1) + 1;
                    vertices.indices16[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;

                    vertices.indices16[iIndex++] = i * (slices + 1) + j % (slices + 1) + 1;
                    vertices.indices16[iIndex++] = i * (slices + 1) + j;
                    vertices.indices16[iIndex++] = (i - 1) * (slices + 1) + j;
                }

            }
        }

        // �𽥷�������
        if (levels > 1)
        {
            for (uint32_t j = 1; j <= slices; ++j)
            {
                if (indexCount > 65535)
                {
                    vertices.indices32[iIndex++] = (levels - 2) * (slices + 1) + j;
                    vertices.indices32[iIndex++] = (levels - 2) * (slices + 1) + j % (slices + 1) + 1;
                    vertices.indices32[iIndex++] = (levels - 1) * (slices + 1) + 1;
                }
                else
                {
                    vertices.indices16[iIndex++] = (levels - 2) * (slices + 1) + j;
                    vertices.indices16[iIndex++] = (levels - 2) * (slices + 1) + j % (slices + 1) + 1;
                    vertices.indices16[iIndex++] = (levels - 1) * (slices + 1) + 1;
                }
            }
        }

        return vertices;
    }

    GeometryData CreateBox(float width, float height, float depth)
    {
        using namespace DirectX;

        GeometryData vertices;

        vertices.vertices.resize(24);
        vertices.normals.resize(24);
        vertices.tangents.resize(24);
        vertices.texcoords.resize(24);

        float w2 = width / 2, h2 = height / 2, d2 = depth / 2;

        // ����(+X��)
        vertices.vertices[0] = XMFLOAT3(w2, -h2, -d2);
        vertices.vertices[1] = XMFLOAT3(w2, h2, -d2);
        vertices.vertices[2] = XMFLOAT3(w2, h2, d2);
        vertices.vertices[3] = XMFLOAT3(w2, -h2, d2);
        // ����(-X��)
        vertices.vertices[4] = XMFLOAT3(-w2, -h2, d2);
        vertices.vertices[5] = XMFLOAT3(-w2, h2, d2);
        vertices.vertices[6] = XMFLOAT3(-w2, h2, -d2);
        vertices.vertices[7] = XMFLOAT3(-w2, -h2, -d2);
        // ����(+Y��)
        vertices.vertices[8] = XMFLOAT3(-w2, h2, -d2);
        vertices.vertices[9] = XMFLOAT3(-w2, h2, d2);
        vertices.vertices[10] = XMFLOAT3(w2, h2, d2);
        vertices.vertices[11] = XMFLOAT3(w2, h2, -d2);
        // ����(-Y��)
        vertices.vertices[12] = XMFLOAT3(w2, -h2, -d2);
        vertices.vertices[13] = XMFLOAT3(w2, -h2, d2);
        vertices.vertices[14] = XMFLOAT3(-w2, -h2, d2);
        vertices.vertices[15] = XMFLOAT3(-w2, -h2, -d2);
        // ����(+Z��)
        vertices.vertices[16] = XMFLOAT3(w2, -h2, d2);
        vertices.vertices[17] = XMFLOAT3(w2, h2, d2);
        vertices.vertices[18] = XMFLOAT3(-w2, h2, d2);
        vertices.vertices[19] = XMFLOAT3(-w2, -h2, d2);
        // ����(-Z��)
        vertices.vertices[20] = XMFLOAT3(-w2, -h2, -d2);
        vertices.vertices[21] = XMFLOAT3(-w2, h2, -d2);
        vertices.vertices[22] = XMFLOAT3(w2, h2, -d2);
        vertices.vertices[23] = XMFLOAT3(w2, -h2, -d2);

        for (size_t i = 0; i < 4; ++i)
        {
            // ����(+X��)
            vertices.normals[i] = XMFLOAT3(1.0f, 0.0f, 0.0f);
            vertices.tangents[i] = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
            // ����(-X��)
            vertices.normals[i + 4] = XMFLOAT3(-1.0f, 0.0f, 0.0f);
            vertices.tangents[i + 4] = XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f);
            // ����(+Y��)
            vertices.normals[i + 8] = XMFLOAT3(0.0f, 1.0f, 0.0f);
            vertices.tangents[i + 8] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
            // ����(-Y��)
            vertices.normals[i + 12] = XMFLOAT3(0.0f, -1.0f, 0.0f);
            vertices.tangents[i + 12] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
            // ����(+Z��)
            vertices.normals[i + 16] = XMFLOAT3(0.0f, 0.0f, 1.0f);
            vertices.tangents[i + 16] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
            // ����(-Z��)
            vertices.normals[i + 20] = XMFLOAT3(0.0f, 0.0f, -1.0f);
            vertices.tangents[i + 20] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
        }

        for (size_t i = 0; i < 6; ++i)
        {
            vertices.texcoords[i * 4] = XMFLOAT2(0.0f, 1.0f);
            vertices.texcoords[i * 4 + 1] = XMFLOAT2(0.0f, 0.0f);
            vertices.texcoords[i * 4 + 2] = XMFLOAT2(1.0f, 0.0f);
            vertices.texcoords[i * 4 + 3] = XMFLOAT2(1.0f, 1.0f);
        }

        vertices.indices16.resize(36);

        uint16_t indices[] = {
            0, 1, 2, 2, 3, 0,		// ����(+X��)
            4, 5, 6, 6, 7, 4,		// ����(-X��)
            8, 9, 10, 10, 11, 8,	// ����(+Y��)
            12, 13, 14, 14, 15, 12,	// ����(-Y��)
            16, 17, 18, 18, 19, 16, // ����(+Z��)
            20, 21, 22, 22, 23, 20	// ����(-Z��)
        };
        memcpy_s(vertices.indices16.data(), sizeof indices, indices, sizeof indices);

        return vertices;
    }

    GeometryData CreateCylinder(float radius, float height, uint32_t slices, uint32_t stacks, float texU, float texV)
    {
        using namespace DirectX;

        GeometryData vertices;
        uint32_t vertexCount = (slices + 1) * (stacks + 3) + 2;
        uint32_t indexCount = 6 * slices * (stacks + 1);

        vertices.vertices.resize(vertexCount);
        vertices.normals.resize(vertexCount);
        vertices.tangents.resize(vertexCount);
        vertices.texcoords.resize(vertexCount);

        if (indexCount > 65535)
            vertices.indices32.resize(indexCount);
        else
            vertices.indices16.resize(indexCount);

        float h2 = height / 2;
        float theta = 0.0f;
        float per_theta = 2 * PI / slices;
        float stackHeight = height / stacks;
        //
        // ���沿��
        //
        {
            // �Ե������������˵�
            size_t vIndex = 0;
            for (size_t i = 0; i < stacks + 1; ++i)
            {
                float y = -h2 + i * stackHeight;
                // ��ǰ�㶥��
                for (size_t j = 0; j <= slices; ++j)
                {
                    theta = j * per_theta;
                    float u = theta / 2 / PI;
                    float v = 1.0f - (float)i / stacks;

                    vertices.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), y, radius * sinf(theta)), XMFLOAT3(cosf(theta), 0.0f, sinf(theta));
                    vertices.normals[vIndex] = XMFLOAT3(cosf(theta), 0.0f, sinf(theta));
                    vertices.tangents[vIndex] = XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f);
                    vertices.texcoords[vIndex++] = XMFLOAT2(u * texU, v * texV);
                }
            }

            // ��������
            size_t iIndex = 0;
            for (uint32_t i = 0; i < stacks; ++i)
            {
                for (uint32_t j = 0; j < slices; ++j)
                {
                    if (indexCount > 65535)
                    {
                        vertices.indices32[iIndex++] = i * (slices + 1) + j;
                        vertices.indices32[iIndex++] = (i + 1) * (slices + 1) + j;
                        vertices.indices32[iIndex++] = (i + 1) * (slices + 1) + j + 1;

                        vertices.indices32[iIndex++] = i * (slices + 1) + j;
                        vertices.indices32[iIndex++] = (i + 1) * (slices + 1) + j + 1;
                        vertices.indices32[iIndex++] = i * (slices + 1) + j + 1;
                    }
                    else
                    {
                        vertices.indices16[iIndex++] = i * (slices + 1) + j;
                        vertices.indices16[iIndex++] = (i + 1) * (slices + 1) + j;
                        vertices.indices16[iIndex++] = (i + 1) * (slices + 1) + j + 1;

                        vertices.indices16[iIndex++] = i * (slices + 1) + j;
                        vertices.indices16[iIndex++] = (i + 1) * (slices + 1) + j + 1;
                        vertices.indices16[iIndex++] = i * (slices + 1) + j + 1;
                    }
                }
            }
        }

        //
        // ���ǵ׸ǲ���
        //
        {
            size_t vIndex = (slices + 1) * (stacks + 1), iIndex = 6 * slices * stacks;
            uint32_t offset = static_cast<uint32_t>(vIndex);

            // ���붥��Բ��
            vertices.vertices[vIndex] = XMFLOAT3(0.0f, h2, 0.0f);
            vertices.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
            vertices.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
            vertices.texcoords[vIndex++] = XMFLOAT2(0.5f, 0.5f);

            // ���붥��Բ�ϸ���
            for (uint32_t i = 0; i <= slices; ++i)
            {
                theta = i * per_theta;
                float u = cosf(theta) * radius / height + 0.5f;
                float v = sinf(theta) * radius / height + 0.5f;
                vertices.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), h2, radius * sinf(theta));
                vertices.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
                vertices.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
                vertices.texcoords[vIndex++] = XMFLOAT2(u, v);
            }

            // ����׶�Բ��
            vertices.vertices[vIndex] = XMFLOAT3(0.0f, -h2, 0.0f);
            vertices.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
            vertices.tangents[vIndex] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
            vertices.texcoords[vIndex++] = XMFLOAT2(0.5f, 0.5f);

            // ����ײ�Բ�ϸ���
            for (uint32_t i = 0; i <= slices; ++i)
            {
                theta = i * per_theta;
                float u = cosf(theta) * radius / height + 0.5f;
                float v = sinf(theta) * radius / height + 0.5f;
                vertices.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta));
                vertices.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
                vertices.tangents[vIndex] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
                vertices.texcoords[vIndex++] = XMFLOAT2(u, v);
            }


            // ���붥������������
            for (uint32_t i = 1; i <= slices; ++i)
            {
                if (indexCount > 65535)
                {
                    vertices.indices32[iIndex++] = offset;
                    vertices.indices32[iIndex++] = offset + i % (slices + 1) + 1;
                    vertices.indices32[iIndex++] = offset + i;
                }
                else
                {
                    vertices.indices16[iIndex++] = offset;
                    vertices.indices16[iIndex++] = offset + i % (slices + 1) + 1;
                    vertices.indices16[iIndex++] = offset + i;
                }

            }

            // ����ײ�����������
            offset += slices + 2;
            for (uint32_t i = 1; i <= slices; ++i)
            {
                if (indexCount > 65535)
                {
                    vertices.indices32[iIndex++] = offset;
                    vertices.indices32[iIndex++] = offset + i;
                    vertices.indices32[iIndex++] = offset + i % (slices + 1) + 1;
                }
                else
                {
                    vertices.indices16[iIndex++] = offset;
                    vertices.indices16[iIndex++] = offset + i;
                    vertices.indices16[iIndex++] = offset + i % (slices + 1) + 1;
                }
            }
        }


        return vertices;
    }

    GeometryData CreateCone(float radius, float height, uint32_t slices)
    {
        using namespace DirectX;

        GeometryData vertices;

        uint32_t vertexCount = 3 * slices + 1;
        uint32_t indexCount = 6 * slices;
        vertices.vertices.resize(vertexCount);
        vertices.normals.resize(vertexCount);
        vertices.tangents.resize(vertexCount);
        vertices.texcoords.resize(vertexCount);

        if (indexCount > 65535)
            vertices.indices32.resize(indexCount);
        else
            vertices.indices16.resize(indexCount);

        float h2 = height / 2;
        float theta = 0.0f;
        float per_theta = 2 * PI / slices;
        float len = sqrtf(height * height + radius * radius);

        //
        // Բ׶����
        //
        {
            size_t iIndex = 0;
            size_t vIndex = 0;

            // ����Բ׶��˶���(ÿ������λ����ͬ����������ͬ�ķ���������������)
            for (uint32_t i = 0; i < slices; ++i)
            {
                theta = i * per_theta + per_theta / 2;
                vertices.vertices[vIndex] = XMFLOAT3(0.0f, h2, 0.0f);
                vertices.normals[vIndex] = XMFLOAT3(radius * cosf(theta) / len, height / len, radius * sinf(theta) / len);
                vertices.tangents[vIndex] = XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f);
                vertices.texcoords[vIndex++] = XMFLOAT2(0.5f, 0.5f);
            }

            // ����Բ׶����ײ�����
            for (uint32_t i = 0; i < slices; ++i)
            {
                theta = i * per_theta;
                vertices.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta));
                vertices.normals[vIndex] = XMFLOAT3(radius * cosf(theta) / len, height / len, radius * sinf(theta) / len);
                vertices.tangents[vIndex] = XMFLOAT4(-sinf(theta), 0.0f, cosf(theta), 1.0f);
                vertices.texcoords[vIndex++] = XMFLOAT2(cosf(theta) / 2 + 0.5f, sinf(theta) / 2 + 0.5f);
            }

            // ��������
            for (uint32_t i = 0; i < slices; ++i)
            {
                if (indexCount > 65535)
                {
                    vertices.indices32[iIndex++] = i;
                    vertices.indices32[iIndex++] = slices + (i + 1) % slices;
                    vertices.indices32[iIndex++] = slices + i % slices;
                }
                else
                {
                    vertices.indices16[iIndex++] = i;
                    vertices.indices16[iIndex++] = slices + (i + 1) % slices;
                    vertices.indices16[iIndex++] = slices + i % slices;
                }
            }
        }

        //
        // Բ׶����
        //
        {
            size_t iIndex = 3 * (size_t)slices;
            size_t vIndex = 2 * (size_t)slices;

            // ����Բ׶���涥��
            for (uint32_t i = 0; i < slices; ++i)
            {
                theta = i * per_theta;

                vertices.vertices[vIndex] = XMFLOAT3(radius * cosf(theta), -h2, radius * sinf(theta)),
                    vertices.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
                vertices.tangents[vIndex] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f);
                vertices.texcoords[vIndex++] = XMFLOAT2(cosf(theta) / 2 + 0.5f, sinf(theta) / 2 + 0.5f);
            }
            // ����Բ׶����Բ��
            vertices.vertices[vIndex] = XMFLOAT3(0.0f, -h2, 0.0f),
                vertices.normals[vIndex] = XMFLOAT3(0.0f, -1.0f, 0.0f);
            vertices.texcoords[vIndex++] = XMFLOAT2(0.5f, 0.5f);

            // ��������
            uint32_t offset = 2 * slices;
            for (uint32_t i = 0; i < slices; ++i)
            {
                if (indexCount > 65535)
                {
                    vertices.indices32[iIndex++] = offset + slices;
                    vertices.indices32[iIndex++] = offset + i % slices;
                    vertices.indices32[iIndex++] = offset + (i + 1) % slices;
                }
                else
                {
                    vertices.indices16[iIndex++] = offset + slices;
                    vertices.indices16[iIndex++] = offset + i % slices;
                    vertices.indices16[iIndex++] = offset + (i + 1) % slices;
                }

            }
        }


        return vertices;
    }

    GeometryData CreatePlane(const DirectX::XMFLOAT2& planeSize, const DirectX::XMFLOAT2& maxTexCoord)
    {
        return CreatePlane(planeSize.x, planeSize.y, maxTexCoord.x, maxTexCoord.y);
    }

    GeometryData CreatePlane(float width, float depth, float texU, float texV)
    {
        using namespace DirectX;

        GeometryData vertices;

        vertices.vertices.resize(4);
        vertices.normals.resize(4);
        vertices.tangents.resize(4);
        vertices.texcoords.resize(4);


        uint32_t vIndex = 0;
        vertices.vertices[vIndex] = XMFLOAT3(-width / 2, 0.0f, -depth / 2);
        vertices.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
        vertices.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
        vertices.texcoords[vIndex++] = XMFLOAT2(0.0f, texV);

        vertices.vertices[vIndex] = XMFLOAT3(-width / 2, 0.0f, depth / 2);
        vertices.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
        vertices.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
        vertices.texcoords[vIndex++] = XMFLOAT2(0.0f, 0.0f);

        vertices.vertices[vIndex] = XMFLOAT3(width / 2, 0.0f, depth / 2);
        vertices.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
        vertices.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
        vertices.texcoords[vIndex++] = XMFLOAT2(texU, 0.0f);

        vertices.vertices[vIndex] = XMFLOAT3(width / 2, 0.0f, -depth / 2);
        vertices.normals[vIndex] = XMFLOAT3(0.0f, 1.0f, 0.0f);
        vertices.tangents[vIndex] = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
        vertices.texcoords[vIndex++] = XMFLOAT2(texU, texV);

        vertices.indices16 = { 0, 1, 2, 2, 3, 0 };

        return vertices;
    }

    GeometryData CreateGrid(const DirectX::XMFLOAT2& gridSize, const DirectX::XMUINT2& slices, const DirectX::XMFLOAT2& maxTexCoord,
        const std::function<float(float, float)>& heightFunc,
        const std::function<DirectX::XMFLOAT3(float, float)>& normalFunc,
        const std::function<DirectX::XMFLOAT4(float, float)>& colorFunc)
    {
        using namespace DirectX;

        GeometryData vertices;
        uint32_t vertexCount = (slices.x + 1) * (slices.y + 1);
        uint32_t indexCount = 6 * slices.x * slices.y;
        vertices.vertices.resize(vertexCount);
        vertices.normals.resize(vertexCount);
        vertices.tangents.resize(vertexCount);
        vertices.texcoords.resize(vertexCount);
        if (indexCount > 65535)
            vertices.indices32.resize(indexCount);
        else
            vertices.indices16.resize(indexCount);

        uint32_t vIndex = 0;
        uint32_t iIndex = 0;

        float sliceWidth = gridSize.x / slices.x;
        float sliceDepth = gridSize.y / slices.y;
        float leftBottomX = -gridSize.x / 2;
        float leftBottomZ = -gridSize.y / 2;
        float posX, posZ;
        float sliceTexWidth = maxTexCoord.x / slices.x;
        float sliceTexDepth = maxTexCoord.y / slices.y;

        XMFLOAT3 normal;
        XMFLOAT4 tangent;
        // �������񶥵�
        //  __ __
        // | /| /|
        // |/_|/_|
        // | /| /| 
        // |/_|/_|
        for (uint32_t z = 0; z <= slices.y; ++z)
        {
            posZ = leftBottomZ + z * sliceDepth;
            for (uint32_t x = 0; x <= slices.x; ++x)
            {
                posX = leftBottomX + x * sliceWidth;
                // ���㷨��������һ��
                normal = normalFunc(posX, posZ);
                XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&normal)));
                // ���㷨ƽ����z=posZƽ�湹�ɵ�ֱ�ߵ�λ��������ά��w����Ϊ1.0f
                XMStoreFloat4(&tangent, XMVector3Normalize(XMVectorSet(normal.y, -normal.x, 0.0f, 0.0f)) + g_XMIdentityR3);

                vertices.vertices[vIndex] = XMFLOAT3(posX, heightFunc(posX, posZ), posZ);
                vertices.normals[vIndex] = normal;
                vertices.tangents[vIndex] = tangent;
                vertices.texcoords[vIndex++] = XMFLOAT2(x * sliceTexWidth, maxTexCoord.y - z * sliceTexDepth);
            }
        }
        // ��������
        for (uint32_t i = 0; i < slices.y; ++i)
        {
            for (uint32_t j = 0; j < slices.x; ++j)
            {
                if (indexCount > 65535)
                {
                    vertices.indices32[iIndex++] = i * (slices.x + 1) + j;
                    vertices.indices32[iIndex++] = (i + 1) * (slices.x + 1) + j;
                    vertices.indices32[iIndex++] = (i + 1) * (slices.x + 1) + j + 1;

                    vertices.indices32[iIndex++] = (i + 1) * (slices.x + 1) + j + 1;
                    vertices.indices32[iIndex++] = i * (slices.x + 1) + j + 1;
                    vertices.indices32[iIndex++] = i * (slices.x + 1) + j;
                }
                else
                {
                    vertices.indices16[iIndex++] = i * (slices.x + 1) + j;
                    vertices.indices16[iIndex++] = (i + 1) * (slices.x + 1) + j;
                    vertices.indices16[iIndex++] = (i + 1) * (slices.x + 1) + j + 1;

                    vertices.indices16[iIndex++] = (i + 1) * (slices.x + 1) + j + 1;
                    vertices.indices16[iIndex++] = i * (slices.x + 1) + j + 1;
                    vertices.indices16[iIndex++] = i * (slices.x + 1) + j;
                }

            }
        }

        return vertices;
    }

}
