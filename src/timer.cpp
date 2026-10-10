/// @file timer.cpp
#include "timer.h"
#include "strings.h"
#include "icon_provider.h"
#include "led.h"
#include "preferences_manager.h"
#include "esp_log.h"

/// Log tag for this file, used by ESP_LOGx() calls. src/states/*.cpp each define their own copy
/// of this same "TIMER" tag, since it's a per-translation-unit static.
[[maybe_unused]] static const char *TAG = "TIMER";

extern Preferences preferences;

Preset::Preset(Icon *icon, const unsigned char *background, const char *name, unsigned long duration,
               unsigned long pauseDuration, unsigned long longPauseDuration, unsigned int longPauseAfter) {
    this->icon = icon;
    this->background = background;
    this->name = name;
    this->duration = duration;
    this->pauseDuration = pauseDuration;
    this->longPauseAfter = longPauseAfter;
    this->longPauseDuration = longPauseDuration;
}

Preset::~Preset() {}

Icon *Preset::getIcon() { return icon; }

const unsigned char *Preset::getBackground() { return background; }

unsigned long Preset::getDuration() {
#ifdef DEBUG
    return duration / 60;
#endif
    return duration;
}

unsigned long Preset::getPauseDuration() {
#ifdef DEBUG
    return pauseDuration / 60;
#endif
    return pauseDuration;
}

const char *Preset::getName() { return name; }

unsigned long Preset::getLongPauseDuration() {
#ifdef DEBUG
    return longPauseDuration / 60;
#endif
    return longPauseDuration;
}

unsigned int Preset::getLongPauseAfter() { return longPauseAfter; }

Timer::Timer(DISPLAY_CLASS &display) : display(display) {
    this->state = TimerState::SelectingPreset;
    this->currentPreset = nullptr;
    this->presetIndex = 0;
    this->needsRedraw = true;
    this->lastEncoderCount = 0;
    this->lastRedrawTime = 0;
    this->pauseStartTime = 0;
    this->totalPausedTime = 0;
    this->startTime = 0;
    this->elapsed = 0;
    this->menuNeedsRedraw = false;
    this->flashingIcon = false;
    this->breakImage = nullptr;
    this->needsFullRedraw = true;
    this->lastPauseRedrawTime = 0;
    this->lastPauseState = false;
    this->lastMessageUpdate = 0;
    this->lastActivityTime = millis();
    this->blankEncoderSnapshot = 0;

    MenuItem *items = new MenuItem[3]{MenuItem(messageCache.getMessage(Messages::MenuItem_Pause)),
                                      MenuItem(messageCache.getMessage(Messages::MenuItem_BreakNow)),
                                      MenuItem(messageCache.getMessage(Messages::MenuItem_Cancel))};
    this->topMenu = new Menu(display, items, 3);
    this->topMenu->setSelectedIndex(1);
    lastEncoderCount = 0;

    MenuItem *confirmationItems = new MenuItem[2]{MenuItem(messageCache.getMessage(Messages::MenuItem_StartBreak)),
                                                  MenuItem(messageCache.getMessage(Messages::MenuItem_BackToPresets))};
    this->confirmationMenu = new Menu(display, confirmationItems, 2);
    this->confirmationMenu->setEncoderCount(0);  // Initialize encoder count
}

Timer::~Timer() {
    if (topMenu) {
        delete[] topMenu->getItems();
        delete topMenu;
    }
}

void Timer::addPreset(Icon *icon, const unsigned char *background, const char *name, unsigned long duration,
                      unsigned long pauseDuration, unsigned long longPauseDuration, unsigned int longPauseAfter) {
    presets.push_back(Preset(icon, background, name, duration, pauseDuration, longPauseDuration, longPauseAfter));
}

void Timer::selectPreset(int index) {
    currentPreset = &presets[index];
    presetIndex = index;
}

void Timer::nextPreset() {
    presetIndex++;
    if (presetIndex >= presets.size()) {
        presetIndex = 0;
    }

    currentPreset = &presets[presetIndex];
}

void Timer::previousPreset() {
    if (presetIndex == 0) {
        presetIndex = presets.size() - 1;
    } else {
        presetIndex--;
    }

    currentPreset = &presets[presetIndex];
}

