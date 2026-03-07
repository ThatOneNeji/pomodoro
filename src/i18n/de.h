#pragma once

struct Msgs {
    static constexpr const char *OPEN = "Open";
    static constexpr const char *CLOSE = "Close";
    static constexpr const char *STOP = "Stop";
    static constexpr const char *STATISTICS = "Statistics";
    static constexpr const char *INTOTAL = "In total";
    static constexpr const char *DONE = "Done";
    static constexpr const char *BREAKOVER = "Break Over";
    static constexpr const char *RESUME = "Resume";
    static constexpr const char *BREAKNOW = "Break now";
    static constexpr const char *SKIPBREAK = "Skip Break";
    static constexpr const char *TOSELECT = "To Select";

    static constexpr std::array<const char *, 4> BREAK_MESSAGES = {"Break time!", "Take a rest", "Time to relax",
                                                                   "Well done!"};
};

static const std::vector<const char *> genericPresetMessages = {
    "Okaaay, let's go!",
    "Heute schon auf Reddit gewesen?",
    "Bist du hydriert?",
    "Vielleicht eine kleine Kaffeepause?",
    "Langsam ist aber auch Zeit fuer\nFeierabend, oder nicht?",
    "Schoen hier, aber warst du heute\nschon auf Reddit?",
    "Du schaffst das (hoffen wir)",
    "Schauen wir mal was wird\n\n                              was wird",
};

static const std::vector<const char *> genericStartBreakMessages = {"Break time!", "Take a rest", "Time to relax",
                                                                    "Well done!"};

static const char *gINTOTAL = "Gesamt";
static const char *gSTATISTICS = "Statistik";

static const std::vector<const char *> chatGptFacts = {
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
