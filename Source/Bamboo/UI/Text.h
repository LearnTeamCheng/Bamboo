#include "../Bamboo/UI/UIElement.h"
#include <string>
namespace Bamboo::UI
{
    class Text : public UIElement
    {
    public:
        void SetText(const std::string &text);
        const std::string &GetText() const;

    private:
        std::string m_Text;
    };
}