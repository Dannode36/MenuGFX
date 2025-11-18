#include <sstream>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "MenuGFX.h"
#include "Encoder.h"
#include <iomanip>

#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 'SSI', 10x10px
const unsigned char SSI [] PROGMEM = {
	0x92, 0x40, 0x12, 0x40, 0x24, 0x40, 0xc4, 0x80, 0x08, 0x80, 0x31, 0x00, 0xc1, 0x00, 0x06, 0x00, 
	0x18, 0x00, 0xe0, 0x00
};

template<class T, size_t N>
constexpr size_t size(T (&)[N]) { return N; }

#define ENC_SW 4
Encoder enc(2, 3);

Menu* activeMenu = nullptr;

enum Units : uint8_t{
    UNIT_METRIC = 0,
    UNIT_IMPERIAL = 1
};

struct Config{
    Units units = UNIT_METRIC;
} config;

EnumOption frequencyOptions[]{
    {.label = {"868MHz", }, .numericValue = 868},
    {.label = {"915MHz", }, .numericValue = 915}
};

EnumOption rfStatusOptions[]{
    {.label = {"On", }, .numericValue = 1},
    {.label = {"Off", }, .numericValue = 0}
};

EnumOption displayUnitsOptions[]{
    {.label = {"Metric"}, .numericValue = 0},
    {.label = {"Imperial"}, .numericValue = 1}
};

MenuValue idValue{
    .data = ValueData { 
        .type = VALUE_INT, 
        .i = 0
    },
    .minVal = 0,
    .maxVal = MAXFLOAT,
    .step = 1,
};

MenuValue frequencyValue{
    .data = ValueData { 
        .type = VALUE_ENUM, 
        .options = frequencyOptions
    },
    .optionCount = size(frequencyOptions),
    .currentOption = 1
};

MenuValue rfStatusValue{
    .data = ValueData { 
        .type = VALUE_ENUM, 
        .options = rfStatusOptions
    },
    .optionCount = size(rfStatusOptions),
    .currentOption = 0
};

MenuValue powerValue{
    .data = ValueData { 
        .type = VALUE_INT, 
        .i = 13
    },
    .minVal = 2,
    .maxVal = 20,
    .step = 1,
    .posSign = true,
    .suffix = "dBm"
};

MenuValue displayBrightnessValue{
    .data = ValueData { 
        .type = VALUE_INT, 
        .i = 100
    },
    .minVal = 5,
    .maxVal = 100,
    .step = 5,
    .suffix = "%"
};

MenuValue displayUnitsValue{
    .data = ValueData { 
        .type = VALUE_ENUM, 
        .options = displayUnitsOptions
    },
    .optionCount = size(displayUnitsOptions),
    .currentOption = 0
};

MenuValue subMenuValue{
    .data = ValueData { 
        .type = VALUE_MENU, 
        .submenu = nullptr
    },
    .suffix = "->",
};

MenuValue backMenuValue{
    .data = ValueData { 
        .type = VALUE_MENU, 
        .submenu = nullptr
    },
    .suffix = "<-",
};

MenuItem items[]{
    {.name = "Back", .value = backMenuValue},
    {.name = "RF ID", .value = idValue, .editable = true},
    {.name = "Frequency", .value = frequencyValue, .editable = true},
    {.name = "TX Power", .value = powerValue, .editable = true},
    {.name = "RF Status", .value = rfStatusValue, .editable = true},
    {.name = "Brightness", .value = displayBrightnessValue, .editable = true},
    {.name = "Units", .value = displayUnitsValue, .editable = true},
    {.name = "More options", .value = subMenuValue}
};

Menu menu {
    .title = "Options",
    .items = items,
    .itemCount = size(items),
    .selectedItem = 0,
    .scrollVal = 0,
    .clampScroll = true,
    .c = SH110X_WHITE,
    .bg = SH110X_BLACK
};

//Forward declarations
void drawMenu();
void encSwHandler();
void drawInterface();

void setup()   {

    Serial.begin(9600);

    if (CrashReport) {
        Serial.print(CrashReport);
        delay(5000);
    }

    //Setup encoder switch interrupt handler
    pinMode(ENC_SW, INPUT_PULLUP); 
    attachInterrupt(digitalPinToInterrupt(ENC_SW), encSwHandler, FALLING);

    delay(250); // wait for the OLED to power up
    display.begin(i2c_Address, true); // Address 0x3C default
    //display.setContrast (0); // dim display
    display.clearDisplay();
}

long encVal = 0;