void Timer::enterPresetSelection() {
    state = TimerState::SelectingPreset;
    // Keep the last-used preset selected (index 0 on first entry after boot).
    selectPreset(presetIndex < presets.size() ? presetIndex : 0);
    needsRedraw = true;
    lastActivityTime = millis();
}

void Timer::reset() {
    setLedMode(LedMode::Off);
    redrawInterval = REDRAW_INTERVAL_DEFAULT;
    startTime = millis();
    elapsed = 0;
    pauseStartTime = 0;
    totalPausedTime = 0;
    Button::instance->checkAndClearButtonPress();
    topMenu->getItems()[0].setText(messageCache.getMessage(Messages::MenuItem_Pause));
    topMenu->getItems()[1].setText(messageCache.getMessage(Messages::MenuItem_BreakNow));
    topMenu->setSelectedIndex(1);
}

void Timer::blankDisplay(volatile const int *encoderCount) {
    ESP_LOGI(TAG, "blankDisplay: idle timeout reached, blanking display");

    // E-paper is bistable, so cutting power alone would leave the last image visible. A single
    // full refresh can also leave ghosting from prior partial updates, so flash black then white
    // to properly clear the panel before hibernating.
    display.fillScreen(GxEPD_BLACK);
    display.display(false);
    display.fillScreen(GxEPD_WHITE);
    display.display(false);
    display.hibernate();

    blankEncoderSnapshot = *encoderCount;
    displayBlanked = true;
    needsRedraw = false;
    needsFullRedraw = false;
}

bool Timer::wakeDisplayIfTriggered(volatile const int *encoderCount) {
    const bool buttonPressed = Button::instance->checkAndClearButtonPress();
    const bool encoderMoved = *encoderCount != blankEncoderSnapshot;

    if (!buttonPressed && !encoderMoved) {
        return false;
    }

    ESP_LOGI(TAG, "wakeDisplayIfTriggered: waking display from idle blank");
    wakeDisplay();
    return true;
}

void Timer::wakeDisplay() {
    // hibernate() requires a hardware reset to recover from; init() drives that reset.
    display.init(115200, true, 2, false);
    display.setRotation(0);

    displayBlanked = false;
    lastActivityTime = millis();
    needsFullRedraw = true;
}

void Timer::start() {
    if (currentPreset != nullptr) {
        showSpeechBubble = pref_getCheckbox("msgs", true);

        // remember selected preset
        auto savedPresetIndex = this->presetIndex;

        reset();
        topMenu->setEncoderCount(lastEncoderCount);  // Sync encoder count

        ESP_LOGI(TAG, "start with preset %u", savedPresetIndex);
        state = TimerState::Running;
        selectPreset(savedPresetIndex);
    }
}

void Timer::pause() {
    if (state == TimerState::Running) {
        state = TimerState::UserInitiatedPause;
        pauseStartTime = millis();
        lastActivityTime = millis();  // start the PAUSE_BLANK_TIMEOUT countdown fresh
        setLedMode(LedMode::TimerPaused);
        topMenu->getItems()[0].setText(messageCache.getMessage(Messages::MenuItem_Resume));
        topMenu->setEncoderCount(lastEncoderCount);  // Sync encoder count
    } else if (state == TimerState::RunningBreak) {
        state = TimerState::UserInitiatedBreakPause;
        pauseStartTime = millis();
        lastActivityTime = millis();  // start the PAUSE_BLANK_TIMEOUT countdown fresh
        setLedMode(LedMode::TimerPaused);
        topMenu->getItems()[0].setText(messageCache.getMessage(Messages::MenuItem_Resume));
        topMenu->setEncoderCount(lastEncoderCount);  // Sync encoder count
    }
}

void Timer::resume() {
    if (state == TimerState::UserInitiatedPause) {
        totalPausedTime += millis() - pauseStartTime;
        state = TimerState::Running;
        setLedMode(LedMode::Off);
        topMenu->getItems()[0].setText(messageCache.getMessage(Messages::MenuItem_Pause));
        topMenu->setEncoderCount(lastEncoderCount);  // Sync encoder count
    } else if (state == TimerState::UserInitiatedBreakPause) {
        totalPausedTime += millis() - pauseStartTime;
        state = TimerState::RunningBreak;
        setLedMode(LedMode::Off);
        topMenu->getItems()[0].setText(messageCache.getMessage(Messages::MenuItem_Pause));
        topMenu->setEncoderCount(lastEncoderCount);  // Sync encoder count
    }
}

