#!/usr/bin/env node
'use strict';
// Verify the shipped build without replacing the readable source used by the
// behavior suites. HTML, DOM attributes, CSS, and external script URLs must not
// change when the inline JavaScript is compressed for the ESP32.
const assert = require('assert');
const fs = require('fs');
const path = require('path');
const vm = require('vm');
const zlib = require('zlib');
const crypto = require('crypto');
const child = require('child_process');
const root = path.resolve(__dirname, '../..');
const { applicationScript, minifyWeb, EXPECTED_ESBUILD_VERSION } = require('../../tools/minify_web.cjs');
let checks = 0;
function equal(actual, expected, message) { checks++; assert.deepStrictEqual(actual, expected, message); }
function ok(value, message) { checks++; assert.ok(value, message); }
function rejects(fn, pattern, message) { checks++; assert.throws(fn, pattern, message); }
const sourcePath = path.join(root, 'FlightBoardWeb.h');
const sourceBytes = fs.readFileSync(sourcePath);
const html = sourceBytes.toString('utf8').match(/R"FBPAGE\(([\s\S]*?)\)FBPAGE"/)[1];
const source = applicationScript(html);
const minified = minifyWeb(html);
const built = applicationScript(minified);
equal(minified.slice(0, built.start), html.slice(0, source.start), 'DOM, CSS and external scripts before JS are unchanged');
equal(minified.slice(built.end), html.slice(source.end), 'DOM after JS is unchanged');
equal(minifyWeb(html), minified, 'Repeated build is deterministic');
equal(minifyWeb(html), minified, 'Third build is deterministic');
ok(Buffer.byteLength(minified) < Buffer.byteLength(html), 'Minification reduces embedded bytes');
new vm.Script(built.code, { filename: 'shipped inline app' }); checks++;
equal(fs.readFileSync(sourcePath), sourceBytes, 'Readable source is never overwritten');
equal(JSON.parse(fs.readFileSync(path.join(root, 'package.json'))).devDependencies.esbuild, EXPECTED_ESBUILD_VERSION, 'Build dependency is exactly pinned');
equal(JSON.parse(fs.readFileSync(path.join(root, 'package-lock.json'))).packages['node_modules/esbuild'].version, EXPECTED_ESBUILD_VERSION, 'Lockfile pins the same compiler');

const cli = child.spawnSync(process.execPath, [path.join(root, 'tools/minify_web.cjs')], { input: html, encoding: 'utf8' });
equal(cli.status, 0, 'Build CLI succeeds');
equal(cli.stdout, minified, 'CLI produces only the exact build artifact');
equal(cli.stderr, '', 'Successful build has no diagnostics');

// Preserve explicit exports, external references, property names, UTF-8 strings
// and template interpolation; application globals become IIFE-local names.
const fixture = '<!doctype html><style>.x{color:#fff}</style><div id="luminosité"></div><script src="/hash.js"></script><script>\n' +
  'function adjustNightBrightness(longBrightnessName){ const chosenBrightness = Math.min(60, longBrightnessName); return {nightBrightness:chosenBrightness, message:`Nuit : ${chosenBrightness} % ☾`}; }\n' +
  'var markup="<\\/script>";globalThis.result={adjustNightBrightness,markup,hash:sha256("abc")};\n</script><p>Après</p>';
const fixtureBuilt = minifyWeb(fixture);
const originalContext = {sha256:s=>'external-'+s}, minifiedContext = {sha256:s=>'external-'+s};
vm.runInNewContext(applicationScript(fixture).code, originalContext);
vm.runInNewContext(applicationScript(fixtureBuilt).code, minifiedContext);
for (const value of [0, 5, 20, 60, 100]) {
  equal(JSON.stringify(minifiedContext.result.adjustNightBrightness(value)), JSON.stringify(originalContext.result.adjustNightBrightness(value)), `Properties and values preserved at ${value}%`);
}
equal(minifiedContext.result.markup, originalContext.result.markup, 'A string containing a closing script tag remains safely escaped');
equal(minifiedContext.result.hash, 'external-abc', 'External SHA-256 dependency remains available inside the IIFE');
equal(minifiedContext.adjustNightBrightness, undefined, 'Application declarations stay inside the IIFE');
rejects(() => minifyWeb('<button onclick="start()">Start</button><script>function start(){}</script>'), /Inline HTML/, 'Inline handlers cannot silently lose access to global functions');
rejects(() => minifyWeb('<html></html>'), /exactly one/, 'Missing app script fails closed');
rejects(() => minifyWeb('<script>var x=1;</script><script>var y=2;</script>'), /exactly one/, 'Multiple app scripts fail closed');
rejects(() => minifyWeb('<script type="module">var x=1;</script>'), /classic inline/, 'Unexpected script type fails closed');
rejects(() => minifyWeb('<script>var x=1;'), /Malformed/, 'Malformed script markup fails closed');
rejects(() => minifyWeb('<script>function {</script>'), /Transform failed/, 'JavaScript parse failure stops the build');
rejects(() => minifyWeb(fixture, { version: '0.0.0' }), /requires esbuild/, 'Unexpected compiler version fails closed');
rejects(() => minifyWeb(fixture, { version: EXPECTED_ESBUILD_VERSION, transformSync() { return { code: 'var x=1;', warnings: [{ text: 'warning' }] }; } }), /warnings/, 'Compiler warnings stop the build');
const invalidCli = child.spawnSync(process.execPath, [path.join(root, 'tools/minify_web.cjs')], { input: '<html></html>', encoding: 'utf8' });
equal(invalidCli.status, 1, 'CLI failure has a nonzero exit status');
equal(invalidCli.stdout, '', 'Failed CLI never emits a partial build');

if (!process.argv.includes('--source-only')) {
  const header = fs.readFileSync(path.join(root, 'FlightBoardWebGzip.h'), 'utf8');
  const compressed = fs.readFileSync(path.join(root, 'assets/zcs-web.bin'));
  const manifest = JSON.parse(fs.readFileSync(path.join(root, 'assets/zcs-web.json')));
  equal(manifest.size, compressed.length, 'Web manifest size matches the artifact');
  equal(manifest.sha256, crypto.createHash('sha256').update(compressed).digest('hex'), 'Web manifest fingerprint matches the artifact');
  ok(header.includes('ZCS_WEB_SIZE = '+compressed.length+';') && header.includes(manifest.sha256), 'Firmware pins the exact downloaded content');
  ok(!header.includes('FLIGHT_BOARD_HTML_GZIP'), 'Full UI is never included in the firmware image');
  const served = zlib.gunzipSync(compressed).toString('utf8');
  equal(served, minified, 'Firmware gzip exactly equals the pinned transform of the current readable source');
  equal(compressed.readUInt32LE(4), 0, 'Gzip omits a build timestamp');
  equal(compressed[9], 255, 'Gzip has an OS-independent marker');
  new vm.Script(applicationScript(served).code, { filename: 'actual firmware gzip' }); checks++;
}
console.log(`Downloadable web build: ${checks} assertions passed; readable SHA-256 ${crypto.createHash('sha256').update(sourceBytes).digest('hex')}`);
