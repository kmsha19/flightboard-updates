#!/usr/bin/env node
'use strict';

// Build-only dependency. No JavaScript is fetched or compiled on the ESP32.
// Preserve all HTML/CSS and external scripts byte for byte; transform only the
// one classic inline application script. Top-level names and properties remain
// unchanged. Syntax optimization and tree shaking are deliberately disabled.
const fs = require('fs');
const vm = require('vm');
const EXPECTED_ESBUILD_VERSION = '0.28.2';
const OPTIONS = Object.freeze({
  loader: 'js',
  target: 'esnext',
  charset: 'utf8',
  legalComments: 'inline',
  minifyWhitespace: true,
  minifyIdentifiers: true,
  minifySyntax: false,
  treeShaking: false,
  logLevel: 'silent'
});

function applicationScript(html) {
  if (typeof html !== 'string') throw new Error('Expected a UTF-8 HTML string');
  const elements = [...html.matchAll(/<script\b([^>]*)>([\s\S]*?)<\/script\s*>/gi)];
  // Refuse unsupported/malformed markup instead of silently shipping raw JS.
  if (elements.length !== (html.match(/<script\b/gi) || []).length ||
      elements.length !== (html.match(/<\/script\s*>/gi) || []).length) {
    throw new Error('Malformed script elements in the embedded interface');
  }
  const inline = elements.filter(match => !/\bsrc\s*=/i.test(match[1]));
  if (inline.length !== 1 || inline[0][1].trim() !== '') {
    throw new Error('Expected exactly one classic inline application script without attributes');
  }
  const match = inline[0];
  const start = match.index + match[0].indexOf('>') + 1;
  return { start, end: start + match[2].length, code: match[2] };
}

function minifyWeb(html, compiler) {
  const script = applicationScript(html);
  const esbuild = compiler || require('esbuild');
  if (esbuild.version !== EXPECTED_ESBUILD_VERSION) {
    throw new Error(`Reproducible web build requires esbuild ${EXPECTED_ESBUILD_VERSION}; run npm ci`);
  }
  const result = esbuild.transformSync(script.code, OPTIONS);
  if (!result || typeof result.code !== 'string' || !result.code.trim() ||
      (result.warnings && result.warnings.length)) {
    throw new Error('Inline application minification failed or produced warnings');
  }
  // It remains a classic script, not a module or dynamically evaluated payload.
  new vm.Script(result.code, { filename: 'ZCS CONTROL embedded application' });
  if (/<\/script\b/i.test(result.code)) throw new Error('Unsafe closing script tag in generated JavaScript');
  return html.slice(0, script.start) + result.code + html.slice(script.end);
}

module.exports = { applicationScript, minifyWeb, OPTIONS, EXPECTED_ESBUILD_VERSION };

if (require.main === module) {
  try {
    if (process.argv.length !== 2) throw new Error('Usage: node tools/minify_web.cjs < interface.html > interface.min.html');
    const html = fs.readFileSync(0, 'utf8');
    process.stdout.write(minifyWeb(html));
  } catch (error) {
    process.stderr.write(`Embedded web build failed: ${error.message}\n`);
    process.exitCode = 1;
  }
}
