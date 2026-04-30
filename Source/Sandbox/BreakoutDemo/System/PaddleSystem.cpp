#include "PaddleSystem.h"
#include "../Bamboo/Core/Log.h"
#include "../Bamboo/Core/Input.h"
#include "../Bamboo/Core/KeyCodes.h"
#include "../Bamboo/ECS/Component/TransformComponent.h"
#include "../Component/PaddleComponent.h"

void PaddleSystem::Update(entt::registry &registry, float deltaTime)
{

    auto view = registry.view<Bamboo::TransformComponent, PaddleComponent>();
    for (auto entity : view)
    {
        auto &transform = view.get<Bamboo::TransformComponent>(entity);

        if (Bamboo::Input::IsKeyPressed(Bamboo::Key::A))
        {
            transform.Position.x -= 100 * deltaTime;
        }
        else if (Bamboo::Input::IsKeyPressed(Bamboo::Key::D))
        {
            transform.Position.x += 100 * deltaTime;
        }
    }
}