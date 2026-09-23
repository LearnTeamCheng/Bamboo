#include "PaddleSystem.h"
#include "../Bamboo/Core/Log.h"
#include "../Bamboo/Core/Input.h"
#include "../Bamboo/Core/KeyCodes.h"
#include "../Bamboo/ECS/Component/TransformComponent.h"
#include "../Component/PaddleComponent.h"

#include "../Bamboo/ECS/SystemContext.h"
#include "../Bamboo/ECS/World.h"



void PaddleSystem::Update(Bamboo::SystemContext &context, float deltaTime)
{
    auto view = context.world.GetRegistry().view<Bamboo::TransformComponent, PaddleComponent>();
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