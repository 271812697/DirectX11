#include"scene.h"
namespace scene {
	void Scene::AddFBO(UINT width, UINT height)
	{
		UINT key= FBOs.size();
		FBOs.try_emplace(key, width, height);  // construct FBO in-place
	}
	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity e = { name, registry.create(), &registry };

		// every entity has a transform and a tag component
		e.AddComponent<Transform>();
		
		directory.emplace(e.id, e.name);

		return e;
	}
	void Scene::DestroyEntity(Entity e)
	{
		CORE_TRACE("Destroying entity: {0}", e.name);
		directory.erase(e.id);
		registry.destroy(e.id);
	}
	Scene::Scene(const std::string& title) : title(title), directory() {
		this->resource_manager = ResourceManager();
	}
	Scene::~Scene()
	{
		registry.each([this](auto id) {
			CORE_TRACE("Destroying entity: {0}", directory.at(id));
			});

		registry.clear();
	}
	void Scene::Init(void)
	{
	}
	void Scene::OnSceneRender(void)
	{
	}
	void Scene::OnImGuiRender(void)
	{
	}
}