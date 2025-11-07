#pragma once
#include <vector>
#include "UIElement.h"
namespace Bamboo::UI
{
    class Canvas
    {
    public:
        enum class ClearFlag
        {
            COLOR = 1,
            DEPTH = 2,
            STENCIL = 4
        };

    public:
        Canvas();
        ~Canvas();

        void SetSize(int width, int height);
        void SetClearFlag(ClearFlag flag) { m_ClearFlag = flag; }
        
        void AddElement(const std::shared_ptr<UIElement>& element);
        void RemoveElement(const std::shared_ptr<UIElement>& element);
    private:
        std::vector<std::shared_ptr<UIElement>> m_Elements;
        int m_Width;
        int m_Height;
        ClearFlag m_ClearFlag;
    };
}