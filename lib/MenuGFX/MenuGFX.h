#pragma once
#include <string>
#include <Adafruit_GFX.h>

#define ELEMENT_SPACING 4
#define FONT_HEIGHT 8
#define TITLE_HEIGHT (FONT_HEIGHT + 1)
#define SCROLLBAR_WIDTH 3
#define VALUE_OFFSET 5

enum ValueType {
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_STRING,
    VALUE_ENUM,
    VALUE_MENU
};

struct EnumOption {
    const char* label;
    float numericValue;  // could represent int/float meaningfully
};

struct ValueData{
    ValueType type;
    union {
        int i;
        float f;
        const char* s;
        struct Menu* submenu;
        const EnumOption* options;
    };
};

struct MenuValue {
    ValueData data;

    // Constraints for numeric types
    float minVal;
    float maxVal;
    float step;
    bool posSign; //Print with sign prefix

    // For enumerated types
    uint8_t optionCount;
    uint8_t currentOption;

    //For rendering purposes
    std::string prefix;
    std::string suffix;

    std::function<void(int)> onChange;
};

struct MenuItem {
    const char* name;
    MenuValue value;
    bool editable;
};

struct Menu {
    const char* title; //Title bar text
    MenuItem* items;
    uint16_t itemCount;
    uint16_t selectedItem{}; //Highlighted element
    bool isEditing{}; //Highlight the value of the selected element
    struct Menu* parent;

    int16_t scrollVal{};
    bool scrollBar{true};
    bool loopScroll{}; //Infinite scrolling effect. Loop from the last element back to the first
    bool maintainSelection{}; //Don't change the selection to the nearest in-view element when it scrolls out of view
    bool clampScroll{}; // Clamp the last element in the menu to the bottom of the screen. Keeps the screen filled with elements 

    uint16_t c; //Text colour (selction text background colour)
    uint16_t bg; //Text background colour (selction text colour)
    //uint16_t selc;
    //uint16_t selbg;

    /// @brief Scroll through the menu by a +/- amount of elements
    /// @param scrollDelta Amount of scrolling to apply in number of elements
    void scroll(int16_t scrollDelta);

    /// @brief Draw the menu to a display
    /// @param display Display to draw to
    void draw(Adafruit_GFX& display);

    /// @brief Used internally by the draw function
    /// @param display Display to draw to
    void drawScrollBar(Adafruit_GFX& display);

    /// @brief Returns the currently selected MenuItem
    MenuItem& getSelection();
};

inline MenuValue IntValue(int initial, float min, float max, float step = 1, bool posSign = false, const char* suffix = nullptr) {
    MenuValue mv{};
    mv.data.type = VALUE_INT;
    mv.data.i = initial;
    mv.minVal = min;
    mv.maxVal = max;
    mv.step   = step;
    mv.posSign = posSign;
    mv.suffix = suffix;
    return mv;
}

inline MenuValue EnumValue(const EnumOption* options, size_t count, int current = 0) {
    MenuValue mv{};
    mv.data.type = VALUE_ENUM;
    mv.data.options = options;
    mv.optionCount = count;
    mv.currentOption = current;
    return mv;
}

inline MenuValue SubmenuValue(const char* text = "->"){
    MenuValue mv{};
    mv.data.type = VALUE_MENU;
    mv.suffix = text;
    return mv;
}