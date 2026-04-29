#pragma once
#include "../Bamboo/UI/UIElement.h"

#include "../Bamboo/Math/Color.h"
#include <string>
namespace Bamboo::UI
{
    class Text : public UIElement
    {
    public:
        enum class TextAlign
        {
            Left,
            Center,
            Right
        };


    public:
        void SetText(const std::string &text) { m_Text = text; }
        const std::string &GetText() const { return m_Text; }

        void SetFont(const std::string &font) { m_Font = font; }
        const std::string &GetFont() const { return m_Font; }

        void SetFontSize(int size) { m_FontSize = size; }
        int GetFontSize() const { return m_FontSize; }

        void SetColor(const Color &color) { m_Color = color; }
        const Color &GetColor() const { return m_Color; }

        void SetAlign(TextAlign align) { m_Align = align; }
        TextAlign GetAlign() const { return m_Align; }
        /// @brief 设置是否加粗
        void SetBold(bool bold) { m_Bold = bold; }
        bool IsBold() const { return m_Bold; }
        virtual void OnRender() override;

    private:
        bool m_Bold = false;
        std::string m_Font;
        int m_FontSize = 16;
        Color m_Color = Color::White;
        TextAlign m_Align = TextAlign::Left;
        std::string m_Text;
    };
}