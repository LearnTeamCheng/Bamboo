#include <random>

#include "BreakoutApp.h"
#include "../Bamboo/ECS/Entity.h"

//system headers
#include "System/BallSystem.h"
#include "System/PaddleSystem.h"

#include"../Bamboo/Math/Random.h"

#include "Component/PaddleComponent.h"


BreakoutApp::BreakoutApp(const std::string &appName) : Application(appName)
{

    //// 创建一个随机设备来生成种子
    //std::random_device rd;
    //// 使用种子初始化随机数生成器
    //std::mt19937 gen(rd());
    //// 定义范围，比如生成0到99之间的随机数
    //std::uniform_int_distribution<> dis(0, 255);
    // 生成随机数

    for (int i = 0; i < 50; i++)
    {

        auto &entity = GetSceneManager()->GetActiveScene()->CreateEntity();
        auto &sprite = entity.AddComponent<Bamboo::SpriteRendererComponent>();

        sprite.Size = Bamboo::Vector2(100.0f, 50.0f);


        float r =   Bamboo::Random::GlobalFloat(0, 1.0f);
        float g = Bamboo::Random::GlobalFloat(0, 1.0f);
        float b =  Bamboo::Random::GlobalFloat(0, 1.0f);

        sprite.SpriteColor = Bamboo::Color(r, g, b);

        auto &transform = entity.GetComponent<Bamboo::TransformComponent>();
        int row = i % 10;
        int low = i / 10;
        float y = 600 - sprite.Size.y * low - low * 10;
        float x = 100 + sprite.Size.x * row + row * 10;

        transform.Position = Bamboo::Vector3(x, y, 0);
    }

    //创建球拍
    {
        GetSceneManager()->GetActiveScene()->AddSystem<PaddleSystem>();
        auto &paddleEntity = GetSceneManager()->GetActiveScene()->CreateEntity();
        auto& sprite = paddleEntity.AddComponent<Bamboo::SpriteRendererComponent>();
        paddleEntity.AddComponent<PaddleComponent>();
        sprite.Size = Bamboo::Vector2(200, 40.0f);
        sprite.SpriteColor = Bamboo::Color::Red;

        auto& transform = paddleEntity.GetComponent<Bamboo::TransformComponent>();
        transform.Position = Bamboo::Vector3(GetWindow().get()->GetWidth() / 2 ,50.0f, 0.0f);
    }
    //GetSceneManager()->GetActiveScene()->AddSystem<BallSystem>();
}

BreakoutApp::~BreakoutApp() {}