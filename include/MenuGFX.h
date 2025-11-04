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

struct MenuValue {
    ValueType type;
    union {
        int i;
        float f;
        const char* s;
        struct Menu* submenu;
    };

    // Constraints for numeric types
    float minVal;
    float maxVal;
    float step;
    bool posSign; //Print with sign prefix

    // For enumerated types
    const EnumOption* options;
    uint8_t optionCount;
    uint8_t currentOption;

    std::string prefix;
    std::string suffix;
};

struct MenuItem {
    const char* name;
    MenuValue value;
    bool editable;

};
struct Menu {
    const char* title;
    MenuItem* items;
    uint16_t itemCount;
    uint16_t selectedItem{};
    bool editing{};
    struct Menu* parent;

    int16_t scrollVal{};
    bool drawScrollBar{true};
    bool loopScroll{};
    bool maintainSelection{};
    bool clampScroll{};

    /// @brief Scroll through the menu by a +/- amount of elements
    /// @param scrollDelta Amount of scrolling to apply in number of elements
    /// @param loopScroll Infinite scrolling effect. Loop from the last element back to the first
    /// @param maintainSelection Don't change the selection to the nearest in-view element when it scrolls out of view
    void scroll(int16_t scrollDelta);
};

void drawMenu(Adafruit_GFX& display, Menu& menu, uint16_t c, uint16_t bg, uint16_t selc, uint16_t selbg);