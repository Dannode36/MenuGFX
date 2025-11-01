#pragma once
#include <string>
#include <Adafruit_GFX.h>

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
    uint8_t itemCount;
    uint8_t selectedIndex;
    struct Menu* parent;
    uint16_t currentItem{};
    bool editing{};

    uint16_t scrollVal{};
    bool loopScroll{};

    void scroll(uint16_t scrollDelta);
};

void drawMenu(Adafruit_GFX& display, Menu& menu, uint16_t c, uint16_t bg, uint16_t selc, uint16_t selbg);