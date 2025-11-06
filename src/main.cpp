#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "MenuGFX.h"
#include "Encoder.h"

#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

template<class T, size_t N>
constexpr size_t size(T (&)[N]) { return N; }

#define ENC_SW 4
Encoder enc(2, 3);

Menu menu {
    .title = "Options",
    .selectedItem = 1,
    .scrollVal = 1,
    .clampScroll = true,
    .c = SH110X_WHITE,
    .bg = SH110X_BLACK
};

EnumOption frequencyOptions[]{
    {.label = {"868MHz", }, .numericValue = 868},
    {.label = {"915MHz", }, .numericValue = 915}
};

EnumOption rfStatusOptions[]{
    {.label = {"On", }, .numericValue = 1},
    {.label = {"Off", }, .numericValue = 0}
};

MenuValue idValue{
    .type = VALUE_INT,
    .i = 0,
    .minVal = 0,
    .maxVal = MAXFLOAT,
    .step = 1,
};

MenuValue frequencyValue{
    .type = VALUE_ENUM,
    .options = frequencyOptions,
    .optionCount = size(frequencyOptions),
    .currentOption = 1
};

MenuValue rfStatusValue{
    .type = VALUE_ENUM,
    .options = rfStatusOptions,
    .optionCount = size(rfStatusOptions),
    .currentOption = 0
};

MenuValue powerValue{
    .type = VALUE_INT,
    .i = 13,
    .minVal = 2,
    .maxVal = 20,
    .step = 1,
    .posSign = true,
    .suffix = "dBm"
};

MenuValue displayBrightnessValue{
    .type = VALUE_INT,
    .i = 100,
    .minVal = 5,
    .maxVal = 100,
    .step = 5,
    .suffix = "%"
};

MenuValue subMenuValue{
    .type = VALUE_MENU,
    .submenu = nullptr,
    .suffix = "->",
};

MenuValue backMenuValue{
    .type = VALUE_MENU,
    .submenu = nullptr,
    .suffix = "<-",
};

MenuItem items[]{
    {.name = "Back", .value = backMenuValue},
    {.name = "RF ID", .value = idValue, .editable = true},
    {.name = "Frequency", .value = frequencyValue, .editable = true},
    {.name = "TX Power", .value = powerValue, .editable = true},
    {.name = "RF Status", .value = rfStatusValue, .editable = true},
    {.name = "Brightness", .value = displayBrightnessValue, .editable = true},
    {.name = "Brightness", .value = displayBrightnessValue, .editable = true},
    {.name = "More options", .value = subMenuValue}
};

uint32_t lastDebounceTime = 0;
uint32_t debounceDelay = 50; //millis
void encSwHandler() {
    if ((millis() - lastDebounceTime) > debounceDelay){

        if(menu.getSelection().editable){
            menu.isEditing = !menu.isEditing;
        }
        
        lastDebounceTime = millis();
    }
}

void setup()   {

    Serial.begin(9600);

    if (CrashReport) {
        Serial.print(CrashReport);
        delay(5000);
    }

    pinMode(ENC_SW, INPUT_PULLUP); 
    attachInterrupt(digitalPinToInterrupt(ENC_SW), encSwHandler, FALLING);
    
    menu.items = items;
    menu.itemCount = size(items);

    delay(250); // wait for the OLED to power up
    display.begin(i2c_Address, true); // Address 0x3C default
    //display.setContrast (0); // dim display
    display.clearDisplay();
    menu.draw(display);
    display.display();

    // while (true)
    // {
    //     // delay(1000);
    //     // menu.scroll(1);

    //     // display.clearDisplay();
    //     // menu.draw(display);
    //     // display.display();

    //     for (size_t i = 0; i < 6; i++)
    //     {
    //         delay(1000);
    //         menu.scroll(1);
    //         display.clearDisplay();
    //         menu.draw(display);
    //         display.display();
    //     }

    //     for (size_t i = 0; i < 6; i++)
    //     {
    //         delay(1000);
    //         menu.scroll(-1);

    //         display.clearDisplay();
    //         menu.draw(display);
    //         display.display();
    //     }
    // }
}

long encVal = 0;

void loop(){
    long newEncVal = round(enc.read() / 4.f);
    if(newEncVal != encVal){
        menu.scroll(newEncVal - encVal);

        
        
        encVal = newEncVal;
    }

    display.clearDisplay();
    menu.draw(display);
    display.display();
}