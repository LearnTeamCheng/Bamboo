#include "GraphicsContext.h"
namespace Bamboo {
	Scope<GraphicsContext> GraphicsContext::Create(void* window) {
		return CreateScope<GraphicsContext>();
	}
}