#pragma once
#include <vector>
#include <array>

struct Msgs {
    /* Single words */
    static constexpr const char *OPEN = "Open";
    static constexpr const char *CLOSE = "Close";
    static constexpr const char *STOP = "Stop";
    static constexpr const char *PAUSE = "Pause";
    static constexpr const char *PAUSED = "- PAUSED -";
    static constexpr const char *STATISTICS = "Statistics";
    static constexpr const char *INTOTAL = "In total";
    static constexpr const char *DONE = "Done";
    static constexpr const char *BREAKOVER = "Break Over";
    static constexpr const char *RESUME = "Resume";
    static constexpr const char *BREAKNOW = "Break now";
    static constexpr const char *SKIPBREAK = "Skip Break";
    static constexpr const char *TOSELECT = "To Select";

    static constexpr const char *SETTINGS = "Settings";         // Einstellungen
    static constexpr const char *MISCMSG = "Distracting news";  // Ablenkende Nachrichten
    static constexpr const char *MODE = "LPE Mode";             // LPE Modus
    static constexpr const char *RSTDEVICE = "Reset Device";    // Reset Device

    static constexpr const char *CURRENTCYCLE = "Current cycle";   // Aktueller Zyklus
    static constexpr const char *CURRENTTIME = "Working hours";    // Arbeitszeit
    static constexpr const char *CURRENTBREAKTIME = "Break time";  // Pausenzeit
    static constexpr const char *TOTALCYCLES = "Cycles";           // Zyklen
    static constexpr const char *TOTALTIME = "Working hours";      // Arbeitszeit
    static constexpr const char *TOTALBREAKTIME = "Break time";    // Pausenzeit

    /* Message lists */
    static constexpr std::array<const char *, 4> BREAK_MESSAGES = {"Break time!", "Take a rest", "Time to relax",
                                                                   "Well done!"};

    static constexpr std::array<const char *, 4> RESTARTTIMERMSGLPE = {"Noch mal!", "AGAIN!", "Here we go again...",
                                                                       "Do it agane"};

    static constexpr std::array<const char *, 4> RESTARTTIMERMSG = {"Restart", "Let's go again", "One more time"};

