#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_GFX.h>
#include "gfx_utils.h"
#include "icons.h"
#include "icon.h"
#include "timer.h"
#include "checkbox.h"
#include "debug.h"
#include "images.h"
#include <vector>

class SplashScreen {
   private:
    DISPLAY_CLASS &display;
    Timer &timer;

    Menu buttons = Menu(display, new MenuItem[2]{MenuItem("Start"), MenuItem(Msgs::SETTINGS)}, 2);

    std::vector<Checkbox> checkboxes = {
        Checkbox(&icon_lpehacker, Msgs::MODE, "lpe", true),
        Checkbox(&icon_lpenote, Msgs::MISCMSG, "msgs", true),
        Checkbox(nullptr, Msgs::RSTDEVICE, "reset"),
    };

    Checkbox *selectedCheckbox;
    int16_t selectedSettingsIndex = 0;
    int16_t lastEncoderCount = 0;

   public:
    SplashScreen(DISPLAY_CLASS &display, Timer &timer);
    ~SplashScreen();
    void setLastEncoderCount(int16_t count);
    void draw();
    void loop(volatile int *encoderCount);

    void drawSettings();
    void loopSettings(volatile int *encoderCount);
};

#endif
