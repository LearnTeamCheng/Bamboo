#include "GraphicsContext.h"
#include "RendererAPI.h"
#include "../Bamboo/GraphicsAPI/OpenGL/OpenGLContext.h"

namespace Bamboo {
	Scope<GraphicsContext> GraphicsContext::Create(void* window) {
		return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
	}
}