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

};
#endif // BUTTON_H