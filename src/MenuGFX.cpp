#include "MenuGFX.h"

void printValue(Adafruit_GFX& display, MenuValue& value){

    std::string s;
    switch (value.type)
    {
    case VALUE_INT:
        s += value.prefix;
        if (value.posSign && value.i > 0){
            s += '+';
        }
        s += std::to_string(value.i) + value.suffix;
        break;
    case VALUE_FLOAT:
        s += value.prefix;
        if (value.posSign && value.f > 0){
            s += '+';
        }
        s += std::to_string(value.f) + value.suffix;
        break;
    case VALUE_STRING:
        s = value.s;
        break;
    case VALUE_ENUM:
        s = value.options[value.currentOption].label;
        break;
    case VALUE_MENU:
        s = value.prefix + value.suffix;
        break;
    default:
        break;
    }

    int16_t x, y;
    uint16_t w, h;
    display.getTextBounds(s.c_str(), 0, 0, &x, &y, &w, &h);

    // Compute start X so the text ends at the right edge
    int startX = display.width() - w - 1; // -1 for padding
    
    // Draw the text
    int16_t currentY = display.getCursorY(); 
    display.setCursor(startX, currentY);
    display.print(s.c_str());
}



void drawMenu(Adafruit_GFX& display, Menu& menu, uint16_t c, uint16_t bg, uint16_t selc, uint16_t selbg)
{
    display.setTextColor(c, bg);
    display.setCursor(0, 0);
    display.setTextSize(1);

    //Draw title
    display.println(menu.title);

    //Draw title underscore
    int y = display.getCursorY() + 1;
    display.drawLine(0, y, display.width(), y, c);
    display.setTextSize(1);

    int16_t newY = display.getCursorY() + 4; 
    display.setCursor(0, newY);

    //Draw menu
    for (size_t i = menu.scrollVal; i < menu.itemCount; i++)
    {
        bool selected = menu.currentItem == i;
        bool editing = selected && menu.editing && menu.items[menu.currentItem].editable;

        if(editing){
            display.setTextColor(c, bg);
            display.write(menu.items[i].name);
            display.setTextColor(selc, selbg);
            printValue(display, menu.items[i].value);
        }
        else if (selected){
            display.setTextColor(selc, selbg);
            display.write(menu.items[i].name);
            display.setTextColor(c, bg);
            printValue(display, menu.items[i].value);
        }
        else{
            display.setTextColor(c, bg);
            display.write(menu.items[i].name);
            printValue(display, menu.items[i].value);
        }
        display.println();

        //Adjust item spacing
        int16_t newY = display.getCursorY() + 4; 
        display.setCursor(0, newY);
    }
}

void Menu::scroll(uint16_t scrollDelta)
{
    scrollVal += scrollDelta;

    
    
    if(scrollVal > itemCount){
        if(loopScroll){
            while (scrollVal > itemCount) //Should only loop once unless scrollDelta is massive (>itemCount)
            {
                scrollVal -= itemCount; 
            }
        }
        else{
            scrollVal = itemCount; //Clamp value
        }
    }
    else if (scrollVal < 0){
        if(loopScroll){
            while (scrollVal < 0) //Should only loop once unless scrollDelta is massive (>itemCount)
            {
                scrollVal += itemCount; 
            }
        }
        else{
            scrollVal = 0; //Clamp value
        }
    }
}
