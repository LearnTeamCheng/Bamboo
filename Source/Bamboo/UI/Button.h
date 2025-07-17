#ifndef BUTTON_H
#define BUTTON_H

#include <iostream>
#include <string>


#include "UI/UIElement.h"

class Button : public UIElement
{
public:
    Button(const std::string& text);
	~Button();

    void SetText(const std::string& text);
    std::string GetText() const;

    void AddClickListener();
    void RemoveClickListener();
private:
    string m_text;
    bool m_isClicked;
};
#endif // BUTTON_H