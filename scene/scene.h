#pragma once
#include <map>
#include <string>
#include "../core/log.h"
#include "../component/all.h"
#include "entity.h"
#include "resource.h"
using namespace component;
namespace scene {
    class Scene {
    private:
        entt::registry registry;
        std::map<entt::entity, std::string> directory;
        //friend class Renderer;

    protected:
        ResourceManager resource_manager;
        //std::map<GLuint, UBO> UBOs;  // indexed by uniform buffer's binding point
        //std::map<GLuint, FBO> FBOs;  // indexed by the order of creation

        //void AddUBO(GLuint shader_id);
        //void AddFBO(GLuint width, GLuint height);

        Entity CreateEntity(const std::string& name);
        void DestroyEntity(Entity entity);

    public:
        std::string title;
        explicit Scene(const std::string& title);
        virtual ~Scene();

        virtual void Init(void);
        virtual void OnSceneRender(void);
        virtual void OnImGuiRender(void);

    private:

    };
    namespace color {
        // some commonly used color presets
        static    DirectX::XMFLOAT3 white{ 1.0f,1.0f,1.0f };
        static   DirectX::XMFLOAT3 black{ 0.0f,0.0f,0.0f };
        static   DirectX::XMFLOAT3 red{ 1.0f, 0.0f, 0.0f };
        static   DirectX::XMFLOAT3 green{ 0.0f, 1.0f, 0.0f };
        static   DirectX::XMFLOAT3 lime{ 0.5f, 1.0f, 0.0f };
        static   DirectX::XMFLOAT3 blue{ 0.0f, 0.0f, 1.0f };
        static   DirectX::XMFLOAT3 cyan{ 0.0f, 1.0f, 1.0f };
        static   DirectX::XMFLOAT3 yellow{ 1.0f, 1.0f, 0.0f };
        static   DirectX::XMFLOAT3 orange{ 1.0f, 0.5f, 0.0f };
        static   DirectX::XMFLOAT3 purple{ 0.5f, 0.0f, 1.0f };
    }


}
