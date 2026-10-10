/**
 * @file test_timer.cpp
 * @brief Host tests for the Timer state machine (test/README.md, TMR-xx).
 *
 * Runs the real src/timer.cpp and src/states/ against the fakes in test/fakes/. Time only moves
 * when a test moves fake::now, and loop() is called by hand, as the main loop would.
 */
#include <unity.h>

#include <string>

#include "fakes.h"
#include "../../src/timer.h"

static constexpr unsigned long MIN = 60UL * 1000;

static DISPLAY_CLASS display;
static Button button(0);
static volatile int encoder = 0;
static Timer *timer = nullptr;

/// Preset 0, "Short": 25 min work, 5 min break, 15 min long break after every 2 cycles.
static constexpr unsigned long WORK = 25 * MIN;
static constexpr unsigned long BREAK = 5 * MIN;
static constexpr unsigned long LONG_BREAK = 15 * MIN;

void setUp() {
    fake::reset();
    encoder = 0;
    button.pressed = false;
    timer = new Timer(display);
    timer->addPreset(&icon_stopwatch, nullptr, "Short", WORK, BREAK, LONG_BREAK, 2);
    timer->addPreset(&icon_stopwatch, nullptr, "Long", 50 * MIN, 10 * MIN, 30 * MIN, 4);
    timer->enterPresetSelection();
}

void tearDown() {
    delete timer;
    timer = nullptr;
}

/// Move fake time forward, then run one main-loop iteration.
static void advance(unsigned long ms) {
    fake::now += ms;
    timer->loop(&encoder);
}

static bool command(TimerCommand cmd, int arg = 0) { return timer->handleCommand(cmd, arg, &encoder); }

/// Press the encoder button and let the timer see it.
static void press() {
    button.pressed = true;
    timer->loop(&encoder);
}

/// Turn the encoder one step and let the timer see it.
static void turn(int steps) {
    encoder += steps;
    timer->loop(&encoder);
}

/// Bring a fresh timer (preset 0, cycle 1) into @p state, using commands and time only.
static void goTo(TimerState state) {
    switch (state) {
        case TimerState::SelectingPreset:
            break;
        case TimerState::Running:
            command(TimerCommand::Start);
            break;
        case TimerState::UserInitiatedPause:
            command(TimerCommand::Start);
            command(TimerCommand::Pause);
            break;
        case TimerState::WaitingConfirmStartOfBreak:
            command(TimerCommand::Start);
            advance(WORK);
            break;
        case TimerState::RunningBreak:
            command(TimerCommand::Start);
            command(TimerCommand::Break);
            break;
        case TimerState::UserInitiatedBreakPause:
            command(TimerCommand::Start);
            command(TimerCommand::Break);
            command(TimerCommand::Pause);
            break;
        case TimerState::WaitingConfirmEndOfBreak:
            command(TimerCommand::Start);
            command(TimerCommand::Break);
            advance(BREAK);
            break;
        default:
            TEST_FAIL_MESSAGE("goTo: unsupported state");
    }
    TEST_ASSERT_EQUAL_MESSAGE((int) state, (int) timer->getState(), "goTo did not reach the state");
}

// TMR-01: each state accepts exactly the commands in websocket.md's table; a rejected command
// leaves the state unchanged.
void test_commands_valid_per_state() {
    const TimerCommand commands[] = {TimerCommand::Select, TimerCommand::Start,   TimerCommand::Pause,
                                     TimerCommand::Resume, TimerCommand::Restart, TimerCommand::Stop,
                                     TimerCommand::Break};
    const char *commandNames[] = {"select", "start", "pause", "resume", "restart", "stop", "break"};

    struct Row {
        TimerState state;
        const char *name;
        const char *valid;  ///< Space-separated command names, as in websocket.md.
    };
    const Row table[] = {
        {TimerState::SelectingPreset, "SelectingPreset", "select start"},
        {TimerState::Running, "Running", "pause restart break stop"},
        {TimerState::UserInitiatedPause, "UserInitiatedPause", "resume restart break stop"},
        {TimerState::WaitingConfirmStartOfBreak, "WaitingConfirmStartOfBreak", "break"},
        {TimerState::RunningBreak, "RunningBreak", "pause stop"},
        {TimerState::UserInitiatedBreakPause, "UserInitiatedBreakPause", "resume stop"},
        {TimerState::WaitingConfirmEndOfBreak, "WaitingConfirmEndOfBreak", "start restart stop"},
    };

    for (const Row &row : table) {
        for (unsigned int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
            tearDown();
            setUp();
            goTo(row.state);

            const std::string valid = std::string(" ") + row.valid + " ";
            const bool expected = valid.find(std::string(" ") + commandNames[i] + " ") != std::string::npos;
            const bool accepted = command(commands[i]);

            const std::string message = std::string(commandNames[i]) + " in " + row.name;
            TEST_ASSERT_EQUAL_MESSAGE(expected, accepted, message.c_str());
            if (!accepted) {
                TEST_ASSERT_EQUAL_MESSAGE((int) row.state, (int) timer->getState(),
                                          (message + ": rejected, but the state changed").c_str());
            }
        }
    }
}

