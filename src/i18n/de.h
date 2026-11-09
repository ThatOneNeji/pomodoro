/**
 * @file de.h
 * @brief German strings for the ::Msgs API (see messages.h for how these are selected/consumed).
 */
#pragma once
#include <vector>
#include <array>

/**
 * @brief German localized strings and message lists (see the file-level @note: currently incomplete
 * relative to en.h, and largely untranslated in the `Msgs` struct itself).
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

    static constexpr const char *SETTINGS = "Settings";   ///< Preset selection screen "Settings" menu item.
    static constexpr const char *STATUS = "Status";       ///< Preset selection screen "Status" menu item.
    static constexpr const char *BACK = "Back";           ///< Settings screen "Back" row.
    static constexpr const char *SHUTDOWN = "Shut down";  ///< Settings screen "Shut down" button.
    static constexpr const char *WIFI = "WiFi";           ///< Checkbox label for switching WiFi on or off.
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
    static constexpr std::array GENERIC_PRESET_MESSAGES = {
        "Okaaay, let's go!",
        "Heute schon auf Reddit gewesen?",
        "Bist du hydriert?",
        "Vielleicht eine kleine Kaffeepause?",
        "Langsam ist aber auch Zeit fuer Feierabend, oder nicht?",
        "Schoen hier, aber warst du heute schon auf Reddit?",
        "Du schaffst das (hoffen wir)",
        "Schauen wir mal was wird, was wird",
    };

    /// Candidate trivia facts shown alongside GENERIC_PRESET_MESSAGES while a preset is running.
    static constexpr std::array CHAT_GPT_FACTS = {
        "Das Gehirn eines Elefanten enthaelt ueber 257 Mrd. Neuronen und zeigt starke Emotionen.",
        "Tintenfische besitzen drei Herzen und ein ausgekluegeltes Nervensystem, das Probleme effizient loest.",
        "Voegel besitzen Magnetrezeptoren, die ihnen helfen, das Erdmagnetfeld als Kompass zu nutzen.",
        "Bienen kommunizieren durch einen praezisen Schwaenzeltanz, mit dem sie Futterquellen uebermitteln.",
        "Schlangen spueren Waerme ueber spezialisierte Rezeptoren, die sie zu geschickten Jaegern machen.",
        "Lungenfische atmen mit Kiemen und primitiven Lungen, ueberbruecken so Wasser und Land.",
        "Chamaeleons aendern ihre Farbe nicht nur zur Tarnung, sondern auch zur Kommunikation.",
        "Kojoten sind sehr anpassungsfaehig und leben in verschieden Umgebungen, von Wuesten bis zu Staedten.",
        "Kolibris sind die einzigen Voegel, die rueckwaerts fliegen koennen, dank spezieller Flugmuskulatur.",
        "Giraffen haben ein komplexes Blutkreislaufsystem, das ihren Kopf mit sauerstoffreichem Blut versorgt.",
        "Haie besitzen Lorenzinische Ampullen, die ihnen helfen, schwache elektrische Felder zu spueren.",
        "Ein Ameisenstaat kann das Gewicht mehrerer Elefanten tragen",
        "Faultiere bewegen sich sehr langsam, damit Algen sich ansiedeln und sie gut getarnt bleiben.",
        "Geparden erreichen Top- Geschwindigkeiten, koennen aber nur kurz laufen.",
        "Seesterne haben kein Gehirn, sondern ein verteiltes Nervensystem, das in ihren Armen wirkt.",
        "Wale nutzen Infraschall, deren Toene sich ueber viele km im Ozean ausbreiten.",
        "Axolotl regenerieren Gliedmassen, was sie zu interessanten Forschungsobjekten macht.",
        "Pinguine speichern Waerme, indem sie ihre Federn eng anlegen und den Waermeverlust minimieren.",
        "Die DNA vieler Tiere zeigt erstaunliche Gemeinsamkeiten, die evolutionaere Verwandtschaften offenbaren.",
        "Schmetterlinge haben ausgekluegelte Farberkennungssysteme",
        "Menschen und Bananen teilen etwa 60% ihrer Gene, ein Hinweis auf geteilte Wurzeln.",
        "Quallen haben uralte Gene, die sich kaum veraendert haben.",
        "Koalas haben fingerabdruckartige Rillen, aehnlich wie Menschen.",
        "Haie und Rochen teilen genetische Wurzeln und gehoeren zur Knorpelfisch-Familie.",
        "Every move on the board is a rebellion; no piece is sacred, and the king is just another target.",
        "In chess, chaos is art, each pawn a revolutionary spark, every check a call to arms.",
        "Google en passant  holy hell!",
        "Als die Dinosaurier existierten, gab es Vulkane, die auf dem Mond ausbrachen.",
        "Die einzigen Buchstaben, die nicht im Periodensystem vorkommen, sind 'J' und 'Q'.",
        "Wenn ein Eisbaer und ein Grizzlybaer sich paaren, wird 'Pizzy Bear' genannt.",
        "Daniel Radcliffe war allergisch gegen seine Harry-Potter-Brille, doch Harry Potter traegt sie.",
        "Im Englischen heisst es 'French Exit', wenn man ohne Abschied geht",
        "In Arizona kann das Faellen eines Saguaro-Kaktus als Verbrechen geahndet werden.",
        "Der in Statuen gezeigte Buddha ist nicht der wahre Buddha; der echte war mager durch Askese.",
        "Ein einzelner Spaghetti-Strang wird als 'Spaghetto' bezeichnet, eine kuriose Tatsache.",
        "Princess Peach blieb still bis 1988, da Designer sie nicht beweglich machten.",
        "Der erste Film mit Soundtrack war Schneewittchen und die sieben Zwerge.",
        "Reichst du mit deinen Autoschluesseln an deinen Kopf, erhoeht sich die Reichweite der Fernbedienung.",
        "Fruchtaufkleber sind essbar, wie auch das Obst selbst; Vor dem Verzehr waschen!",
        "Der Name des Riesenameisbers ist Myrmecophaga Tridactyla, was 'Ameisenessend mit 3 Fingern' heisst.",
        "Das Wort 'Astronaut' kommt aus dem Griechischen 'astro' = Stern, und 'naut' heisst Seefahrer."};
};
