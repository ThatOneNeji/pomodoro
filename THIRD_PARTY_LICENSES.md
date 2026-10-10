# Third-Party Licenses

## Bootstrap Icons

The full Bootstrap Icons SVG set is included under `assets/svg/`, from
[Bootstrap Icons](https://github.com/twbs/icons), used here under the MIT License. The icons
compiled into the firmware are listed in `assets/manifest.json`; currently:

- `code-slash`, `bullseye`, `tools`, `stopwatch` (preset and start screen icons)
- `bell-fill`, `chat-quote-fill`, `check-lg`, `cup-hot-fill`

`scripts/gen_assets.py` rasterizes them into the bitmaps in `src/icons/icons.cpp`. The web UI's
icons in `html/icons/` are from the same set.

```
The MIT License (MIT)

Copyright (c) 2019-2024 The Bootstrap Authors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```