// TMR-02: stop returns to preset selection with the same preset still selected
// (bug fixed 2026-10-10: it always jumped to preset index 1).
void test_stop_keeps_selected_preset() {
    TEST_ASSERT_TRUE(command(TimerCommand::Select, 1));
    TEST_ASSERT_TRUE(command(TimerCommand::Start));
    TEST_ASSERT_TRUE(command(TimerCommand::Stop));
    TEST_ASSERT_EQUAL((int) TimerState::SelectingPreset, (int) timer->getState());
    TEST_ASSERT_EQUAL(1, timer->getPresetIndex());

    tearDown();
    setUp();
    TEST_ASSERT_TRUE(command(TimerCommand::Start));  // preset 0
    TEST_ASSERT_TRUE(command(TimerCommand::Break));
    TEST_ASSERT_TRUE(command(TimerCommand::Stop));
    TEST_ASSERT_EQUAL((int) TimerState::SelectingPreset, (int) timer->getState());
    TEST_ASSERT_EQUAL(0, timer->getPresetIndex());
}

// TMR-03: a paused break still runs its full length (bug fixed 2026-10-10: it ended early).
void test_paused_break_runs_full_length() {
    goTo(TimerState::RunningBreak);
    advance(2 * MIN);
    TEST_ASSERT_TRUE(command(TimerCommand::Pause));
    advance(10 * MIN);  // paused for longer than the whole break
    TEST_ASSERT_EQUAL((int) TimerState::UserInitiatedBreakPause, (int) timer->getState());
    TEST_ASSERT_EQUAL(3 * MIN, timer->getRemainingMillis());

    TEST_ASSERT_TRUE(command(TimerCommand::Resume));
    advance(3 * MIN - 1);
    TEST_ASSERT_EQUAL((int) TimerState::RunningBreak, (int) timer->getState());
    TEST_ASSERT_EQUAL(1, timer->getRemainingMillis());
    advance(1);
    TEST_ASSERT_EQUAL((int) TimerState::WaitingConfirmEndOfBreak, (int) timer->getState());
}

// TMR-04: time spent paused doesn't count towards a work interval.
void test_paused_work_excludes_pause_time() {
    goTo(TimerState::Running);
    advance(10 * MIN);
    TEST_ASSERT_TRUE(command(TimerCommand::Pause));
    advance(30 * MIN);
    TEST_ASSERT_EQUAL((int) TimerState::UserInitiatedPause, (int) timer->getState());
    TEST_ASSERT_EQUAL(15 * MIN, timer->getRemainingMillis());

    TEST_ASSERT_TRUE(command(TimerCommand::Resume));
    advance(15 * MIN - 1);
    TEST_ASSERT_EQUAL((int) TimerState::Running, (int) timer->getState());
    advance(1);
    TEST_ASSERT_EQUAL((int) TimerState::WaitingConfirmStartOfBreak, (int) timer->getState());
}

// TMR-05: a long break after every longPauseAfter (here 2) cycles, a short break otherwise.
void test_long_break_every_n_cycles() {
    const bool expectLong[] = {false, true, false, true};
    command(TimerCommand::Start);
    for (unsigned int cycle = 1; cycle <= 4; cycle++) {
        advance(WORK);
        TEST_ASSERT_EQUAL((int) TimerState::WaitingConfirmStartOfBreak, (int) timer->getState());
        TEST_ASSERT_TRUE(command(TimerCommand::Break));

        const std::string message = "cycle " + std::to_string(cycle);
        TEST_ASSERT_EQUAL_MESSAGE(cycle, timer->getCycles(), message.c_str());
        TEST_ASSERT_EQUAL_MESSAGE(expectLong[cycle - 1], timer->isOnLongBreak(), message.c_str());
        TEST_ASSERT_EQUAL_MESSAGE(expectLong[cycle - 1] ? LONG_BREAK : BREAK, timer->getIntervalMillis(),
                                  message.c_str());

        advance(timer->getIntervalMillis());
        TEST_ASSERT_EQUAL((int) TimerState::WaitingConfirmEndOfBreak, (int) timer->getState());
        TEST_ASSERT_TRUE(command(TimerCommand::Start));
    }
}