void Timer::startBreak() {
    cycles += 1;
    incrementTotalCycles();
    minutesWorked += elapsed / 1000 / 60;
    incrementTotalTime(elapsed);

    ESP_LOGD(TAG, "startBreak: cycles %u with long break after %u", cycles, currentPreset->getLongPauseAfter());
    if (cycles % (currentPreset->getLongPauseAfter()) == 0) {
        ESP_LOGI(TAG, "startBreak: long break");
        currentBreakDuration = currentPreset->getLongPauseDuration();
        isLongBreak = true;
        longestEarnedPauseInShortCycles = 0;

        // nullptr in the neutral set: the long break then uses the short break's statistics layout.
        breakImage = IconProvider::getInstance()->getLongBreakImage();

        if (breakImage) {
            // HACK: The display doesn't like drawing images in one go
            drawRunningBreak();
            display.display(true);
        }
    } else {
        ESP_LOGI(TAG, "startBreak: short break");
        currentBreakDuration = currentPreset->getPauseDuration();
        isLongBreak = false;
        needsRedraw = true;
    }

    state = TimerState::RunningBreak;
    reset();
    topMenu->getItems()[1].setText(messageCache.getMessage(Messages::MenuItem_SkipBreak));

    // Restore the last selected index
    topMenu->setSelectedIndex(1);

    needsRedraw = true;
}

void Timer::stop() {
    setLedMode(LedMode::Off);
    reset();

    enterPresetSelection();
}

void Timer::cancelWorkInterval() {
    incrementTotalTime(elapsed);
    minutesWorked += elapsed / 1000 / 60;
    stop();
    needsFullRedraw = true;
}

void Timer::cancelBreakInterval() {
    incrementTotalBreakTime(elapsed);
    minutesOnBreak += elapsed / 1000 / 60;
    stop();
    needsFullRedraw = true;
}

void Timer::confirmStartOfBreak() {
    messageCache.clearCache(Messages::TimerWaitingForConfirmationStartOfBreak_Header);
    startBreak();
    needsFullRedraw = true;
}

void Timer::confirmEndOfBreak(bool restartWork) {
    messageCache.clearCache(Messages::TimerWaitingForConfirmationEndOfBreak_Header);

    incrementTotalBreakTime(elapsed);
    minutesOnBreak += elapsed / 1000 / 60;

    if (restartWork) {
        start();
    } else {
        reset();
        enterPresetSelection();
    }
    needsFullRedraw = true;
}

bool Timer::handleCommand(TimerCommand command, int arg, volatile const int *encoderCount) {
    // Like any physical input, a remote command wakes a blanked display first.
    if (displayBlanked) {
        wakeDisplay();
    }

    bool accepted = false;

    switch (state) {
        case TimerState::SelectingPreset:
            if (command == TimerCommand::Select && arg >= 0 && (unsigned int) arg < presets.size()) {
                selectPreset(arg);
                needsRedraw = true;
                accepted = true;
            } else if (command == TimerCommand::Start) {
                start();
                topMenu->setEncoderCount(*encoderCount);
                needsFullRedraw = true;
                accepted = true;
            }
            break;

        case TimerState::Running:
        case TimerState::UserInitiatedPause:
            if (command == TimerCommand::Pause && state == TimerState::Running) {
                pause();
                needsRedraw = true;
                accepted = true;
            } else if (command == TimerCommand::Resume && state == TimerState::UserInitiatedPause) {
                resume();
                needsRedraw = true;
                accepted = true;
            } else if (command == TimerCommand::Restart) {
                // Count the time worked so far, like a cancel would, but keep the cycle count:
                // start() only resets per-interval timekeeping.
                incrementTotalTime(elapsed);
                minutesWorked += elapsed / 1000 / 60;
                start();
                topMenu->setEncoderCount(*encoderCount);
                needsFullRedraw = true;
                accepted = true;
            } else if (command == TimerCommand::Break) {
                startBreak();
                accepted = true;
            } else if (command == TimerCommand::Stop) {
                cancelWorkInterval();
                accepted = true;
            }
            break;

        case TimerState::RunningBreak:
        case TimerState::UserInitiatedBreakPause:
            if (command == TimerCommand::Pause && state == TimerState::RunningBreak) {
                pause();
                needsFullRedraw = true;
                accepted = true;
            } else if (command == TimerCommand::Resume && state == TimerState::UserInitiatedBreakPause) {
                resume();
                needsFullRedraw = true;
                accepted = true;
            } else if (command == TimerCommand::Stop) {
                cancelBreakInterval();
                accepted = true;
            }
            break;

        case TimerState::WaitingConfirmStartOfBreak:
            if (command == TimerCommand::Break) {
                confirmStartOfBreak();
                accepted = true;
            }
            break;

        case TimerState::WaitingConfirmEndOfBreak:
            if (command == TimerCommand::Start || command == TimerCommand::Restart) {
                confirmEndOfBreak(true);
                accepted = true;
            } else if (command == TimerCommand::Stop) {
                confirmEndOfBreak(false);
                accepted = true;
            }
            break;

        default:
            break;
    }

    if (accepted) {
        lastActivityTime = millis();
    } else {
        ESP_LOGW(TAG, "handleCommand: command %d not valid in state %d", (int) command, (int) state);
    }
    return accepted;
}

