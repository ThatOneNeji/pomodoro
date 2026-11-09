# Third-party notices

This project is licensed under the GPL-3.0 (see `LICENSE` and the License section of `README.md`). It builds on the following third-party work, under their own licenses.

## Original project

| Work | License | Notes |
| --- | --- | --- |
| [Rukenshia/pomodoro](https://github.com/Rukenshia/pomodoro): the firmware, web page and case this repository is forked from | GPL-3.0 | Licensed upstream on 2026-11-08 ([issue #4](https://github.com/Rukenshia/pomodoro/issues/4)). Includes the case STLs in `cad/original/`, from the designer's [Onshape document](https://cad.onshape.com/documents/06055e629740267835bb7660/w/df56eb93ab74e2f4d61e5097/e/21a7853695e4900200750891). |

## Libraries (downloaded by PlatformIO at build time, not stored in this repository)

| Library | License |
| --- | --- |
| [GxEPD2](https://github.com/ZinggJM/GxEPD2) | GPL-3.0 |
| [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library) | BSD |
| [Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO) | MIT |
| [ESPAsyncWebServer](https://github.com/ESP32Async/ESPAsyncWebServer) | LGPL-3.0 |
| [AsyncTCP](https://github.com/ESP32Async/AsyncTCP) | LGPL-3.0 |
| [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) | LGPL-3.0 |
| [ArduinoJson](https://github.com/bblanchon/ArduinoJson) | MIT |
| [PubSubClient](https://github.com/knolleary/pubsubclient) | MIT |
| [ESP32Encoder](https://github.com/madhephaestus/ESP32Encoder) | See its repository (no license file in the downloaded package) |
| [Unity](https://github.com/ThrowTheSwitch/Unity) (host tests only) | MIT |

## Assets stored in this repository

| Asset | License | Notes |
| --- | --- | --- |
| [Bootstrap Icons](https://github.com/twbs/icons) v1.13.2, in `assets/svg/` and the icons generated from them in `src/icons/` | MIT | Copyright (c) 2019-2024 The Bootstrap Authors; full text in `assets/svg/LICENSE`. |
| [Funnel Display](https://fonts.google.com/specimen/Funnel+Display), as bitmap fonts generated into `src/fonts/` | SIL Open Font License 1.1 | The `.ttf` sources aren't stored here. |
| HelvetiPixel, as bitmap fonts generated into `src/fonts/` | **Unknown** | Source and license not yet found. Until they are, it may need replacing with a font under a known license. |
| Meme / "LPE" art: `assets/icons/lpe*.png`, and `bg_cat`, `bg_lpe_bubble`, `bg_pablo`, `bg_stonks` and `bg_what_a_week` in `assets/images/` | **Not licensed** | Meme art from the original project, some of it third-party works (e.g. a film still and a comic panel), which the GPL can't cover. Only built in with `INCLUDE_MEME_ASSETS`, off by default; removing them is on the roadmap ("Drop LPE entirely"). |
