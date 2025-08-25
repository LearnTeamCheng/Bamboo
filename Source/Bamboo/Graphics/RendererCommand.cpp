#include "RendererCommand.h"

namespace Bamboo {

    Scope<RendererAPI> RendererCommand::s_RendererAPI = RendererAPI::Create();
}