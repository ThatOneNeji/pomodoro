/**
 * @file timer.h
 * @brief The pomodoro state machine (Timer) and the per-preset durations it cycles through (Preset).
 */
#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>
#include "config.h"
#include <GxEPD2_BW.h>
#include <GxEPD2_GFX.h>
#include <vector>

#include "gfx_utils.h"
#include "icons.h"
#include "debug.h"
#include "menu.h"
#include "led.h"
#include "button.h"
#include "statistics.h"

/// A named work/break duration configuration (e.g. "Coding": 45min work, 15min break, long break every 2 cycles).
class Preset {
   private:
    Icon *icon;
    const unsigned char *background;
    const char *name;
    unsigned long duration;
    unsigned long pauseDuration;
    unsigned int longPauseAfter;
    unsigned long longPauseDuration;

   public:
    /**
     * @brief Construct a preset.
     * @param icon Icon representing this preset.
     * @param background Full-screen background image shown while this preset is running.
     * @param name Preset name (also used to look up its running-message strings; see Timer::getRunningMessage()).
     * @param duration Work interval duration, in milliseconds.
     * @param pauseDuration Short break duration, in milliseconds.
     * @param longPauseDuration Long break duration, in milliseconds.
     * @param longPauseAfter Number of completed cycles between long breaks.
     */
    Preset(Icon *icon, const unsigned char *background, const char *name, unsigned long duration,
           unsigned long pauseDuration, unsigned long longPauseDuration, unsigned int longPauseAfter);
    ~Preset();  ///< Destructor.

    /// @return This preset's icon.
    Icon *getIcon();

    /// @return This preset's full-screen running background image.
    const unsigned char *getBackground();

    /// @return Work interval duration. In DEBUG builds this is divided by 60, for fast iteration.
    unsigned long getDuration();

    /// @return Short break duration. In DEBUG builds this is divided by 60, for fast iteration.
    unsigned long getPauseDuration();

    /// @return Long break duration. In DEBUG builds this is divided by 60, for fast iteration.
    unsigned long getLongPauseDuration();

    /// @return Number of completed cycles between long breaks.
    unsigned int getLongPauseAfter();

    /// @return This preset's name.
    const char *getName();
};

/// The pomodoro state machine's states. See Timer::loop() for the transition table.
enum class TimerState {
    SelectingPreset,             ///< Idle, browsing presets before starting a timer.
    Running,                     ///< A work interval is actively counting down.
    WaitingConfirmStartOfBreak,  ///< Work interval finished; waiting for the user to confirm starting the break.
    RunningBreak,                ///< A break interval is actively counting down.
    WaitingConfirmEndOfBreak,    ///< Break interval finished; waiting for the user to confirm resuming work.
    UserInitiatedPause,          ///< Running was manually paused by the user.
    UserInitiatedBreakPause,     ///< RunningBreak was manually paused by the user.
    Stopped                      ///< Timer cancelled back to preset selection.
};

/**
 * @brief Owns the pomodoro TimerState machine, its presets, and the UI it drives.
 *
 * Timer combines three responsibilities that would normally be split apart: state transitions
 * (start/pause/resume/startBreak/stop), timekeeping (startTime/elapsed/totalPausedTime, all
 * measured via millis()), and drawing (the draw-/handle-prefixed private methods, implemented across
 * src/states/, one file per TimerState). loop() is the single entry point tying all three
 * together each iteration of the main Arduino loop().
 */
class Timer {
   private:
    DISPLAY_CLASS &display;
    std::vector<Preset> presets;
    Preset *currentPreset;
    unsigned int presetIndex;
    TimerState state;
    unsigned long startTime;  ///< millis() timestamp the current interval began at (set by reset()).
    unsigned long elapsed;    ///< Milliseconds elapsed in the current interval, refreshed each loop() while running.
    unsigned long pauseStartTime;  ///< millis() timestamp the current user-initiated pause began at.
    unsigned long
        totalPausedTime;  ///< Total milliseconds spent paused during the current interval, subtracted out of elapsed.

    bool isLongBreak = false;          ///< Whether the break in progress/about to start is a long break.
    unsigned int cycles = 0;           ///< Completed work cycles for the current preset, since power-on.
    unsigned long minutesWorked = 0;   ///< Minutes worked so far for the current preset, since power-on.
    unsigned long minutesOnBreak = 0;  ///< Minutes on break so far for the current preset, since power-on.
    unsigned long longestEarnedPauseInShortCycles =
        0;  ///< Longest break duration earned across short cycles since the last long break.
    unsigned long currentBreakDuration = 0;  ///< Duration of the break currently running/about to start.

    Menu *topMenu;         ///< Persistent menu bar (Pause/Break/Cancel-style actions), shown across most states.
    bool menuNeedsRedraw;  ///< Whether only the menu bar (not the full screen) needs redrawing this loop.

