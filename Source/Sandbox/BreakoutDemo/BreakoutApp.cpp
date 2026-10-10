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
#if TEST_SPRITE
    for (int i = 0; i < 50; i++)
    {

        auto entity = GetSceneManager()->GetActiveScene()->CreateEntity();
        auto &sprite = entity.AddComponent<Bamboo::SpriteRendererComponent>();
        sprite.size = Bamboo::Vector2(100.0f, 50.0f);

        float r = Bamboo::Random::GlobalFloat(0, 1.0f);
        float g = Bamboo::Random::GlobalFloat(0, 1.0f);
        float b = Bamboo::Random::GlobalFloat(0, 1.0f);

        sprite.color = Bamboo::Color(r, g, b);

        auto &transform = entity.GetComponent<Bamboo::TransformComponent>();
        int row = i % 10;
        int low = i / 10;

        // 按 10 列铺开砖块。
        // 注意：这里的 -640 是因为当前相机把 1280×720 的视口映射成 x∈[-1280,1280]、y∈[-720,720]
        // （世界单位 = 0.5 像素，见 P1-6），属于硬编码的临时摆法。
        // 相机参数修好后应改成"以视口中心为原点"的写法。
        float y = sprite.size.y * low + low * 10;
        float x = -640 + sprite.size.x * 0.5f + 5 + sprite.size.x * row + row * 10;

        transform.position = Bamboo::Vector3(x, y, 0.0f);
    }
#endif

    // 创建球拍
    {
        GetSceneManager()->GetActiveScene()->AddSystem<PaddleSystem>();
        auto paddleEntity = GetSceneManager()->GetActiveScene()->CreateEntity();
        auto &sprite = paddleEntity.AddComponent<Bamboo::SpriteRendererComponent>();
        paddleEntity.AddComponent<PaddleComponent>();
        sprite.size = Bamboo::Vector2(200, 40.0f);
        sprite.color = Bamboo::Color::Red;

        auto &transform = paddleEntity.GetComponent<Bamboo::TransformComponent>();
        transform.position = Bamboo::Vector3(GetWindow().get()->GetWidth() / 2, -50.0f, 0.0f);
    }

    // 创建一个三角形
    {
        auto entity = GetSceneManager()->GetActiveScene()->CreateEntity();
        auto& triangle =  entity.AddComponent<Bamboo::QuadComponent>();
        triangle.color = Bamboo::Color::Blue;  // 设置三角形的颜色
        triangle.size = Bamboo::Vector2(100.0f, 100.0f);  // 设置三角形的大小

        auto &transform = entity.GetComponent<Bamboo::TransformComponent>();
    }
    GetSceneManager()->GetActiveScene()->AddSystem<BallSystem>();
}

BreakoutApp::~BreakoutApp() {}