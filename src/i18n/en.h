/**
 * @file en.h
 * @brief English strings for the ::Msgs API (see strings.h for how these are selected/consumed).
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

    static constexpr const char *SETTINGS = "Settings";  ///< Splash screen "Settings" menu item.
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
    static constexpr std::array<const char *, 4> BREAK_MESSAGES = {"Break time!", "Take a rest", "Time to relax",
                                                                   "Well done!"};

    /// Candidate "restart timer" messages used in LPE mode.
    static constexpr std::array<const char *, 4> RESTARTTIMERMSGLPE = {"Noch mal!", "AGAIN!", "Here we go again...",
                                                                       "Do it agane"};

    /// Candidate "restart timer" messages used outside LPE mode.
    static constexpr std::array<const char *, 4> RESTARTTIMERMSG = {"Restart", "Let's go again", "One more time"};

    /// Candidate speech-bubble messages shown while a preset timer is running.
    static constexpr std::array<const char *, 8> GENERIC_PRESET_MESSAGES = {
        "Okaaay, let's go!",
        "Have you been on Reddit today?",
        "Are you hydrated?",
        "Perhaps a short coffee break?",
        "It's about time to call it a day,\nisn't it?",
        "Nice here, but have you been on\nReddit today?",
        "You can do it (we hope)",
        "Let's see what happens.\n\n                          What happens?"};

    /// Candidate trivia facts shown alongside GENERIC_PRESET_MESSAGES while a preset is running.
    static constexpr std::array<const char *, 42> CHAT_GPT_FACTS = {
        "An elephant's brain contains over\n257 billion neurons\nand displays strong emotions.",
        "Octopuses have three hearts and a\nsophisticated nervous system\nthat efficiently solves problems.",
        "Birds possess magnetoreceptors that\nhelp them use the Earth's\nmagnetic field as a compass.",
        "Bees communicate through a precise\nwaggle dance, which they use\nto convey food sources.",
        "Snakes sense heat through specialized\nreceptors, which makes them skilled hunters.",
        "Lungfish breathe with gills and\nprimitive lungs, bridging the\ngap between water and land.",
        "Chameleons change their color not\nonly for camouflage, but also\nfor communication.",
        "Coyotes are highly adaptable and\nlive in diverse environments,\nfrom deserts to cities.",
        "Hummingbirds are the only birds that\ncan fly backwards, thanks\nto special flight muscles.",
        "Giraffes have a complex circulatory\nsystem that supplies their\nheads with oxygen-rich blood.",
        "Sharks possess ampullae of Lorenzini,\nwhich help them sense\nweak electric fields.",
        "An ant colony can support the weight\nof several elephants.",
        "Sloths move very slowly, so that\nalgae can grow on them and\nthey remain well camouflaged.",
        "Cheetahs reach top speeds, but can\nonly run for short distances.",
        "Starfish don't have a brain, but a\ndistributed nervous system,\nwhich operates in their arms.",
        "Whales use infrasound, the sounds\nof which travel many\nkilometers through the ocean.",
        "Axolotls regenerate limbs, making\nthem interesting subjects\nfor research.",
        "Penguins conserve heat by folding\ntheir feathers tightly\nand minimizing heat loss.",
        "The DNA of many animals shows\nastonishing similarities that\nreveal evolutionary relations.",
        "Butterflies have sophisticated\ncolor recognition systems.",
        "Humans and bananas share about 60%\nof their genes, a clue to\nshared roots.",
        "Jellyfish have ancient genes that\nhave barely changed.",
        "Koalas have fingerprint-like ridges,\nsimilar to humans.",
        "Sharks and rays share genetic roots\nand belong to the\ncartilaginous fish family.",
        "Every move on the board is a\nrebellion; no piece is sacred,\nand the king is just another target.",
        "In chess, chaos is art, each pawn a\nrevolutionary spark,\nevery check a call to arms.",
        "Google en passant\n\nHoly hell!",
        "When dinosaurs existed, there were\nvolcanoes erupting on the moon.",
        "The only letters not found in the\nperiodic table are 'J' and 'Q'.",
        "When a polar bear and a grizzly bear\nmate, it's called a 'pizzy bear'.",
        "Daniel Radcliffe was allergic to his\nHarry Potter glasses,\nyet Harry Potter wears them.",
        "In English, it's called a\n'French exit' when someone leaves\nwithout saying goodbye.",
        "In Arizona, cutting down a saguaro\ncactus can be a crime.",
        "The Buddha depicted in statues is\nnot the real Buddha; the real one\nwas thin through asceticism.",
        "A single strand of spaghetti is called\na 'spaghetto', a curious fact.",
        "Princess Peach remained still until\n1988, as designers didn't\nmake her movable.",
        "The first film with a soundtrack\nwas Snow White and the Seven Dwarfs.",
        "If you hold your car keys to your\nhead, the remote control's\nrange increases.",
        "Fruit stickers are edible,as is the \nfruit itself, wash before eating!",
        "The name of the giant anteater is\nMyrmecophaga tridactyla, which means\n'ant-eating with three fingers'.",
        "The word 'astronaut' comes from the\nGreek 'astro' = star,\nand 'naut' means seafarer."};
};

/// Legacy duplicate of Msgs::GENERIC_PRESET_MESSAGES, predating the struct; still read by strings.cpp.
// TODO: Convert this to struct
static const std::vector<const char *> genericPresetMessages = {
    "Okaaay, let's go!",
    "Have you been on Reddit today?",
    "Are you hydrated?",
    "Perhaps a short coffee break?",
    "It's about time to call it a day,\nisn't it?",
    "Nice here, but have you been on\nReddit today?",
    "You can do it (we hope)",
    "Let's see what happens.\n\n                          What happens?"};

/// Legacy duplicate of Msgs::BREAK_MESSAGES, predating the struct; still read by strings.cpp.
// TODO: Convert this to struct
static const std::vector<const char *> genericStartBreakMessages = {"Break time!", "Take a rest", "Time to relax",
                                                                    "Well done!"};

/// Legacy duplicate of Msgs::CHAT_GPT_FACTS, predating the struct; still read by strings.cpp.
// TODO: Convert this to struct

static const std::vector<const char *> chatGptFacts = {
    "An elephant's brain contains over\n257 billion neurons\nand displays strong emotions.",
    "Octopuses have three hearts and a\nsophisticated nervous system\nthat efficiently solves problems.",
    "Birds possess magnetoreceptors that\nhelp them use the Earth's\nmagnetic field as a compass.",
    "Bees communicate through a precise\nwaggle dance, which they use\nto convey food sources.",
    "Snakes sense heat through specialized\nreceptors, which makes them skilled hunters.",
    "Lungfish breathe with gills and\nprimitive lungs, bridging the\ngap between water and land.",
    "Chameleons change their color not\nonly for camouflage, but also\nfor communication.",
    "Coyotes are highly adaptable and\nlive in diverse environments,\nfrom deserts to cities.",
    "Hummingbirds are the only birds that\ncan fly backwards, thanks\nto special flight muscles.",
    "Giraffes have a complex circulatory\nsystem that supplies their\nheads with oxygen-rich blood.",
    "Sharks possess ampullae of Lorenzini,\nwhich help them sense\nweak electric fields.",
    "An ant colony can support the weight\nof several elephants.",
    "Sloths move very slowly, so that\nalgae can grow on them and\nthey remain well camouflaged.",
    "Cheetahs reach top speeds, but can\nonly run for short distances.",
    "Starfish don't have a brain, but a\ndistributed nervous system,\nwhich operates in their arms.",
    "Whales use infrasound, the sounds\nof which travel many\nkilometers through the ocean.",
    "Axolotls regenerate limbs, making\nthem interesting subjects\nfor research.",
    "Penguins conserve heat by folding\ntheir feathers tightly\nand minimizing heat loss.",
    "The DNA of many animals shows\nastonishing similarities that\nreveal evolutionary relations.",
    "Butterflies have sophisticated\ncolor recognition systems.",
    "Humans and bananas share about 60%\nof their genes, a clue to\nshared roots.",
    "Jellyfish have ancient genes that\nhave barely changed.",
    "Koalas have fingerprint-like ridges,\nsimilar to humans.",
    "Sharks and rays share genetic roots\nand belong to the\ncartilaginous fish family.",
    "Every move on the board is a\nrebellion; no piece is sacred,\nand the king is just another target.",
    "In chess, chaos is art, each pawn a\nrevolutionary spark,\nevery check a call to arms.",
    "Google en passant\n\nHoly hell!",
    "When dinosaurs existed, there were\nvolcanoes erupting on the moon.",
    "The only letters not found in the\nperiodic table are 'J' and 'Q'.",
    "When a polar bear and a grizzly bear\nmate, it's called a 'pizzy bear'.",
    "Daniel Radcliffe was allergic to his\nHarry Potter glasses,\nyet Harry Potter wears them.",
    "In English, it's called a\n'French exit' when someone leaves\nwithout saying goodbye.",
    "In Arizona, cutting down a saguaro\ncactus can be a crime.",
    "The Buddha depicted in statues is\nnot the real Buddha; the real one\nwas thin through asceticism.",
    "A single strand of spaghetti is called\na 'spaghetto', a curious fact.",
    "Princess Peach remained still until\n1988, as designers didn't\nmake her movable.",
    "The first film with a soundtrack\nwas Snow White and the Seven Dwarfs.",
    "If you hold your car keys to your\nhead, the remote control's\nrange increases.",
    "Fruit stickers are edible,as is the \nfruit itself, wash before eating!",
    "The name of the giant anteater is\nMyrmecophaga tridactyla, which means\n'ant-eating with three fingers'.",
    "The word 'astronaut' comes from the\nGreek 'astro' = star,\nand 'naut' means seafarer."};
