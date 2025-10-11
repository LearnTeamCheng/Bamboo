#include "../Bamboo/Core/Input.h"
#include "../Bamboo/Game/Application.h"
#include <GLFW/glfw3.h>
namespace Bamboo
{
    bool Input::IsKeyPressed(KeyCode key)
    {
        auto * window = static_cast<GLFWwindow*> (Application::GetInstance().GetWindow().GetNativeWindow());
        auto state = glfwGetKey(window, key);
        return state == GLFW_PRESS;
    }


    Vector2 Input::GetMousePosition()
    {
        auto * window = static_cast<GLFWwindow*> (Application::GetInstance().GetWindow().GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return Vector2((float)xpos, (float)ypos);
    }

}