#include "MenuGFX.h"

const uint16_t howManyElementsCanFitOnScreen(Adafruit_GFX& display){
    return (display.height() - TITLE_HEIGHT - ELEMENT_SPACING) / (FONT_HEIGHT + ELEMENT_SPACING);
}

uint16_t getScrollBarSegments(Adafruit_GFX& display, Menu& menu){
    if(menu.clampScroll){
        return menu.itemCount - howManyElementsCanFitOnScreen(display);
    }
    else{
        return menu.itemCount;
    }
}

void printValue(Adafruit_GFX& display, MenuValue& value, uint8_t rightOffset){

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
    int startX = display.width() - w - rightOffset - 1; // -1 for padding
    
    // Draw the text
    int16_t currentY = display.getCursorY(); 
    display.setCursor(startX, currentY);
    display.print(s.c_str());
}

void drawScrollBar(Adafruit_GFX& display, Menu& menu, uint16_t c){
    int16_t halfWidth = SCROLLBAR_WIDTH / 2;
    int16_t middle = display.width() - halfWidth - 2;
    int16_t yStart = TITLE_HEIGHT + 2;
    int16_t length = display.height() - 2 - yStart;
    length -= length % 2 == 0 ? 0 : 1; //Ensure consistent handle/box spacing between odd/even element counts

    int16_t handleLength = length / menu.itemCount;

    //Draw center line
    display.drawFastVLine(middle, yStart, length, c);
    display.drawFastHLine(middle - 1, yStart, SCROLLBAR_WIDTH, c);
    display.drawFastHLine(middle - 1, yStart + length - 1, SCROLLBAR_WIDTH, c);

    //Draw box
    display.drawRect(middle - halfWidth, yStart + menu.scrollVal * handleLength + 1, SCROLLBAR_WIDTH, handleLength, c);
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

    int16_t newY = display.getCursorY() + ELEMENT_SPACING; 
    display.setCursor(0, newY);

    //Draw menu
    size_t drawStartIndex = menu.clampScroll ? min(menu.scrollVal, menu.itemCount - howManyElementsCanFitOnScreen(display)) : menu.scrollVal;
    for (size_t i = drawStartIndex; i < menu.itemCount; i++)
    {
        bool selected = menu.selectedItem == i;
        bool editing = selected && menu.editing && menu.items[menu.selectedItem].editable;

        if(editing){
            display.setTextColor(c, bg);
            display.write(menu.items[i].name);
            display.setTextColor(selc, selbg);
            printValue(display, menu.items[i].value, VALUE_OFFSET);
        }
        else if (selected){
            display.setTextColor(selc, selbg);
            display.write(menu.items[i].name);
            display.setTextColor(c, bg);
            printValue(display, menu.items[i].value, VALUE_OFFSET);
        }
        else{
            display.setTextColor(c, bg);
            display.write(menu.items[i].name);
            printValue(display, menu.items[i].value, VALUE_OFFSET);
        }
        display.println();

        //Adjust item spacing
        int16_t newY = display.getCursorY() + ELEMENT_SPACING; 
        display.setCursor(0, newY);
    }

    drawScrollBar(display, menu, c);
}

void Menu::scroll(int16_t scrollDelta)
{
    scrollVal += scrollDelta;

    if(scrollVal >= itemCount){
        if(loopScroll){
            while (scrollVal >= itemCount) //Should only loop once unless scrollDelta is massive (>itemCount)
            {
                scrollVal -= itemCount; 
            }
        }
        else{
            scrollVal = itemCount - 1; //Clamp value
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

    //Recalculate selection to be on screen
    if(!maintainSelection){
        selectedItem = scrollVal;
    }
}