    Menu *confirmationMenu;  ///< Menu shown in the WaitingConfirm* states; rebuilt with state-specific items each time.
    bool flashingIcon;       ///< Whether the confirmation icon should be in its "flashed" visual state this frame.

    const unsigned char *breakImage;  ///< Background image chosen for the current long break.

    bool needsRedraw;      ///< Whether a (possibly partial) redraw is needed this loop.
    bool needsFullRedraw;  ///< Whether the next redraw must clear and fully repaint the screen.
    int lastEncoderCount;  ///< Encoder count last synced to topMenu, used to resume tracking after a state change.
    unsigned long lastRedrawTime;  ///< millis() timestamp of the last redraw, used to throttle to ::redrawInterval.
    unsigned long lastPauseRedrawTime;  ///< millis() timestamp of the last redraw while paused.
    bool lastPauseState;  ///< Paused/not-paused flag as of the last redraw, used to detect the transition.
    static const unsigned long REDRAW_INTERVAL_DEFAULT = 5000;  ///< Default redraw throttle, in ms.
    static const unsigned long REDRAW_INTERVAL_FAST =
        1000;  ///< Faster redraw throttle used in the final seconds of an interval, in ms.
    static const unsigned long RUNNING_MESSAGE_REFRESH_INTERVAL =
        5 * 60 * 1000;  ///< How often the running speech-bubble message rotates, in ms.
    unsigned long redrawInterval = REDRAW_INTERVAL_DEFAULT;  ///< Current redraw throttle in effect.

    bool showSpeechBubble =
        true;  ///< Persisted "msgs" setting: whether to show background + speech-bubble messages while running.

    static const unsigned long IDLE_BLANK_TIMEOUT =
        10000;  ///< Idle time in TimerState::SelectingPreset before the display blanks, in ms.
    static const unsigned long PAUSE_BLANK_TIMEOUT =
        5 * 60 * 1000;               ///< Idle time while user-paused before the display blanks, in ms.
    unsigned long lastActivityTime;  ///< millis() timestamp of the last activity seen while idle/paused, relative to
                                     ///< whichever *_BLANK_TIMEOUT currently applies.
    bool displayBlanked = false;     ///< Whether the display is currently blanked and the panel controller hibernating.
    int blankEncoderSnapshot;        ///< Encoder count captured when blankDisplay() was called, used by
                                     ///< wakeDisplayIfTriggered() to detect movement.

    /**
     * @brief Blank and hibernate the display to save power.
     *
     * E-paper is bistable: cutting power alone leaves the last image visible, so this draws a
     * black/white flash first (a single refresh often isn't enough to fully clear ghosting left
     * by prior partial updates) before hibernating. Called from handleSelectingPreset() after
     * ::IDLE_BLANK_TIMEOUT, and from handleRunning()/handleRunningBreak() after
     * ::PAUSE_BLANK_TIMEOUT while user-paused.
     * @param encoderCount Pointer to the shared, debounced encoder count, snapshotted so a later
     *                      wakeDisplayIfTriggered() call can detect movement.
     */
    void blankDisplay(volatile const int *encoderCount);

    /**
     * @brief If the display is blanked and the button was pressed or the encoder moved, wake it.
     *
     * Waking requires a full re-init (the only way to recover from hibernate() is a hardware
     * reset), so this is deliberately not attempted every loop() — only call it while
     * ::displayBlanked is true.
     * @param encoderCount Pointer to the shared, debounced encoder count.
     * @return true if a wake was triggered (and performed) this call.
     */
    bool wakeDisplayIfTriggered(volatile const int *encoderCount);

    /// Draw the WaitingConfirmStartOfBreak/WaitingConfirmEndOfBreak screen. See
    /// src/states/timer_waiting_for_confirmation.cpp.
    void drawWaitingForConfirmation();
    /// Draw the full preset-selection screen. See src/states/timer_selecting_preset.cpp.
    void drawPresetSelection();
    /// Draw only the parts of the preset-selection screen that changed since the last redraw.
    void drawPartialPresetSelection();
    /// Draw the persistent top menu bar. @return The Y coordinate just below the drawn bar.
    int drawMenuBar();
    /// Draw the Running/UserInitiatedPause screen. See src/states/timer_running.cpp.
    void drawRunning();
    /// Handle input/timing for the WaitingConfirm* states. See src/states/timer_waiting_for_confirmation.cpp.
    void handleWaitingForConfirmation(volatile const int *encoderCount);
    /**
     * @brief Handle input for the SelectingPreset state, including idle display-blanking.
     *
     * After ::IDLE_BLANK_TIMEOUT of no encoder/button activity, blanks and hibernates the
     * display to save power. The next encoder movement or button press only wakes the display
     * back up (a full re-init, since hibernate() requires a hardware reset to recover) — that
     * input is not also applied as a preset selection/start. See src/states/timer_selecting_preset.cpp.
     */
    void handleSelectingPreset(volatile const int *encoderCount);
    /**
     * @brief Handle input/timing for the Running/UserInitiatedPause states, including idle
     * display-blanking while user-paused.
     *
     * After ::PAUSE_BLANK_TIMEOUT of no encoder/button activity in TimerState::UserInitiatedPause,
     * blanks the display the same way handleSelectingPreset() does (see blankDisplay()); the
     * Running state itself never blanks. See src/states/timer_running.cpp.
     */
    void handleRunning(volatile const int *encoderCount);
    /**
     * @brief Handle input/timing for the RunningBreak/UserInitiatedBreakPause states, including
     * idle display-blanking while user-paused.
     *
     * After ::PAUSE_BLANK_TIMEOUT of no encoder/button activity in TimerState::UserInitiatedBreakPause,
     * blanks the display the same way handleSelectingPreset() does (see blankDisplay()); the
     * RunningBreak state itself never blanks. See src/states/timer_running_break.cpp.
     */
    void handleRunningBreak(volatile const int *encoderCount);
    /// Draw the RunningBreak/UserInitiatedBreakPause screen. See src/states/timer_running_break.cpp.
    void drawRunningBreak();

