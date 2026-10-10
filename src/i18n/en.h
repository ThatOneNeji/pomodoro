/**
 * @file en.h
 * @brief English strings for the ::Msgs API (see messages.h for how these are selected/consumed).
 */
#pragma once
#include <vector>
#include <array>

/**
 * @brief English localized strings and message lists.
 *
 * Every language header (en.h, de.h, and eventually af.h) is expected to define this same
 * struct API; see msgs_base.h for the (currently unused) aspirational key list.
 */
struct Msgs {
    /* Single words */
    static constexpr const char *OPEN = "Open";              ///< Generic "open" label.
    static constexpr const char *CLOSE = "Close";            ///< Generic "close" label.
    static constexpr const char *STOP = "Stop";              ///< "Cancel timer" menu item.
    static constexpr const char *PAUSE = "Pause";            ///< "Pause timer" menu item.
    static constexpr const char *PAUSED = "- PAUSED -";      ///< Shown while the timer is paused.
    static constexpr const char *STATISTICS = "Statistics";  ///< Statistics screen heading.
    static constexpr const char *INTOTAL = "In total";       ///< Statistics section label.
    static constexpr const char *DONE = "Done";              ///< Header shown once a work interval finishes.
    static constexpr const char *BREAKOVER = "Break Over";   ///< Header shown once a break finishes.
    static constexpr const char *RESUME = "Resume";          ///< "Resume timer" menu item.
    static constexpr const char *BREAKNOW = "Break now";     ///< "Start break early" menu item.
    static constexpr const char *SKIPBREAK = "Skip Break";   ///< "Skip the current break" menu item.
    static constexpr const char *TOSELECT = "To Select";     ///< "Back to preset selection" menu item.

    static constexpr const char *SETTINGS = "Settings";  ///< Preset selection screen "Settings" menu item.
    static constexpr const char *STATUS = "Status";      ///< Preset selection screen "Status" menu item.
    static constexpr const char *BACK = "Back";          ///< Settings screen "Back" row.
    static constexpr const char *MISCMSG =
        "Distracting news";                                   ///< Checkbox label for the random-facts/messages setting.
    static constexpr const char *MODE = "LPE Mode";           ///< Checkbox label for the alternate icon/message theme.
    static constexpr const char *RSTDEVICE = "Reset Device";  ///< Checkbox label for the factory-reset action.

    static constexpr const char *CURRENTCYCLE = "Current cycle";  ///< Statistics label for the in-progress cycle count.
    static constexpr const char *CURRENTTIME = "Working hours";   ///< Statistics label for the in-progress work time.
    static constexpr const char *CURRENTBREAKTIME = "Break time";  ///< Statistics label for the in-progress break time.
    static constexpr const char *TOTALCYCLES = "Cycles";           ///< Statistics label for the lifetime cycle count.
    static constexpr const char *TOTALTIME = "Working hours";      ///< Statistics label for the lifetime work time.
    static constexpr const char *TOTALBREAKTIME = "Break time";    ///< Statistics label for the lifetime break time.

    /* Message lists */

    /// Candidate messages shown when a break begins.
    static constexpr std::array BREAK_MESSAGES = {"Break time!", "Take a rest", "Time to relax", "Well done!"};

    /// Candidate "restart timer" messages used in LPE mode.
    static constexpr std::array RESTARTTIMERMSGLPE = {"Noch mal!", "AGAIN!", "Here we go again...", "Do it agane"};

    /// Candidate "restart timer" messages used outside LPE mode.
    static constexpr std::array RESTARTTIMERMSG = {"Restart", "Let's go again", "One more time"};

    /// Candidate speech-bubble messages shown while a preset timer is running.
    static constexpr std::array GENERIC_PRESET_MESSAGES = {"Okaaay, let's go!",
                                                           "Have you been on Reddit today?",
                                                           "Are you hydrated?",
                                                           "Perhaps a short coffee break?",
                                                           "It's about time to call it a day, isn't it?",
                                                           "Nice here, but have you been on Reddit today?",
                                                           "You can do it (we hope)",
                                                           "Let's see what happens. What happens?"};

    /// Candidate trivia facts shown alongside GENERIC_PRESET_MESSAGES while a preset is running.
    static constexpr std::array CHAT_GPT_FACTS = {
        "An elephant's brain contains over 257 billion neurons and displays strong emotions.",
        "Octopuses have three hearts and a sophisticated nervous system that efficiently solves problems.",
        "Birds possess magnetoreceptors that help them use the Earth's magnetic field as a compass.",
        "Bees communicate through a precise waggle dance, which they use to convey food sources.",
        "Snakes sense heat through specialized receptors, which makes them skilled hunters.",
        "Lungfish breathe with gills and primitive lungs, bridging the gap between water and land.",
        "Chameleons change their color not only for camouflage, but also for communication.",
        "Coyotes are highly adaptable and live in diverse environments, from deserts to cities.",
        "Hummingbirds are the only birds that can fly backwards, thanks to special flight muscles.",
        "Giraffes have a complex circulatory system that supplies their heads with oxygen-rich blood.",
        "Sharks possess ampullae of Lorenzini, which help them sense weak electric fields.",
        "An ant colony can support the weight of several elephants.",
        "Sloths move very slowly, so that algae can grow on them and they remain well camouflaged.",
        "Cheetahs reach top speeds, but can only run for short distances.",
        "Starfish don't have a brain, but a distributed nervous system, which operates in their arms.",
        "Whales use infrasound, the sounds of which travel many kilometers through the ocean.",
        "Axolotls regenerate limbs, making them interesting subjects for research.",
        "Penguins conserve heat by folding their feathers tightly and minimizing heat loss.",
        "The DNA of many animals shows astonishing similarities that reveal evolutionary relations.",
        "Butterflies have sophisticated color recognition systems.",
        "Humans and bananas share about 60% of their genes, a clue to shared roots.",
        "Jellyfish have ancient genes that have barely changed.",
        "Koalas have fingerprint-like ridges, similar to humans.",
        "Sharks and rays share genetic roots and belong to the cartilaginous fish family.",
        "Every move on the board is a rebellion; no piece is sacred, and the king is just another target.",
        "In chess, chaos is art, each pawn a revolutionary spark, every check a call to arms.",
        "Google en passant  Holy hell!",
        "When dinosaurs existed, there were volcanoes erupting on the moon.",
        "The only letters not found in the periodic table are 'J' and 'Q'.",
        "When a polar bear and a grizzly bear mate, it's called a 'pizzy bear'.",
        "Daniel Radcliffe was allergic to his Harry Potter glasses, yet Harry Potter wears them.",
        "In English, it's called a 'French exit' when someone leaves without saying goodbye.",
        "In Arizona, cutting down a saguaro cactus can be a crime.",
        "The Buddha depicted in statues is not the real Buddha; the real one was thin through asceticism.",
        "A single strand of spaghetti is called a 'spaghetto', a curious fact.",
        "Princess Peach remained still until 1988, as designers didn't make her movable.",
        "The first film with a soundtrack was Snow White and the Seven Dwarfs.",
        "If you hold your car keys to your head, the remote control's range increases.",
        "Fruit stickers are edible,as is the fruit itself, wash before eating!",
        "The name of the giant anteater is Myrmecophaga tridactyla, which means 'ant-eating with three fingers'.",
        "The word 'astronaut' comes from the Greek 'astro' = star, and 'naut' means seafarer."};
};
