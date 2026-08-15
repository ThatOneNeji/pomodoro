const fs = require('node:fs');
const path = require('node:path');
const postcss = require('postcss');
const purgecss = require('@fullhuman/postcss-purgecss');
const {minify} = require('html-minifier-terser');

async function buildPage(htmlFile) {
    let html = fs.readFileSync(htmlFile, 'utf8');

    // 1. Stash <pre> blocks so their content doesn't interfere with regex
    const preBlocks = [];
    html = html.replace(/<pre[\s\S]*?<\/pre>/gi, (match) => {
        preBlocks.push(match);
        return `__PRE_BLOCK_${preBlocks.length - 1}__`;
    });

    // 2. Find all <link> tags that are stylesheets
    const linkRe = /<link[^>]*href=["']([^"']+)["'][^>]*>/gim;
    let match;
    const sheets = [];
    while ((match = linkRe.exec(html)) !== null) {
        const fullTag = match[0];
        const href = match[1];
        // Only process actual stylesheets, skip icons, canonical links, etc.
        if (!fullTag.includes('stylesheet')) continue;
        // Skip external URLs (http/https)
        if (/^https?:\/\//i.test(href)) continue;
        sheets.push({tag: fullTag, href});
    }

    // 3. Purge unused CSS and inline each local stylesheet
    for (const {tag, href} of sheets) {
        const cssPath = path.resolve(path.dirname(htmlFile), href);
        if (!fs.existsSync(cssPath)) {
            console.warn(`Skipping missing file: ${cssPath}`);
            continue;
        }

        const rawCss = fs.readFileSync(cssPath, 'utf8');
        const purged = await postcss([
            purgecss({content: [htmlFile]})
        ]).process(rawCss, {from: cssPath});

        html = html.replace(tag, `<style>${purged.css}</style>`);
        console.log(`Inlined: ${href} (${rawCss.length} → ${purged.css.length} bytes)`);
    }

    // 4. Restore <pre> blocks before minification
    html = html.replace(/__PRE_BLOCK_(\d+)__/g, (_, i) => preBlocks[i]);

    // 5. Minify
    const minified = await minify(html, {
        collapseWhitespace: true,
        removeComments: true,
        minifyCSS: true,
        minifyJS: true,
    });

    const outFile = htmlFile.replace(/\.html$/, '.min.html');
    fs.writeFileSync(outFile, minified);
    console.log(`\nDone! ${htmlFile}: ${html.length} bytes → ${outFile}: ${minified.length} bytes`);
}

buildPage('./html/index.html');
buildPage('./html/settings.html');
