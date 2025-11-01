#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "MenuGFX.h"

#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

template<class T, size_t N>
constexpr size_t size(T (&)[N]) { return N; }

void setup()   {

  Serial.begin(9600);

  EnumOption frequencyOptions[]{
    {.label = {"868MHz", }, .numericValue = 868},
    {.label = {"915MHz", }, .numericValue = 915}
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

  EnumOption rfStatusOptions[]{
    {.label = {"On", }, .numericValue = 1},
    {.label = {"Off", }, .numericValue = 0}
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
    {.name = "More options", .value = subMenuValue}
  };

  Menu menu {
    .title = "Options",
    .items = items,
    .itemCount = size(items),
    .currentItem = 1,
    .scrollVal = 1
  };

  delay(250); // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
  //display.setContrast (0); // dim display
  display.clearDisplay();
  
  drawMenu(display, menu, SH110X_WHITE, SH110X_BLACK, SH110X_BLACK, SH110X_WHITE);
  display.display();

}

void loop() {

}