    /// @return A running-message string appropriate for ::currentPreset (e.g. Messages::Preset_Coding_Message).
    const char *getRunningMessage();
    unsigned long lastMessageUpdate;  ///< millis() timestamp the running speech-bubble message was last rotated.

    /// Reset per-interval timekeeping (startTime/elapsed/totalPausedTime) and the menu bar, shared by
    /// start()/stop()/startBreak().
    void reset();

   public:
    /**
     * @brief Construct the timer against a display; starts with no presets, in TimerState::SelectingPreset.
     * @param display Shared e-paper display reference.
     */
    explicit Timer(DISPLAY_CLASS &display);
    ~Timer();  ///< Destructor.

    // Timer owns topMenu/confirmationMenu via raw new/delete and holds a reference member, so
    // it can't be copied safely (a compiler-generated copy would double-free those on
    // destruction) and has no sensible copy semantics anyway; make that explicit rather than
    // relying on implicit (and, for assignment, already-broken) behavior.
    Timer(const Timer &) = delete;
    Timer &operator=(const Timer &) = delete;

    /**
     * @brief Append a preset to the end of the preset list.
     * @param icon Icon representing this preset.
     * @param background Full-screen background image shown while this preset is running.
     * @param name Preset name (also used to look up its running-message strings; see getRunningMessage()).
     * @param duration Work interval duration, in milliseconds.
     * @param pauseDuration Short break duration, in milliseconds.
     * @param longPauseDuration Long break duration, in milliseconds.
     * @param longPauseAfter Number of completed cycles between long breaks (default 4).
     */
    void addPreset(Icon *icon, const unsigned char *background, const char *name, unsigned long duration,
                   unsigned long pauseDuration, unsigned long longPauseDuration, unsigned int longPauseAfter = 4);

    /// @param index Index into the preset list to make current.
    void selectPreset(int index);

    /// Select the next preset, wrapping around at the end.
    void nextPreset();

    /// Select the previous preset, wrapping around at the start.
    void previousPreset();

    /// Switch to TimerState::SelectingPreset and reselect preset index 1.
    void enterPresetSelection();

    /// Begin (or restart) a work interval for the current preset, entering TimerState::Running.
    void start();

    /// Pause the current work or break interval (TimerState::UserInitiatedPause/UserInitiatedBreakPause). No-op if not
    /// running.
    void pause();

    /// Resume from a user-initiated pause, returning to TimerState::Running or TimerState::RunningBreak.
    void resume();

    /**
     * @brief Begin a break interval, incrementing cycle/time statistics and picking a short or long break.
     *
     * Whether the break is long is determined by whether ::cycles is a multiple of the current
     * preset's `longPauseAfter`. Enters TimerState::RunningBreak.
     */
    void startBreak();

    /// Cancel the current interval and return to preset selection (TimerState::SelectingPreset).
    void stop();

    /**
     * @brief Advance the state machine by one iteration: handle input/timing for the current state, then redraw if
     * needed.
     * @param encoderCount Pointer to the shared, debounced encoder count.
     */
    void loop(volatile const int *encoderCount);

    /// @return The current state.
    TimerState getState();

    /// @return The active preset's name, or nullptr if none is selected (e.g. before the first addPreset() call).
    const char *getCurrentPresetName();

    /**
     * @brief Milliseconds remaining in the current work/break interval.
     * @return The remaining time while Running/RunningBreak (or their paused variants), or 0 in
     * any other state (e.g. SelectingPreset).
     */
    unsigned long getRemainingMillis();

    /// @return true if the hardware button was pressed since the last check (and clears the pending flag).
    bool checkAndClearButtonPress();
};

#endif
