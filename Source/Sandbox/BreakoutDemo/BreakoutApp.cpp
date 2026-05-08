#include <random>

#include "BreakoutApp.h"
#include "../Bamboo/ECS/Entity.h"

// system headers
#include "System/BallSystem.h"
#include "System/PaddleSystem.h"

#include "../Bamboo/Math/Random.h"

#include "Component/PaddleComponent.h"



BreakoutApp::BreakoutApp(const std::string &appName) : Application(appName)
{
    for (int i = 0; i < 50; i++)
    {

        auto &entity = GetSceneManager()->GetActiveScene()->CreateEntity();
        auto &sprite = entity.AddComponent<Bamboo::SpriteRendererComponent>();
         sprite.Size = Bamboo::Vector2(100.0f, 50.0f);
      //  sprite.Size = camera.CurrentCamera.PixelSizeToWorldSize(Bamboo::Vector2(100.0f, 50.0f));

        float r = Bamboo::Random::GlobalFloat(0, 1.0f);
        float g = Bamboo::Random::GlobalFloat(0, 1.0f);
        float b = Bamboo::Random::GlobalFloat(0, 1.0f);

        sprite.SpriteColor = Bamboo::Color(r, g, b);

        auto &transform = entity.GetComponent<Bamboo::TransformComponent>();
        int row = i % 10;
        int low = i / 10;
        //float y = 600 - sprite.Size.y * low - low * 10;
        //float x = 100 + sprite.Size.x * row + row * 10;

        float y = sprite.Size.y * low + low*10;
        float x = -640  + sprite.Size.x *0.5f +5+sprite.Size.x * row + row * 10;

        // transform.Scale = { sprite.Size.x, sprite.Size.y,0.0f };


      //  auto pos = camera.CurrentCamera.ScreenToWorldPosition(Bamboo::Vector3(x, y, 0.0f));
         transform.Position = Bamboo::Vector3(x,y , 0.0f);
        // transform.Position = pos;
    }

    // 创建球拍
    {
        GetSceneManager()->GetActiveScene()->AddSystem<PaddleSystem>();
        auto &paddleEntity = GetSceneManager()->GetActiveScene()->CreateEntity();
        auto &sprite = paddleEntity.AddComponent<Bamboo::SpriteRendererComponent>();
        paddleEntity.AddComponent<PaddleComponent>();
        sprite.Size = Bamboo::Vector2(200, 40.0f);
        sprite.SpriteColor = Bamboo::Color::Red;

        auto &transform = paddleEntity.GetComponent<Bamboo::TransformComponent>();
        transform.Position = Bamboo::Vector3(GetWindow().get()->GetWidth() / 2, -50.0f, 0.0f);
    }
    GetSceneManager()->GetActiveScene()->AddSystem<BallSystem>();
}

BreakoutApp::~BreakoutApp() {}