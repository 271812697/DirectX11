#pragma once

#ifndef XUTIL_H
#define XUTIL_H

#include <DirectXMath.h>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include<d3dcommon.h>
#include"pch.h"

//
// 宏定义
//

#define LEN_AND_STR(STR) ((UINT)(sizeof(STR) - 1)), (STR)

#if (defined(DEBUG) || defined(_DEBUG)) && !defined(GRAPHICS_DEBUGGER_OBJECT_NAME)
#define GRAPHICS_DEBUGGER_OBJECT_NAME 1
#endif

//
// 设置调试对象名称
//

template<class IObject>
inline void SetDebugObjectName(IObject* pObject, std::string_view name)
{
    pObject->SetPrivateData(WKPDID_D3DDebugObjectName, (uint32_t)name.size(), name.data());
}

//
// 文本转换函数
//

// 以下不可删除
#pragma warning(push)
#pragma warning(disable: 28251)
extern "C" __declspec(dllimport) int __stdcall MultiByteToWideChar(unsigned int cp, unsigned long flags, const char* str, int cbmb, wchar_t* widestr, int cchwide);
extern "C" __declspec(dllimport) int __stdcall WideCharToMultiByte(unsigned int cp, unsigned long flags, const wchar_t* widestr, int cchwide, char* str, int cbmb, const char* defchar, int* used_default);
#pragma warning(pop)

inline std::wstring UTF8ToWString(std::string_view utf8str)
{
    if (utf8str.empty()) return std::wstring();
    int cbMultiByte = static_cast<int>(utf8str.size());
    int req = MultiByteToWideChar(65001, 0, utf8str.data(), cbMultiByte, nullptr, 0);
    std::wstring res(req, 0);
    MultiByteToWideChar(65001, 0, utf8str.data(), cbMultiByte, &res[0], req);
    return res;
}

inline std::string WStringToUTF8(std::wstring_view wstr)
{
    if (wstr.empty()) return std::string();
    int cbMultiByte = static_cast<int>(wstr.size());
    int req = WideCharToMultiByte(65001, 0, wstr.data(), cbMultiByte, nullptr, 0, nullptr, nullptr);
    std::string res(req, 0);
    WideCharToMultiByte(65001, 0, wstr.data(), cbMultiByte, &res[0], req, nullptr, nullptr);
    return res;
}

//
// 获取ID
//

using XID = size_t;
inline XID StringToID(std::string_view str)
{
    static std::hash<std::string_view> hash;
    return hash(str);
}

//
// 数学相关函数
//

namespace XMath
{
    // ------------------------------
    // InverseTranspose函数
    // ------------------------------
    inline DirectX::XMMATRIX XM_CALLCONV InverseTranspose(DirectX::FXMMATRIX M)
    {
        using namespace DirectX;

        // 世界矩阵的逆的转置仅针对法向量，我们也不需要世界矩阵的平移分量
        // 而且不去掉的话，后续再乘上观察矩阵之类的就会产生错误的变换结果
        XMMATRIX A = M;
        A.r[3] = g_XMIdentityR3;

        return XMMatrixTranspose(XMMatrixInverse(nullptr, A));
    }

    inline float Lerp(float a, float b, float t)
    {
        return (1.0f - t) * a + t * b;
    }
}
template<typename T, typename...Args>
std::shared_ptr<T> MakeRef(Args&&...args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}
template<typename T>
using asset_ref = std::shared_ptr<T>;

class RecesoureManager {
private:
    std::map<XID, std::type_index> registry;
    std::map<XID, asset_ref<void>> resources;
public:
    template<typename T>
    void Add(std::string_view str,const asset_ref<T>&val) {
        XID id = StringToID(str);
        if (registry.find(id) != registry.end()) {
            return;
        }
        registry.try_emplace(id, typeid(T));
        resources.insert_or_assign(id,std::static_pointer_cast<void>(val));
    }
    template<typename T>
    asset_ref<T> Get(std::string_view str) {
        XID id = StringToID(str);
        if (registry.find(id) == registry.end()) {
            return nullptr;
        }
        else if (registry.at(id) != std::type_index(typeid(T))) {
            return nullptr;
        }
        return std::static_pointer_cast<T>(resources.at(id));
    }
    void Del(std::string_view str) {
        XID key = StringToID(str);
        if (registry.find(key) != registry.end()) {
            registry.erase(key);
            resources.erase(key);
        }
    }
    void Clear() {
        registry.clear();
        resources.clear();
    }
};


#endif