// TMR-06: restart sets the work interval back to zero but keeps the cycle count.
void test_restart_keeps_cycle_count() {
    goTo(TimerState::WaitingConfirmEndOfBreak);
    TEST_ASSERT_TRUE(command(TimerCommand::Start));
    TEST_ASSERT_EQUAL(1, timer->getCycles());

    advance(10 * MIN);
    TEST_ASSERT_EQUAL(15 * MIN, timer->getRemainingMillis());
    TEST_ASSERT_TRUE(command(TimerCommand::Restart));
    TEST_ASSERT_EQUAL((int) TimerState::Running, (int) timer->getState());
    TEST_ASSERT_EQUAL(WORK, timer->getRemainingMillis());
    TEST_ASSERT_EQUAL(1, timer->getCycles());
}

// TMR-07: remaining and interval time are 0 outside the running states, and remaining never
// wraps around to a huge number once the interval is over.
void test_remaining_time_never_wraps() {
    TEST_ASSERT_EQUAL(0, timer->getRemainingMillis());
    TEST_ASSERT_EQUAL(0, timer->getIntervalMillis());

    goTo(TimerState::Running);
    fake::now += WORK + 5 * MIN;  // past the end, before loop() has noticed
    TEST_ASSERT_EQUAL(0, timer->getRemainingMillis());

    timer->loop(&encoder);
    TEST_ASSERT_EQUAL((int) TimerState::WaitingConfirmStartOfBreak, (int) timer->getState());
    TEST_ASSERT_EQUAL(0, timer->getRemainingMillis());
    TEST_ASSERT_EQUAL(0, timer->getIntervalMillis());
}

// TMR-08: a select command with an out-of-range index is rejected and changes nothing.
void test_select_out_of_range_rejected() {
    TEST_ASSERT_TRUE(command(TimerCommand::Select, 1));
    TEST_ASSERT_FALSE(command(TimerCommand::Select, -1));
    TEST_ASSERT_FALSE(command(TimerCommand::Select, 2));
    TEST_ASSERT_FALSE(command(TimerCommand::Select, 1000));
    TEST_ASSERT_EQUAL(1, timer->getPresetIndex());
    TEST_ASSERT_EQUAL((int) TimerState::SelectingPreset, (int) timer->getState());
}

// TMR-09: the selection cursor steps through the presets, then Status, then Settings, wrapping
// both ways; landing on a preset selects it, and Status/Settings keep the last preset selected.
void test_selection_cursor_wraps() {
    turn(1);  // preset 1
    TEST_ASSERT_EQUAL(1, timer->getPresetIndex());
    turn(1);  // Status
    TEST_ASSERT_EQUAL(1, timer->getPresetIndex());
    turn(1);  // Settings
    TEST_ASSERT_EQUAL(1, timer->getPresetIndex());
    turn(1);  // wraps to preset 0
    TEST_ASSERT_EQUAL(0, timer->getPresetIndex());
    turn(-1);  // wraps back to Settings
    TEST_ASSERT_EQUAL(0, timer->getPresetIndex());
    turn(-1);  // Status: pressing opens it rather than starting a timer
    press();
    TEST_ASSERT_EQUAL((int) TimerState::SelectingPreset, (int) timer->getState());

    turn(-1);  // preset 1: pressing starts it
    TEST_ASSERT_EQUAL(1, timer->getPresetIndex());
    press();
    TEST_ASSERT_EQUAL((int) TimerState::Running, (int) timer->getState());
    TEST_ASSERT_EQUAL_STRING("Long", timer->getCurrentPresetName());
}

// TMR-10: a 24-hour preset runs and finishes (for the planned battery test).
void test_24_hour_preset() {
    const unsigned long day = 24 * 60 * MIN;
    timer->addPreset(&icon_stopwatch, nullptr, "Day", day, BREAK, LONG_BREAK, 4);
    TEST_ASSERT_TRUE(command(TimerCommand::Select, 2));
    TEST_ASSERT_TRUE(command(TimerCommand::Start));
    TEST_ASSERT_EQUAL(day, timer->getIntervalMillis());

    advance(day - MIN);
    TEST_ASSERT_EQUAL((int) TimerState::Running, (int) timer->getState());
    TEST_ASSERT_EQUAL(MIN, timer->getRemainingMillis());
    advance(MIN);
    TEST_ASSERT_EQUAL((int) TimerState::WaitingConfirmStartOfBreak, (int) timer->getState());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_commands_valid_per_state);
    RUN_TEST(test_stop_keeps_selected_preset);
    RUN_TEST(test_paused_break_runs_full_length);
    RUN_TEST(test_paused_work_excludes_pause_time);
    RUN_TEST(test_long_break_every_n_cycles);
    RUN_TEST(test_restart_keeps_cycle_count);
    RUN_TEST(test_remaining_time_never_wraps);
    RUN_TEST(test_select_out_of_range_rejected);
    RUN_TEST(test_selection_cursor_wraps);
    RUN_TEST(test_24_hour_preset);
    return UNITY_END();
}
