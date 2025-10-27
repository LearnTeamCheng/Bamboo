#include "BreakoutApp.h"
#include "../Bamboo/ECS/Entity.h"


BreakoutApp::BreakoutApp(const std::string &appName) : Application(appName)
{
    
    for (int i = 0; i < 50; i++) {

       auto& entity = GetSceneManager()->GetActiveScene()->CreateEntity();
       auto & sprite =  entity.AddComponent<Bamboo::SpriteRendererComponent>();

       sprite.Size = Bamboo::Vector2(100.0f, 50.0f);

        auto& transform = entity.GetComponent<Bamboo::TransformComponent>();
        int row =i%10;
        int low = i/10;
        float y = 600 - sprite.Size.y * low - low*10;
        float x = 100 + sprite.Size.x * row +row*10;

        transform.Position = Bamboo::Vector3(x, y, 0);
    }

}

BreakoutApp::~BreakoutApp() {}