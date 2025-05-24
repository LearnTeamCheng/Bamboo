#pragma once
#include "../Core/Ref.h"
namespace Bamboo {
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Initiaize() = 0;
		virtual void SwapBuffers() = 0;

		static Scope<GraphicsContext> Create(void* window);
	
	};
};