int Timer::drawMenuBar() {
    const unsigned int padding = 8;
    const unsigned int innerPadding = 4;

    // REMOVE if TESTING PASSES -> Bounds bounds = getBounds(display, currentPreset->getName(), &SUB_FONT);

#ifdef DEBUG
    const unsigned int iconSize = 48;
    display.drawRect(padding, padding, iconSize, iconSize, GxEPD_BLACK);
#endif

    // Draw the menu next to the category
    // const uint16_t menuX = padding + innerPadding + iconSize + innerPadding + bounds.w + innerPadding + 8;
    const uint16_t menuX = padding;
    const uint16_t menuY = padding;
    const uint16_t paddingBetweenBoxes = 10;
    // REMOVE if TESTING PASSES -> const uint16_t totalPaddingWidth = paddingBetweenBoxes * (topMenu->getItemCount() -
    // 1);
    const uint16_t rawMenuWidth = display.width() - menuX - padding;
    const uint16_t menuItemWidth =
        (rawMenuWidth - (topMenu->getItemCount() - 1) * paddingBetweenBoxes) / topMenu->getItemCount();
    const uint16_t menuHeight = innerPadding + 48;  // iconSize;

    // Draw the menu
    for (int i = 0; i < topMenu->getItemCount(); i++) {
        const bool selected = i == topMenu->getSelectedIndex();
        const auto foreground = GxEPD_BLACK;
        // REMOVE if TESTING PASSES -> const auto background = GxEPD_WHITE;

        const unsigned int xOffset = menuX + i * (menuItemWidth + paddingBetweenBoxes);

        auto item = topMenu->getItems()[i];

        if (selected) {
            display.fillRoundRect(xOffset, menuY, menuItemWidth, menuHeight, 10, GxEPD_WHITE);
            drawPatternInRoundedArea(display, xOffset, menuY, menuItemWidth, menuHeight, 10, Pattern::SparseDots);
        } else {
            display.fillRoundRect(xOffset, menuY, menuItemWidth, menuHeight, 10, GxEPD_WHITE);
        }
        display.drawRoundRect(xOffset, menuY, menuItemWidth, menuHeight, 10, GxEPD_BLACK);

        drawCenteredText(display, item.getText(), xOffset + menuItemWidth / 2, menuY + menuHeight / 2, &SUB_FONT,
                         foreground);

#ifdef DEBUG
        display.drawFastVLine(xOffset + menuItemWidth / 2, menuY, menuHeight, GxEPD_BLACK);
        display.drawFastHLine(xOffset, menuY + menuHeight / 2, menuItemWidth, GxEPD_BLACK);
        display.drawCircle(xOffset + menuItemWidth / 2, menuY + menuHeight / 2, 5, GxEPD_BLACK);
#endif
    }

    return menuY + menuHeight;
}

