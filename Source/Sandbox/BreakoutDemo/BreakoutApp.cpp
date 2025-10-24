#include "BreakoutApp.h"
#include "../Bamboo/ECS/Entity.h"
//using namespace Bamboo;

//#include <chrono>

BreakoutApp::BreakoutApp(const std::string &appName) : Application(appName)
{
    
    //using namespace std::chrono_literals;

    for (int i = 0; i < 10; i++) {

       auto& entity = GetSceneManager()->GetActiveScene()->CreateEntity();
       auto & sprite =  entity.AddComponent<Bamboo::SpriteRendererComponent>();

       sprite.Size = Bamboo::Vector2(100.0f, 50.0f);

       sprite.SpriteColor = Bamboo::Color::Red;

        auto& transform = entity.GetComponent<Bamboo::TransformComponent>();
        float y = 400;
        float x = 100 + sprite.Size.x * i +i*10;

        transform.Position = Bamboo::Vector3(x, y, 0);
    }


    auto cameraentity = GetSceneManager()->GetActiveScene()->CreateEntity();

    //增加摄像机组件
    auto& cameraComponent = cameraentity.AddComponent<Bamboo::CameraComponent>();
    cameraComponent.CurrentCamera = Bamboo::Camera();
    cameraComponent.CurrentCamera.SetOrthographic(10, 1, 100.0f);
    cameraComponent.CurrentCamera.SetViewportSize(1280, 720);

}

BreakoutApp::~BreakoutApp() {}