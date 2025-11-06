#include "MenuGFX.h"

const uint16_t howManyElementsCanFitOnScreen(const Adafruit_GFX& display){
    return (display.height() - TITLE_HEIGHT - ELEMENT_SPACING) / (FONT_HEIGHT + ELEMENT_SPACING);
}

void printValue(Adafruit_GFX& display, const MenuValue& value, uint8_t rightOffset){

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

void Menu::drawScrollBar(Adafruit_GFX& display){
    int16_t halfWidth = SCROLLBAR_WIDTH / 2;
    int16_t middle = display.width() - halfWidth - 2;
    int16_t yStart = TITLE_HEIGHT + 2;
    int16_t length = display.height() - 2 - yStart;
    length -= length % 2 == 0 ? 0 : 1; //Ensure consistent handle/box spacing between odd/even element counts

    int16_t handleLength = length / (clampScroll ? itemCount - howManyElementsCanFitOnScreen(display) + 1 : itemCount); //These expressions hurt to understand
    int16_t adjustedScrollValue = clampScroll ? min(scrollVal, itemCount - howManyElementsCanFitOnScreen(display)) : scrollVal; //Further magic

    //Draw center line
    display.drawFastVLine(middle, yStart, length, c);
    display.drawFastHLine(middle - 1, yStart, SCROLLBAR_WIDTH, c);
    display.drawFastHLine(middle - 1, yStart + length - 1, SCROLLBAR_WIDTH, c);

    //Draw box
    display.drawRect(middle - halfWidth, yStart + adjustedScrollValue * handleLength + 1, SCROLLBAR_WIDTH, handleLength, c);
}

MenuItem &Menu::getSelection()
{
    return items[selectedItem];
}

void Menu::draw(Adafruit_GFX& display)
{
    display.setTextColor(c, bg);
    display.setCursor(0, 0);
    display.setTextSize(1);

    //Draw title
    display.println(title);

    //Draw title underscore
    int y = display.getCursorY() + 1;
    display.drawLine(0, y, display.width(), y, c);
    display.setTextSize(1);

    {
        int16_t newY = display.getCursorY() + ELEMENT_SPACING; 
        display.setCursor(0, newY);
    }

    //Draw menu
    size_t adjustedScrollValue = clampScroll ? min(scrollVal, itemCount - howManyElementsCanFitOnScreen(display)) : scrollVal;
    for (size_t i = adjustedScrollValue; i < itemCount; i++)
    {
        bool selected = selectedItem == i;
        bool editing = selected && isEditing && items[selectedItem].editable;

        if(editing){
            display.setTextColor(c, bg);
            display.write(items[i].name);
            display.setTextColor(bg, c);
            printValue(display, items[i].value, VALUE_OFFSET);
        }
        else if (selected){
            display.setTextColor(bg, c);
            display.write(items[i].name);
            display.setTextColor(c, bg);
            printValue(display, items[i].value, VALUE_OFFSET);
        }
        else{
            display.setTextColor(c, bg);
            display.write(items[i].name);
            printValue(display, items[i].value, VALUE_OFFSET);
        }
        display.println();

        //Adjust item spacing
        int16_t newY = display.getCursorY() + ELEMENT_SPACING; 
        display.setCursor(0, newY);
    }

    if(scrollBar){
        drawScrollBar(display);
    }
}

void Menu::scroll(int16_t scrollDelta)
{
    if(isEditing){
        auto& item = items[selectedItem];
        if(item.editable){
            auto& value = item.value;

            switch (item.value.type)
            {
            case VALUE_INT:
                value.i += scrollDelta * value.step;
                value.i = std::clamp(value.i, (int)value.minVal, (int)value.maxVal);
                break;
            case VALUE_FLOAT:
                value.f += scrollDelta * value.step;
                value.f = std::clamp(value.f, value.minVal, value.maxVal);
                break;
            case VALUE_STRING:
                //TODO: String editor
                isEditing = false; //Fallback
                Serial.println("String option activated. No action taken");
                return;
            case VALUE_ENUM:
                {
                    int16_t newOption = (int16_t)value.currentOption + scrollDelta;

                    if(loopScroll){
                        value.currentOption = std::clamp(newOption, (int16_t)0, (int16_t)(value.optionCount - 1));
                    }
                    else{
                        value.currentOption = std::clamp(newOption, (int16_t)0, (int16_t)(value.optionCount - 1));
                    }
                }
                break;
            case VALUE_MENU:
                isEditing = false; //Fallback
                Serial.println("Menu option activated. No action taken");
                return;
            default:
                break;
            }
        }
    }
    else{
        scrollVal += scrollDelta;

        if(loopScroll){
            scrollVal = scrollVal % itemCount; //Loop scrolling implementation using modulo
        }
        else{
            if(scrollVal >= itemCount){
                scrollVal = itemCount - 1;
            }
            else if (scrollVal < 0){
                scrollVal = 0;
            }
        }

        //TODO: This could be smarter and allow the cursor to "push the view", 
        //      e.g scrolling outside of the currently displayed items moves the view in that direction.

        //Recalculate selection to be on screen
        if(!maintainSelection){
            selectedItem = scrollVal;
        }
    }
}