    static constexpr std::array<const char *, 8> GENERIC_PRESET_MESSAGES = {
        "Okaaay, let's go!",
        "Have you been on Reddit today?",
        "Are you hydrated?",
        "Perhaps a short coffee break?",
        "It's about time to call it a day,\nisn't it?",
        "Nice here, but have you been on\nReddit today?",
        "You can do it (we hope)",
        "Let's see what happens.\n\n                          What happens?"};

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

// TODO: Convert this to struct
static const std::vector<const char *> genericStartBreakMessages = {"Break time!", "Take a rest", "Time to relax",
                                                                    "Well done!"};

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

static const std::vector<const char *> chatGptFacts2 = {
    "Das Gehirn eines Elefanten\nenthaelt ueber 257 Mrd. Neuronen\nund zeigt starke Emotionen.",
    "Tintenfische besitzen drei Herzen\nund ein ausgekluegeltes Nervensystem,\ndas Probleme effizient loest.",
    "Voegel besitzen Magnetrezeptoren,\ndie ihnen helfen, das Erdmagnetfeld\nals Kompass zu nutzen.",
    "Bienen kommunizieren durch einen\npraezisen Schwaenzeltanz, mit dem sie\nFutterquellen uebermitteln.",
    "Schlangen spueren Waerme ueber\nspezialisierte Rezeptoren,\ndie sie zu geschickten Jaegern machen.",
    "Lungenfische atmen mit Kiemen\nund primitiven Lungen,\nueberbruecken so Wasser und Land.",
    "Chamaeleons aendern ihre Farbe\nnicht nur zur Tarnung,\nsondern auch zur Kommunikation.",
    "Kojoten sind sehr anpassungsfaehig\nund leben in verschieden Umgebungen,\nvon Wuesten bis zu Staedten.",
    "Kolibris sind die einzigen Voegel,\ndie rueckwaerts fliegen koennen,\ndank spezieller Flugmuskulatur.",
    "Giraffen haben ein komplexes\nBlutkreislaufsystem, das ihren Kopf\nmit sauerstoffreichem Blut versorgt.",
    "Haie besitzen Lorenzinische Ampullen,\ndie ihnen helfen, schwache\nelektrische Felder zu spueren.",
    "Ein Ameisenstaat kann das Gewicht\nmehrerer Elefanten tragen",
    "Faultiere bewegen sich sehr langsam,\ndamit Algen sich ansiedeln\nund sie gut getarnt bleiben.",
    "Geparden erreichen Top-\nGeschwindigkeiten, koennen\naber nur kurz laufen.",
    "Seesterne haben kein Gehirn,\nsondern ein verteiltes Nervensystem,\ndas in ihren Armen wirkt.",
    "Wale nutzen Infraschall,\nderen Toene sich ueber viele km\nim Ozean ausbreiten.",
    "Axolotl regenerieren Gliedmaßen,\nwas sie zu interessanten\nForschungsobjekten macht.",
    "Pinguine speichern Waerme,\nindem sie ihre Federn eng anlegen\nund den Waermeverlust minimieren.",
    "Die DNA vieler Tiere zeigt erstaunliche\nGemeinsamkeiten, die evolutionaere\nVerwandtschaften offenbaren.",
    "Schmetterlinge haben ausgekluegelte\nFarberkennungssysteme",
    "Menschen und Bananen\nteilen etwa 60% ihrer Gene,\nein Hinweis auf geteilte Wurzeln.",
    "Quallen haben uralte Gene,\ndie sich kaum veraendert haben.",
    "Koalas haben\nfingerabdruckartige Rillen,\naehnlich wie Menschen.",
    "Haie und Rochen\nteilen genetische Wurzeln\nund gehoeren zur Knorpelfisch-Familie.",
    "Every move on the board\nis a rebellion; no piece is sacred,\nand the king is just another target.",
    "In chess, chaos is art,\neach pawn a revolutionary spark,\nevery check a call to arms.",
    "Google en passant\n\n                          holy hell!",
    "Als die Dinosaurier existierten,\ngab es Vulkane, die auf dem Mond\nausbrachen.",
    "Die einzigen Buchstaben, die\nnicht im Periodensystem vorkommen,\nsind 'J' und 'Q'.",
    "Wenn ein Eisbaer und ein\nGrizzlybaer sich paaren,\nwird 'Pizzy Bear' genannt.",
    "Daniel Radcliffe war allergisch gegen\nseine Harry-Potter-Brille,\ndoch Harry Potter traegt sie.",
    "Im Englischen heißt es 'French Exit',\nwenn man ohne Abschied geht",
    "In Arizona kann das Faellen eines\nSaguaro-Kaktus als Verbrechen\ngeahndet werden.",
    "Der in Statuen gezeigte Buddha\nist nicht der wahre Buddha;\nder echte war mager durch Askese.",
    "Ein einzelner Spaghetti-Strang\nwird als 'Spaghetto' bezeichnet,\neine kuriose Tatsache.",
    "Princess Peach blieb still\nbis 1988, da Designer\nsie nicht beweglich machten.",
    "Der erste Film mit Soundtrack\nwar Schneewittchen\nund die sieben Zwerge.",
    "Reichst du mit deinen Autoschluesseln\nan deinen Kopf, erhoeht sich\ndie Reichweite der Fernbedienung.",
    "Fruchtaufkleber sind essbar,\nwie auch das Obst selbst;\nVor dem Verzehr waschen!",
    "Der Name des Riesenameisbers\nist Myrmecophaga Tridactyla,\nwas 'Ameisenessend mit 3 Fingern' heißt.",
    "Das Wort 'Astronaut' kommt\naus dem Griechischen 'astro' = Stern,\nund 'naut' heißt Seefahrer."};