void Timer::loop(volatile const int *encoderCount) {
    [[maybe_unused]] auto loopStartTime =
        millis();  // only read by ESP_LOGD calls below, which compile out below Debug level

    switch (state) {
        case TimerState::SelectingPreset:
            handleSelectingPreset(encoderCount);
            break;
        case TimerState::UserInitiatedPause:
        case TimerState::Running:
            handleRunning(encoderCount);
            break;
        case TimerState::UserInitiatedBreakPause:
        case TimerState::RunningBreak:
            handleRunningBreak(encoderCount);
            break;
        case TimerState::WaitingConfirmEndOfBreak:
        case TimerState::WaitingConfirmStartOfBreak:
            handleWaitingForConfirmation(encoderCount);
            break;
        default:
            ESP_LOGE(TAG, "loop: unknown state %d", (int) state);
            break;
    }

    if (menuNeedsRedraw) {
        [[maybe_unused]] auto millisMenuStart = millis();  // only read by the ESP_LOGD call below
        drawMenuBar();
        display.displayWindow(0, 0, display.width(), 8 + 4 + 48 + 4);
        ESP_LOGD(TAG, "loop: menu update took %lu ms", millis() - millisMenuStart);

        menuNeedsRedraw = false;

        if (!needsRedraw && !needsFullRedraw) {
            ESP_LOGD(TAG, "loop: display update took %lu ms", millis() - loopStartTime);
        }
    }

    if (needsRedraw || needsFullRedraw) {
        ESP_LOGD(TAG, "loop: needs redraw with state %d", (int) state);

        display.firstPage();

        if (needsFullRedraw) {
            // display.clearScreen();
            display.fillScreen(GxEPD_WHITE);
        }

        switch (state) {
            case TimerState::SelectingPreset:
                ESP_LOGD(TAG, "loop: drawPresetSelection");
                drawPresetSelection();
                break;
            case TimerState::UserInitiatedPause:
            case TimerState::Running:
                ESP_LOGD(TAG, "loop: drawRunning");
                drawRunning();
                break;
            case TimerState::UserInitiatedBreakPause:
            case TimerState::RunningBreak:
                ESP_LOGD(TAG, "loop: drawRunningBreak");
                drawRunningBreak();
                break;
            case TimerState::WaitingConfirmEndOfBreak:
            case TimerState::WaitingConfirmStartOfBreak:
                ESP_LOGD(TAG, "loop: drawWaitingForConfirmation");
                drawWaitingForConfirmation();
                break;
            default:
                ESP_LOGE(TAG, "loop: unknown state %d", (int) state);
                break;
        }

        if (needsFullRedraw) {
            ESP_LOGD(TAG, "loop: full display update");
            display.display(false);
        } else {
            ESP_LOGD(TAG, "loop: partial display update");
            display.display(true);
        }

        menuNeedsRedraw = false;
        needsRedraw = false;
        needsFullRedraw = false;
        lastRedrawTime = millis();
        ESP_LOGD(TAG, "loop: display update took %lu ms", millis() - loopStartTime);
    }
}

TimerState Timer::getState() { return state; }

const char *Timer::getCurrentPresetName() { return currentPreset ? currentPreset->getName() : nullptr; }

unsigned long Timer::getLiveElapsed() {
    if (state == TimerState::Running || state == TimerState::RunningBreak) {
        return millis() - startTime - totalPausedTime;
    }
    return elapsed;
}

unsigned long Timer::getRemainingMillis() {
    const unsigned long liveElapsed = getLiveElapsed();
    switch (state) {
        case TimerState::Running:
        case TimerState::UserInitiatedPause:
            return (currentPreset && liveElapsed < currentPreset->getDuration())
                       ? currentPreset->getDuration() - liveElapsed
                       : 0;
        case TimerState::RunningBreak:
        case TimerState::UserInitiatedBreakPause:
            return liveElapsed < currentBreakDuration ? currentBreakDuration - liveElapsed : 0;
        default:
            return 0;
    }
}

unsigned long Timer::getIntervalMillis() {
    switch (state) {
        case TimerState::Running:
        case TimerState::UserInitiatedPause:
            return currentPreset ? currentPreset->getDuration() : 0;
        case TimerState::RunningBreak:
        case TimerState::UserInitiatedBreakPause:
            return currentBreakDuration;
        default:
            return 0;
    }
}

unsigned int Timer::getPresetIndex() { return presetIndex; }

unsigned int Timer::getPresetCount() { return presets.size(); }

Preset *Timer::getPreset(unsigned int index) { return index < presets.size() ? &presets[index] : nullptr; }

bool Timer::isOnLongBreak() { return isLongBreak; }

unsigned int Timer::getCycles() { return cycles; }
