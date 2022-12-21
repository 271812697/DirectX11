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



}
