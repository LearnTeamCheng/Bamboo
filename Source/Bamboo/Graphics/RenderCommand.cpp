#include "RenderCommand.h"

namespace Bamboo {

    Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();
}