void loop(){
    if(activeMenu != nullptr){
        long newEncVal = round(enc.read() / 4.f);
        if(newEncVal != encVal){
            long scrollDelta = newEncVal - encVal;

            if(menu.isEditing){
                auto& item = menu.items[menu.selectedItem];
                if(item.editable){
                    auto& value = item.value;
                    auto& data = item.value.data;

                    //Type handlers
                    switch (data.type)
                    {
                    case VALUE_INT:
                        data.i += scrollDelta * value.step;
                        data.i = std::clamp(data.i, (int)value.minVal, (int)value.maxVal);
                        break;
                    case VALUE_FLOAT:
                        data.f += scrollDelta * value.step;
                        data.f = std::clamp(data.f, value.minVal, value.maxVal);
                        break;
                    case VALUE_STRING:
                        //TODO: String editor
                        menu.isEditing = false; //Fallback
                        Serial.println("String option activated. No action taken");
                        return;
                    case VALUE_ENUM:
                        {
                            int16_t newOption = (int16_t)value.currentOption + scrollDelta;

                            if(menu.loopScroll){
                                value.currentOption = std::clamp(newOption, (int16_t)0, (int16_t)(value.optionCount - 1));
                            }
                            else{
                                value.currentOption = std::clamp(newOption, (int16_t)0, (int16_t)(value.optionCount - 1));
                            }
                        }
                        break;
                    case VALUE_MENU:
                        menu.isEditing = false; //Fallback
                        Serial.println("Menu option activated. No action taken");
                        return;
                    default:
                        break;
                    }
                }
            }
            else{
                menu.scroll(scrollDelta);
            }
            
            encVal = newEncVal;
            drawMenu();
        }
    }
    else{
        drawInterface();
    }
}

void drawBitmapVFlipped(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, int16_t cw, int16_t ch, uint16_t color) {

    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t b = 0;

    display.startWrite();
    for (int16_t j = 0; j < ch; j++, y--) {
        for (int16_t i = 0; i < cw; i++) {
        if (i & 7)
            b <<= 1;
        else
            b = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
        if (b & 0x80)
            display.writePixel(x + i, y + h, color);
        }
    }
    display.endWrite();
}

void drawInterface(){
    display.clearDisplay();

    //Draw signal strength indicator
    int8_t signalStengthIndex = std::min(3, 3);
    
    if(signalStengthIndex == -1){
        display.drawLine(1, 1, 10, 10, SH110X_WHITE);
        display.drawLine(1, 10, 10, 1, SH110X_WHITE);
    }
    else{
        int16_t length = 3 * signalStengthIndex + 1;
        drawBitmapVFlipped(1, 1, SSI, 10, 10, length, length, SH110X_WHITE);
    }

    //Draw battery level indicator
    uint8_t quantizedBattLevel = 1;
    display.drawRoundRect(103, 3, 23, 9, 1, SH110X_WHITE);
    display.drawFastVLine(126, 6, 3, SH110X_WHITE);

    for (size_t i = 0; i < quantizedBattLevel; i++)
    {
        display.fillRect(105 + (4 * i), 5, 3, 5, SH110X_WHITE);
    }

    //Draw toolbar divider line
    display.drawFastHLine(0, 15, display.width(), SH110X_WHITE);

    //Draw speed value
    float speed = 5;
    speed = speed * (config.units == UNIT_METRIC ? 3.6f :  2.237f); //Convert from m/s to configured unit
    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << speed;
    std::string speedStr = stream.str();

    display.setTextSize(4); //24x32
    display.setTextColor(SH110X_WHITE);

    int16_t speed_x1;
    int16_t speed_y1;
    uint16_t speed_w;
    uint16_t speed_h;
    display.getTextBounds(speedStr.c_str(), 0, 0, &speed_x1, &speed_y1, &speed_w, &speed_h);
    display.setCursor((display.width() - speed_w) / 2, (display.height() - speed_h) / 2 + 6);
    display.print(speedStr.c_str());

    //Draw speed units
    display.setTextSize(1);
    int16_t unit_x1;
    int16_t unit_y1;
    uint16_t unit_w;
    uint16_t unit_h;
    std::string units = config.units == UNIT_METRIC ? "km/h" : "mph";
    display.getTextBounds(units.c_str(), 0, 0, &unit_x1, &unit_y1, &unit_w, &unit_h);

    display.setCursor((display.width() - unit_w) / 2, display.getCursorY() + speed_h);
    display.print(config.units == UNIT_METRIC ? "km/h" : "mph");

    display.display();
}

void drawMenu(){
    if(activeMenu == nullptr){
        Serial.println("ERROR: Called drawMenu() while activeMenu was a nullptr");
        return;
    }

    display.clearDisplay();
    activeMenu->draw(display);
    display.display();
}

uint32_t lastDebounceTime = 0;
uint32_t debounceDelay = 50; //millis
void encSwHandler(){
    if ((millis() - lastDebounceTime) > debounceDelay){

        if(menu.getSelection().editable){
            menu.isEditing = !menu.isEditing;
        }
        drawMenu();
        
        lastDebounceTime = millis();
    }
}
