#include "BreakoutApp.h"
#include "../Bamboo/ECS/Entity.h"

#include <random>

BreakoutApp::BreakoutApp(const std::string &appName) : Application(appName)
{

    // 创建一个随机设备来生成种子
    std::random_device rd;
    // 使用种子初始化随机数生成器
    std::mt19937 gen(rd());
    // 定义范围，比如生成0到99之间的随机数
    std::uniform_int_distribution<> dis(0, 255);
    // 生成随机数

    for (int i = 0; i < 50; i++)
    {

        auto &entity = GetSceneManager()->GetActiveScene()->CreateEntity();
        auto &sprite = entity.AddComponent<Bamboo::SpriteRendererComponent>();

        sprite.Size = Bamboo::Vector2(100.0f, 50.0f);

        float r = dis(gen) / 255.0f;
        float g = dis(gen) / 255.0f;
        float b = dis(gen) / 255.0f;

        sprite.SpriteColor = Bamboo::Color(r, g, b);

        auto &transform = entity.GetComponent<Bamboo::TransformComponent>();
        int row = i % 10;
        int low = i / 10;
        float y = 600 - sprite.Size.y * low - low * 10;
        float x = 100 + sprite.Size.x * row + row * 10;

        transform.Position = Bamboo::Vector3(x, y, 0);
    }
}

BreakoutApp::~BreakoutApp() {}