#pragma once
#include <Arduino.h>
#include <pgmspace.h>
// ZCS CONTRÔLE V4.1.2 — interface locale, aucun service ou asset externe.
const char FLIGHT_BOARD_HTML[] PROGMEM = R"FBPAGE(
<!doctype html>
<html lang="fr">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#182622">
<title>ZCS CONTRÔLE · V4.1.2</title>
<style>
:root{color-scheme:light;--bg:#f5f5f0;--card:#fff;--raised:#eef2ed;--line:#dfe4dd;--text:#202c29;--muted:#69766f;--accent:#19786a;--ink:#fff;--amber:#98661d;--error:#b24943;--green:#19786a;--rail:#182622}
*{box-sizing:border-box}html,body{margin:0;min-height:100%;background:var(--bg);color:var(--text);font-family:Inter,ui-sans-serif,system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif}body{font-size:14px;line-height:1.5}button,input,select,textarea{font:inherit}button,summary,select{touch-action:manipulation}button,summary{cursor:pointer}button{color:inherit}button:disabled,input:disabled,select:disabled{cursor:default;opacity:.45}button:focus-visible,input:focus-visible,textarea:focus-visible,select:focus-visible,summary:focus-visible,a:focus-visible{outline:3px solid #5cae96;outline-offset:3px}[hidden]{display:none!important}h1,h2,p{margin:0}a{color:var(--accent)}svg{flex:none}button{transition:background .16s,border-color .16s,box-shadow .16s}
.sidebar{position:fixed;inset:0 auto 0 0;width:208px;background:var(--rail);color:#c0cdc5;padding:34px 18px 22px;display:flex;flex-direction:column;z-index:3}.brand{display:flex;gap:11px;align-items:center;padding:0 8px}.zcsMark{width:37px;height:40px;display:grid;place-items:center;border-radius:11px;background:#b9efce;color:#182622;font-size:26px;font-weight:750;letter-spacing:-3px;padding-right:3px}.brandText{display:flex;flex-direction:column}.brandTitle{color:#fff;font-size:16px;font-weight:700;line-height:1.25;letter-spacing:-.5px}.brandCredit{font-size:9px;margin-top:5px;color:#a4b8ae}.navLabel{font-size:9px;letter-spacing:.14em;text-transform:uppercase;color:#83998e;margin:46px 13px 12px}.navItems{display:flex;flex-direction:column;gap:7px}.navButton{display:flex;align-items:center;gap:12px;text-align:left;border:0;border-radius:10px;background:transparent;color:#b8c9c0;min-height:46px;padding:11px 14px;font-size:12px;font-weight:550}.navButton:hover{background:#263b32;color:#fff}.navButton.active,.navButton[aria-current=page]{background:#b9efce;color:#192d23}.navIcon{width:19px;height:19px;fill:none;stroke:currentColor;stroke-width:1.6;stroke-linecap:round;stroke-linejoin:round}.sidebarFoot{margin-top:auto;padding:26px 10px 0;border-top:1px solid #314339}.deviceMark{display:flex;align-items:center;gap:10px;font-size:11px;color:#d0e0d7}.deviceMark svg{width:26px;height:26px;stroke:#92bda7;fill:none;stroke-width:1.4}.sidebarFoot small{display:block;font-size:9px;color:#94a99e;margin-top:5px}.version{display:inline-block;color:#94a99e;font-size:9px;border:1px solid #40574b;border-radius:5px;padding:2px 6px;margin-top:15px}.app{margin-left:208px;padding:0 36px 18px;min-height:100dvh;display:flex;flex-direction:column;gap:26px;max-width:1800px}.top{min-height:89px;display:flex;align-items:center;justify-content:space-between;border-bottom:1px solid var(--line);gap:16px}.topCaption{font-size:11px;color:var(--muted);letter-spacing:.03em}.topCaption strong{color:var(--text);font-weight:600}.tools{display:flex;align-items:center;gap:6px}.connection{display:flex;align-items:center;gap:7px;font-size:11px;color:var(--muted);margin-right:15px;white-space:nowrap}.dot{width:6px;height:6px;flex:none;border-radius:50%;background:var(--amber)}.btn{min-height:42px;border:1px solid var(--line);border-radius:9px;padding:9px 14px;background:var(--card);font-size:11px;font-weight:600;line-height:1.3;display:inline-flex;align-items:center;justify-content:center;gap:7px}.btn:hover:not(:disabled){background:var(--raised);border-color:#bbc9c0}.btn.primary,.btn.active{background:var(--accent);color:var(--ink);border-color:var(--accent)}.btn.primary:hover:not(:disabled),.btn.active:hover:not(:disabled){background:#125c51;border-color:#125c51}.btn.quiet{border-color:transparent;background:transparent;color:var(--muted)}.btn svg{width:18px;height:18px;fill:none;stroke:currentColor;stroke-width:1.6;stroke-linecap:round;stroke-linejoin:round}.iconBtn{width:40px;padding:9px}.eyebrow{font-size:10px;letter-spacing:.12em;text-transform:uppercase;font-weight:600;color:var(--muted)}
.hub{display:flex;flex-direction:column;gap:25px;flex:1}.hubIntro{display:flex;align-items:flex-end;justify-content:space-between;gap:24px}.hubIntro h1{font-size:36px;line-height:1.2;font-weight:600;letter-spacing:-1.5px}.hubIntro p{color:var(--muted);font-size:12px;margin-top:10px}.librarySearch{display:flex;align-items:center;gap:8px;border:1px solid var(--line);border-radius:10px;background:var(--card);padding:0 12px;min-width:230px}.librarySearch svg{width:16px;height:16px;stroke:var(--muted);fill:none;stroke-width:1.7}.librarySearch input{width:100%;min-width:0;max-width:230px;height:44px;background:transparent;border:0;outline:0;color:var(--text);font-size:11px}.librarySearch:focus-within{outline:2px solid var(--accent);outline-offset:2px}.librarySearch input:focus-visible{outline:0}.hubNow{display:grid;grid-template-columns:1fr 1.1fr;align-items:center;gap:26px;padding:26px 30px;border-radius:18px;background:#1c3029;color:#f0f5ee;min-height:195px;box-shadow:0 8px 22px #21372908}.liveLabel{display:flex;align-items:center;gap:7px;color:#b9efce;font-size:9px;letter-spacing:.11em;text-transform:uppercase;font-weight:600}.liveLabel:before{content:'';height:6px;width:6px;border-radius:50%;background:#b9efce;box-shadow:0 0 0 4px #b9efce12}.hubNowTitle{font-size:25px;font-weight:500;letter-spacing:-.6px;margin-top:14px}.hubNow .smallPrint{color:#a8bdb0;font-size:11px;max-width:310px}.hubNow .preview{border:0;padding:0}.hubNow .preview summary{display:none}.hubNow .previewBody{padding:0;gap:20px}.hubNow #ledPreview{width:256px;max-width:70%;border:8px solid #112019;border-radius:8px;box-shadow:0 6px 20px #0003}.hubNow .previewInfo{color:#a8bdb0;font-size:10px}.hubNow .previewInfo b{color:#e6eee8;font-size:11px}.libraryToolbar{display:flex;justify-content:space-between;align-items:center;gap:14px}.libraryFilters{display:flex;gap:5px;flex-wrap:wrap}.libraryFilter{border:1px solid transparent;border-radius:8px;background:transparent;color:var(--muted);padding:9px 12px;font-size:11px;min-height:38px;font-weight:550}.libraryFilter:hover{background:#e7ece5}.libraryFilter.active,.libraryFilter[aria-pressed=true]{background:#e0ece2;border-color:#d1e2d5;color:#255540}.libraryCount{font-size:10px;color:var(--muted);white-space:nowrap}.libraryGrid{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:20px}.libraryCard{min-width:0;border:1px solid var(--line);border-radius:14px;background:var(--card);overflow:hidden;position:relative;transition:border-color .16s,box-shadow .16s}.libraryCard:hover{border-color:#b5c9bc;box-shadow:0 6px 22px #243e3010}.cardArtwork{background:#192d26;position:relative;display:flex;justify-content:center;align-items:center;aspect-ratio:1.9;overflow:hidden;padding:22px;isolation:isolate}.cardArtwork:after{content:'';position:absolute;inset:0;background:linear-gradient(135deg,#ffffff08,transparent);pointer-events:none}.cardCanvas,.cardArtwork canvas{display:block;width:100%;height:auto;aspect-ratio:2;image-rendering:pixelated;border-radius:5px;max-width:300px;box-shadow:0 6px 15px #0002}.cardInfo{padding:18px 18px 15px}.cardCategory{font-size:8px;color:var(--accent);text-transform:uppercase;letter-spacing:.13em;font-weight:650}.cardName{font-size:16px;font-weight:600;letter-spacing:-.35px;line-height:1.4;margin:5px 0}.cardDescription{font-size:11px;line-height:1.6;color:var(--muted);min-height:35px}.cardActions{display:flex;align-items:center;justify-content:space-between;gap:8px;margin-top:14px;padding-top:12px;border-top:1px solid #edf0eb}.libraryOpen{display:inline-flex;align-items:center;justify-content:center;border:0;border-radius:6px;color:var(--accent);background:#eaf2ec;min-height:34px;padding:7px 12px;font-size:10px;font-weight:650}.libraryOpen:hover{background:#d9eade}.libraryFavorite{height:34px;width:34px;border:0;border-radius:7px;background:transparent;color:#748279;font-size:19px;display:grid;place-items:center}.libraryFavorite:hover,.libraryFavorite[aria-pressed=true]{color:#8b642c;background:#f7efdf}.libraryFavorite svg{width:17px;height:17px;fill:none;stroke:currentColor;stroke-width:1.5}.libraryFavorite[aria-pressed=true] svg{fill:currentColor}.libraryLiveBadge{position:absolute;z-index:1;top:11px;left:11px;font-size:8px;letter-spacing:.03em;border-radius:5px;padding:4px 7px;background:#c9f4d8;color:#214734}.libraryEmpty{border:1px dashed #bccbc0;border-radius:14px;padding:45px 24px;text-align:center;color:var(--muted);font-size:12px}.bottom{display:flex;justify-content:space-between;align-items:center;gap:15px;margin-top:auto;padding-top:12px;min-height:28px}.statusLine,.source{font-size:10px;color:var(--muted)}.source{white-space:nowrap}
.moduleBar{display:flex;align-items:center;justify-content:space-between;gap:12px}.moduleBarLeft{display:flex;align-items:center;gap:15px}.moduleTitle{font-size:14px;font-weight:600}.moduleActivate{min-width:170px}.moduleView{display:grid;grid-template-columns:1.1fr 1fr;gap:20px;flex:1;min-height:480px}.card{background:var(--card);border:1px solid var(--line);border-radius:16px;min-width:0;overflow:hidden}.moduleHero,.moduleSide{padding:32px;display:flex;flex-direction:column;gap:24px}.moduleHero{justify-content:center}.moduleHero h1{font-size:35px;letter-spacing:-1px;font-weight:550}.moduleSide{justify-content:space-between}.cardTitle{font-size:13px;font-weight:600;letter-spacing:-.1px}.moduleLead{color:var(--muted);font-size:12px;max-width:380px;line-height:1.75}.weatherReading{display:flex;align-items:center;gap:24px;margin:14px 0}.weatherTemperature{font-size:clamp(70px,8vw,106px);font-weight:350;line-height:1;letter-spacing:-5px}.weatherIcon{font-size:60px;color:var(--amber);line-height:1}.weatherCondition{font-size:18px;letter-spacing:-.4px;margin-bottom:22px}.weatherMeta{display:flex;gap:24px;align-items:center;border-top:1px solid var(--line);padding-top:20px;color:var(--muted);font-size:11px}.weatherMeta strong{color:var(--text);display:block;font-size:20px;font-weight:500;margin-top:4px}.clockFace{font-size:clamp(65px,7vw,100px);font-weight:350;font-variant-numeric:tabular-nums;letter-spacing:-5px;line-height:1.3}.visualChoices{display:grid;grid-template-columns:1fr 1fr;gap:9px;margin-top:20px}.visualChoices .btn{min-height:48px;justify-content:flex-start;padding:10px;font-size:10px}.visualDot{width:11px;height:11px;border-radius:4px;background:#cb8054;flex:none}.visualChoices [data-visual=aurora] .visualDot{background:linear-gradient(135deg,#4ed6b6,#ad7fe6)}.visualChoices [data-visual=ocean] .visualDot{background:#4ab8e3}.visualChoices [data-visual=warp] .visualDot{background:#a297ff}.visualChoices [data-visual=stars] .visualDot{background:#6999da}.visualChoices [data-visual=rain] .visualDot{background:#55b48b}.visualChoices [data-visual=plain] .visualDot{background:#adb9ae}.moduleFeedback{font-size:11px;color:var(--amber);min-height:18px;margin-top:12px}.preview{border-top:1px solid var(--line);padding-top:16px}.preview summary{display:flex;align-items:center;gap:8px;list-style:none;font-size:10px;color:var(--muted);min-height:28px}.preview summary::-webkit-details-marker{display:none}.preview summary:before{content:'＋';font-size:14px;color:var(--accent)}.preview[open] summary:before{content:'−'}.preview summary span:last-child{margin-left:auto;font-size:9px}.previewBody{display:flex;align-items:center;gap:16px;padding-top:12px}.previewInfo{font-size:10px;color:var(--muted);line-height:1.65}.previewInfo b{color:var(--text);font-weight:500;display:block;margin-bottom:5px}#ledPreview{display:block;width:180px;max-width:55%;height:auto;aspect-ratio:2/1;background:#000;image-rendering:pixelated;flex:none;border-radius:4px}.moduleSide .preview{margin-top:auto}.moduleSide #ledPreview{width:220px}.aviationApp{display:flex;flex-direction:column;gap:22px;flex:1}.locationBar{display:flex;align-items:center;justify-content:space-between;gap:16px}.location{display:flex;align-items:center;gap:10px;min-width:0}.location h1{font-size:30px;letter-spacing:-1px;font-weight:550;line-height:1.3;overflow-wrap:anywhere}.placeMeta{font-size:11px;color:var(--muted);margin-top:5px}.locationPicker{display:flex;gap:8px;align-items:center;flex:none}.locationPicker select{border:1px solid var(--line);border-radius:9px;background:var(--card);color:var(--text);padding:10px;min-height:42px;max-width:180px;font-size:11px}.main{display:grid;grid-template-columns:minmax(0,1.4fr) minmax(290px,1fr);gap:18px;min-height:460px}.skyCard{display:flex;flex-direction:column}.cardHead{display:flex;align-items:center;justify-content:space-between;gap:12px;padding:22px 22px 0}.subtle{color:var(--muted);font-size:10px}.countBadge{background:#f0f3ee;font-size:10px;color:var(--muted);border-radius:6px;padding:5px 8px;white-space:nowrap}.skyBody{display:grid;grid-template-columns:minmax(0,1.2fr) minmax(132px,.8fr);gap:12px;padding:22px;flex:1;min-height:0}.radarBlock{display:flex;flex-direction:column;align-items:center;justify-content:center;gap:20px;min-width:0}.radar{width:min(100%,330px);aspect-ratio:1;position:relative;border:1px solid #426151;border-radius:50%;background:#1b3027;flex:none;overflow:hidden}.ring{position:absolute;left:50%;top:50%;transform:translate(-50%,-50%);border:1px solid #3a5143;border-radius:50%;pointer-events:none}.r1{width:33.33%;height:33.33%}.r2{width:66.66%;height:66.66%}.crossH,.crossV{position:absolute;background:#31483b;pointer-events:none}.crossH{height:1px;left:0;right:0;top:50%}.crossV{width:1px;top:0;bottom:0;left:50%}.center{position:absolute;left:50%;top:50%;transform:translate(-50%,-50%);border:2px solid #b9efce;border-radius:50%;width:7px;height:7px;pointer-events:none}.cardinal{position:absolute;font-size:8px;color:#a4c5b2;pointer-events:none}.north{left:calc(50% - 3px);top:8px}.south{left:calc(50% - 3px);bottom:8px}.west{left:8px;top:calc(50% - 6px)}.east{right:8px;top:calc(50% - 6px)}.planes{position:absolute;inset:0;pointer-events:none}.plane{position:absolute;transform:translate(-50%,-50%);pointer-events:auto;min-height:38px;min-width:38px;padding:3px;border:0;border-radius:6px;background:transparent;color:#b1cbbc;display:flex;flex-direction:column;align-items:center;gap:1px;line-height:1}.plane svg{width:22px;height:22px;fill:currentColor}.plane span{background:#1b3027ed;font-size:8px;border-radius:3px;padding:3px}.plane.selected{color:#f3cc83}.radarCaption{text-align:center;font-size:10px;color:var(--muted);min-height:34px}.radarCaption strong{display:block;color:var(--text);font-weight:500;font-size:11px;margin-bottom:4px}.nearby{display:flex;flex-direction:column;min-height:0;gap:10px}.listLabel{font-size:9px;color:var(--muted);text-transform:uppercase;letter-spacing:.08em}.flightList{overflow:auto;max-height:345px;min-height:0;flex:1;display:flex;flex-direction:column;gap:5px;scrollbar-width:thin;scrollbar-color:var(--line) transparent}.flightRow{display:flex;align-items:center;justify-content:space-between;gap:8px;text-align:left;width:100%;padding:11px 8px;border:1px solid transparent;border-radius:9px;background:transparent;min-height:65px;flex:none}.flightRow:hover{background:var(--raised)}.flightRow.selected{background:#e6f1e9;border-color:#c7ddcf}.flightRow strong{display:block;font-size:12px;font-weight:650;letter-spacing:.02em}.flightRow small{display:block;font-size:9px;color:var(--muted);margin-top:4px;max-width:125px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}.flightDistance{font-size:11px;white-space:nowrap;color:var(--accent)}.flightDistance small{font-size:8px;text-align:right}.listEmpty{font-size:11px;color:var(--muted);padding:20px 0;line-height:1.6}.flightCard{display:flex;flex-direction:column;padding:22px;gap:22px}.flightCard .cardHead{padding:0}.details{display:flex;flex-direction:column;gap:24px;flex:1;justify-content:center}.flightHeader{display:flex;align-items:center;gap:14px}.logoFrame{background:#13241d;border-radius:10px;width:70px;height:54px;display:grid;place-items:center;flex:none}.logoFrame canvas{width:60px;height:40px;image-rendering:pixelated}.logoFrame img{width:60px;height:40px;object-fit:contain}.logoFrame svg{display:block;width:28px;height:28px;fill:#89a695}.callsign{font-size:29px;line-height:1.1;letter-spacing:-1px;font-weight:600;overflow-wrap:anywhere}.airline,.type{font-size:10px;color:var(--muted);margin-top:7px}.type{margin-top:4px}.stats{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:10px;border-block:1px solid var(--line);padding:18px 0}.statLabel{font-size:8px;color:var(--muted);text-transform:uppercase;letter-spacing:.07em}.statValue{font-size:23px;font-weight:500;letter-spacing:-.6px;margin-top:4px;line-height:1.3}.statUnit{font-size:9px;color:var(--muted);margin-top:2px}.empty{padding:24px 4px;flex:1;display:flex;flex-direction:column;justify-content:center;gap:8px;color:var(--muted);font-size:11px}.empty strong{font-size:23px;color:var(--text);font-weight:500;letter-spacing:-.5px}.modeBlock{margin-top:auto}.modeLabel{font-size:10px;color:var(--muted);margin-bottom:10px}.modeButtons{display:grid;grid-template-columns:1fr 1.4fr 1fr;gap:6px}.modeButtons .btn{font-size:10px;padding:10px 4px;white-space:nowrap}.route{min-width:0}.routeAirports{display:grid;grid-template-columns:minmax(0,1fr) 24px minmax(0,1fr);align-items:center;gap:8px}.routeAirport{min-width:0}.routeAirport:last-child{text-align:right}.routeCode{font-size:22px;letter-spacing:.04em;font-weight:550;line-height:1.2}.routeCity{display:block;font-size:10px;color:var(--muted);margin-top:5px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}.routeArrow{text-align:center;font-size:18px;color:var(--accent)}.routeNote{font-size:9px;color:var(--muted);margin-top:8px}.filterButton{min-height:34px;padding:5px 10px;font-size:10px}.filterHead{display:flex;align-items:center;gap:8px}.filterNotice{font-size:10px;color:var(--muted);padding:12px 22px 0}.demoBadge{color:var(--amber);font-size:10px;white-space:nowrap}.demoNotice{display:none;padding:10px 14px;border:1px solid #dac8a7;border-radius:9px;color:var(--amber);font-size:11px;background:#fff9ed}.demoNotice.visible{display:block}
.dialog,.wifiDialog{color:var(--text);background:var(--card);border:1px solid var(--line);border-radius:20px;width:min(620px,calc(100% - 28px));max-height:90dvh;overflow:auto;padding:28px;box-shadow:0 24px 80px #142a2630}.dialog::backdrop,.wifiDialog::backdrop{background:#182a266e;backdrop-filter:blur(5px)}.dialogHead,.wifiHead{display:flex;align-items:center;justify-content:space-between;gap:14px;margin-bottom:10px}.dialogTitle{font-size:25px;font-weight:550;letter-spacing:-.7px}.dialogIntro,.wifiHelp,.warn{font-size:12px;color:var(--muted);line-height:1.75}.dialogIntro{margin-bottom:24px}.section{border-top:1px solid var(--line);padding:18px 0}.section summary{font-size:12px;font-weight:600;min-height:25px}.section summary::marker{color:#8ca696}.sectionBody{padding:3px 0 5px}.field{display:flex;flex-direction:column;gap:7px;font-size:11px;color:var(--muted);margin-top:16px}.field input,.field select,.field textarea{background:#f7f8f4;color:var(--text);padding:11px 12px;border:1px solid var(--line);border-radius:9px;min-height:44px;width:100%;min-width:0}.field input[type=range]{padding:9px 0;accent-color:var(--accent);border:0;background:transparent}.field:has(input[type=range])>span{color:var(--accent);font-size:18px;font-weight:550;font-variant-numeric:tabular-nums}.formGrid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:0 16px}.check,.wifiCheck{display:flex;align-items:center;gap:10px;font-size:12px;color:var(--text);margin:16px 0}.check input,.wifiCheck input{width:19px;height:19px;flex:none;accent-color:var(--accent)}.dialogActions{display:flex;gap:8px;justify-content:flex-end;margin-top:20px;flex-wrap:wrap}.dialogFeedback{font-size:11px;min-height:18px;color:var(--amber);margin-top:12px}.favoriteAdd{display:flex;gap:8px;align-items:flex-end}.favoriteAdd .field{flex:1}.favoriteList{margin-top:12px;display:flex;flex-direction:column;gap:5px}.favoriteItem{display:flex;align-items:center;justify-content:space-between;gap:10px;font-size:12px;padding:6px 0}.favoriteItem small{color:var(--muted);margin-left:8px}.favoriteItem .btn{min-height:36px;font-size:10px}.serviceButtons{display:flex;flex-wrap:wrap;gap:8px;margin-top:12px}.wifiStatus{border:1px solid var(--line);background:var(--bg);border-radius:10px;padding:14px;font-size:12px;line-height:1.6;margin:14px 0;overflow-wrap:anywhere}.wifiStatus[data-tone=success]{color:var(--accent);border-color:#a5c5b5}.wifiStatus[data-tone=error]{color:var(--error);border-color:#d6aaa7}.wifiScanRow{display:flex;align-items:flex-end;gap:8px}.wifiScanRow .field{flex:1;min-width:0}.wifiHelp{margin:8px 0;overflow-wrap:anywhere}.wifiRescue{border:1px solid var(--line);border-radius:10px;padding:14px;margin-top:16px}.wifiRescue p{margin:6px 0}.wifiRescue code{color:var(--text);user-select:all}.uploadInfo{padding:14px;background:var(--bg);border-radius:10px;font-size:12px;color:var(--muted);line-height:1.7;margin:16px 0}.uploadInfo strong{color:var(--text);font-weight:550}progress{display:block;width:100%;height:8px;accent-color:var(--accent);margin:16px 0}.uploadFile{overflow-wrap:anywhere}.smallPrint{font-size:10px;color:var(--muted);line-height:1.7;margin-top:10px}
.libraryCard.isLive{border-color:#7caf95;box-shadow:0 0 0 1px #7caf951a}.cardArtwork .libraryFavorite{position:absolute;z-index:2;right:9px;top:9px;color:#d9e9e0;background:#12271dc4}.cardArtwork .libraryFavorite:hover,.cardArtwork .libraryFavorite[aria-pressed=true]{color:#d0f0bf;background:#31513e}.libraryActivate{border:1px solid #d4e6d9;border-radius:7px;background:#e7f3e9;color:#235a3d;min-height:35px;padding:8px 12px;font-size:10px;font-weight:650}.libraryActivate:hover:not(:disabled){background:#cde6d5}.cardActions .libraryOpen{padding-left:0;background:transparent;color:var(--muted)}.cardActions .libraryOpen:hover{color:var(--accent)}
.libraryShelf{display:flex;align-items:center;gap:12px;flex-wrap:wrap}.libraryShelf .smallPrint{margin:0 auto 0 0}.field textarea{resize:vertical;min-height:90px}.field input[type=color]{max-width:110px;padding:5px}.widgetStatus{font-size:12px;color:var(--muted)}
@media(min-width:1550px){.app{padding-inline:50px}.libraryGrid{grid-template-columns:repeat(4,minmax(0,1fr))}}
@media(max-width:1190px){.sidebar{width:76px;padding:26px 12px}.brand{padding:0;justify-content:center}.brandText,.navLabel,.navButton span,.sidebarFoot{display:none}.navItems{margin-top:48px;gap:14px}.navButton{justify-content:center;padding:12px;min-height:48px}.navIcon{width:21px;height:21px}.app{margin-left:76px;padding-inline:28px}.main{grid-template-columns:minmax(0,1.25fr) minmax(285px,1fr)}.skyBody{padding:18px;gap:8px;grid-template-columns:minmax(0,1fr) minmax(125px,.8fr)}.flightRow small{max-width:100px}.moduleHero,.moduleSide{padding:26px}.cardInfo{padding:16px}.libraryGrid{gap:16px}}
@media(max-width:950px){.app{padding-inline:24px;gap:24px}.libraryGrid{grid-template-columns:repeat(2,minmax(0,1fr))}.hubIntro h1{font-size:32px}.hubIntro{gap:14px}.librarySearch{min-width:190px}.librarySearch input{max-width:180px}.hubNow{grid-template-columns:.85fr 1.15fr;gap:16px;padding:25px}.hubNowTitle{font-size:22px}.hubNow .previewBody{flex-direction:column;gap:10px;align-items:flex-start}.hubNow #ledPreview{max-width:100%}.hubNow .previewInfo b{display:none}.libraryToolbar{align-items:flex-start}.libraryFilter{padding-inline:9px;font-size:10px}.libraryCount{padding-top:11px}.moduleView,.main{grid-template-columns:1fr}.moduleHero,.moduleSide{min-height:330px}.moduleSide #ledPreview{width:256px}.skyBody{grid-template-columns:minmax(0,1.2fr) minmax(170px,.8fr);padding:24px;gap:24px;min-height:380px}.radar{width:min(100%,310px)}.flightCard{padding:25px}.flightList{max-height:300px}.flightRow small{max-width:160px}.statValue{font-size:27px}.callsign{font-size:32px}.weatherTemperature,.clockFace{font-size:100px}.locationPicker select{max-width:155px}.topCaption,.connection{font-size:10px}}
@media(max-width:620px){.sidebar{inset:auto 0 0;width:auto;height:72px;padding:6px 8px 8px;border-radius:16px 16px 0 0}.sidebar .brand{display:none}.navItems{margin:0;flex-direction:row;gap:3px;justify-content:space-around;height:100%}.navButton{flex:1;min-width:0;min-height:54px;flex-direction:column;gap:4px;padding:7px 3px}.navButton span{display:block;font-size:8px;white-space:nowrap}.navIcon{width:18px;height:18px}.app{margin:0;padding:0 16px 88px;gap:20px}.top{min-height:72px;gap:8px}.topCaption{font-size:9px;max-width:76px}.topCaption strong{display:block;font-size:11px}.topCaption span,.tools .demoLabel,.source{display:none}.tools{gap:0}.connection{font-size:9px;max-width:105px;white-space:normal;margin-right:4px}.tools .btn{min-height:36px;width:35px;padding:7px}.hubIntro{align-items:stretch;flex-direction:column;gap:16px}.hubIntro h1{font-size:30px}.librarySearch{min-width:0}.librarySearch input{max-width:none}.hub{gap:20px}.hubNow{grid-template-columns:1fr;gap:20px;padding:24px}.hubNow .previewBody{flex-direction:row;align-items:center;gap:18px}.hubNow #ledPreview{width:200px;max-width:63%}.libraryToolbar{flex-direction:column;gap:5px}.libraryFilters{gap:2px}.libraryCount{padding:0}.libraryGrid{gap:12px}.cardArtwork{padding:13px;aspect-ratio:1.65}.cardInfo{padding:12px}.cardCategory{font-size:7px}.cardName{font-size:14px}.cardDescription{font-size:10px;min-height:47px}.cardActions{gap:3px;margin-top:12px;padding-top:9px}.libraryOpen,.libraryActivate{font-size:9px;padding:7px}.moduleBar{gap:8px}.moduleBarLeft{gap:5px}.moduleBarLeft .btn{padding:8px;font-size:10px}.moduleTitle{font-size:12px}.moduleActivate{min-width:0;max-width:145px;font-size:10px}.moduleHero,.moduleSide{padding:24px}.weatherTemperature,.clockFace{font-size:78px}.moduleSide #ledPreview{width:200px}.locationBar{flex-direction:column;align-items:stretch;gap:15px}.locationPicker select{max-width:none;flex:1}.skyBody{padding:16px;gap:10px;grid-template-columns:minmax(0,1fr) minmax(125px,.8fr);min-height:310px}.plane{min-width:32px;min-height:32px}.plane svg{width:18px;height:18px}.plane span{font-size:7px}.flightRow small{max-width:90px}.flightList{max-height:260px}.cardHead{padding:18px 16px 0;gap:8px}.filterHead{gap:5px}.flightCard,.dialog,.wifiDialog{padding:22px}.bottom{align-items:flex-start}.wifiScanRow,.favoriteAdd{flex-direction:column;align-items:stretch}.wifiScanRow .btn{align-self:flex-start}}
@media(prefers-reduced-motion:reduce){*{scroll-behavior:auto!important;transition:none!important}}
.topCredit{display:block;margin-top:4px;font-size:9px;letter-spacing:0;color:var(--muted)}
</style>
</head>
<body>
<svg width="0" height="0" aria-hidden="true" style="position:absolute"><defs><symbol id="planeIcon" viewBox="-15 -15 30 30"><path d="M0-14 3-3 12 3 12 6 2 3 2 10 5 12 5 14 0 12-5 14-5 12-2 10-2 3-12 6-12 3-3-3Z"/></symbol></defs></svg>
<aside class="sidebar" aria-label="Navigation principale">
  <div class="brand"><span class="zcsMark" aria-hidden="true">Z</span><span class="brandText"><span class="brandTitle">ZCS CONTRÔLE</span><span class="brandCredit">by Christ_Kmusic</span></span></div>
  <p class="navLabel">Votre espace</p>
  <nav class="navItems">
    <button class="navButton active" type="button" data-nav="home" aria-current="page" aria-label="Bibliothèque" title="Bibliothèque"><svg class="navIcon" viewBox="0 0 24 24" aria-hidden="true"><rect x="3" y="3" width="7" height="7" rx="1.5"/><rect x="14" y="3" width="7" height="7" rx="1.5"/><rect x="3" y="14" width="7" height="7" rx="1.5"/><rect x="14" y="14" width="7" height="7" rx="1.5"/></svg><span>Bibliothèque</span></button>
    <button class="navButton" type="button" data-nav="aviation" aria-label="Flight Board" title="Flight Board"><svg class="navIcon" viewBox="0 0 24 24" aria-hidden="true"><path d="m3 10 8 2 7-8c3-2 4 0 2 2l-7 8 1 7-2 1-3-6-5-2Z"/></svg><span>Flight Board</span></button>
    <button class="navButton" type="button" data-nav="weather" aria-label="Météo" title="Météo"><svg class="navIcon" viewBox="0 0 24 24" aria-hidden="true"><path d="M9 16a5 5 0 0 1 0-10 5 5 0 0 1 5 4M9 2v1M2 9h1M4 4l1 1M15 4l-1 1"/><path d="M9 21a4 4 0 0 1 0-8h1a5 5 0 0 1 9-1 4.5 4.5 0 0 1-1 9Z"/></svg><span>Météo</span></button>
    <button class="navButton" type="button" data-nav="clock" aria-label="Horloge" title="Horloge"><svg class="navIcon" viewBox="0 0 24 24" aria-hidden="true"><circle cx="12" cy="12" r="9"/><path d="M12 6v6l4 2"/></svg><span>Horloge</span></button>
    <button class="navButton" type="button" data-nav="animation" aria-label="Animations" title="Animations"><svg class="navIcon" viewBox="0 0 24 24" aria-hidden="true"><path d="m12 3 2.6 6.4L21 12l-6.4 2.6L12 21l-2.6-6.4L3 12l6.4-2.6ZM20 2v4m-2-2h4"/></svg><span>Animations</span></button>
  </nav>
  <div class="sidebarFoot"><div class="deviceMark"><svg viewBox="0 0 28 28" aria-hidden="true"><rect x="2" y="5" width="24" height="16" rx="3"/><path d="M10 25h8m-4-4v4M7 10h2m4 0h2m4 0h2M7 15h2m4 0h2m4 0h2"/></svg><span>Votre panneau<small>128 × 64 pixels</small></span></div><span class="version">V4.1.2</span></div>
</aside>
<div class="app">
<header class="top">
  <p class="topCaption"><strong>ZCS CONTRÔLE</strong><small class="topCredit">by Christ_Kmusic</small></p>
  <div class="tools">
    <span class="connection"><span class="dot" id="dot"></span><span id="connection">Connexion…</span></span>
    <button class="btn quiet" id="demoBtn" type="button" aria-pressed="false" title="Activer la démonstration" aria-label="Activer la démonstration"><svg aria-hidden="true" viewBox="0 0 24 24"><path d="m8 5 11 7-11 7Z"/></svg><span class="demoLabel">Démo</span></button>
    <button class="btn quiet iconBtn" id="fullscreenBtn" type="button" aria-label="Passer en plein écran" title="Plein écran"><svg aria-hidden="true" viewBox="0 0 24 24"><path d="M8 3H3v5m13-5h5v5M3 16v5h5m13-5v5h-5"/></svg></button>
    <button class="btn quiet iconBtn" id="settingsBtn" type="button" aria-label="Ouvrir les réglages" title="Réglages" aria-haspopup="dialog" aria-controls="settingsDialog"><svg aria-hidden="true" viewBox="0 0 24 24"><path d="M4 7h16M4 17h16"/><circle cx="9" cy="7" r="3" fill="var(--bg)"/><circle cx="15" cy="17" r="3" fill="var(--bg)"/></svg></button>
  </div>
</header>
<nav class="moduleBar" id="moduleBar" hidden aria-label="Application ouverte"><div class="moduleBarLeft"><button class="btn quiet" id="homeBtn" type="button">‹ Bibliothèque</button><span class="moduleTitle" id="moduleTitle">Flight Board</span></div><button class="btn primary moduleActivate" id="activateBtn" type="button">Afficher sur le panneau</button></nav>
<section class="hub" id="hub" aria-labelledby="hubTitle">
  <div class="hubIntro"><div><h1 id="hubTitle">La bibliothèque.</h1><p id="librarySubtitle">Un écran. Toutes vos envies.</p></div><label class="librarySearch"><svg viewBox="0 0 20 20" aria-hidden="true"><circle cx="8" cy="8" r="5.5"/><path d="m12 12 5 5"/></svg><input id="librarySearch" type="search" placeholder="Rechercher…" aria-label="Rechercher dans la bibliothèque" autocomplete="off"></label></div>
  <div class="hubNow"><div><div class="liveLabel">En direct sur votre panneau</div><p class="hubNowTitle" id="activeAppLabel">Connexion au panneau…</p><p class="smallPrint" id="hubPanelState"></p></div><div id="hubPreviewHost"></div></div>
  <div class="libraryShelf"><div class="libraryFilters" role="group" aria-label="Bibliothèque et Store"><button class="btn active" id="libraryModeBtn" type="button" aria-pressed="true">Ma bibliothèque</button><button class="btn" id="storeModeBtn" type="button" aria-pressed="false">Store</button></div><span class="smallPrint" id="contentCapacity"></span><button class="btn quiet" id="restoreLogosBtn" type="button" hidden>Restaurer les logos</button><button class="btn quiet" id="storeRefreshBtn" type="button" hidden>Actualiser le Store</button></div><p class="smallPrint" id="contentStatus" role="status" aria-live="polite" hidden></p>
  <div class="libraryToolbar"><div class="libraryFilters" role="group" aria-label="Catégories de la bibliothèque"><button class="libraryFilter active" type="button" data-library-filter="all" aria-pressed="true">Tout</button><button class="libraryFilter" type="button" data-library-filter="clock" aria-pressed="false">Horloges</button><button class="libraryFilter" type="button" data-library-filter="animation" aria-pressed="false">Animations</button><button class="libraryFilter" type="button" data-library-filter="weather" aria-pressed="false">Météo</button><button class="libraryFilter" type="button" data-library-filter="aviation" aria-pressed="false">Aviation</button><button class="libraryFilter" type="button" data-library-filter="widgets" aria-pressed="false">Pratique</button><button class="libraryFilter" type="button" data-library-filter="pixel" aria-pressed="false">Pixel art</button><button class="libraryFilter" type="button" data-library-filter="favorites" aria-pressed="false">Favoris</button></div><span class="libraryCount" id="libraryCount" aria-live="polite"></span></div>
  <div class="libraryGrid" id="libraryGrid" aria-label="Contenus disponibles"></div>
  <p class="libraryEmpty" id="libraryEmpty" hidden>Aucun résultat. Essayez une autre recherche.</p>
</section>
<section class="moduleView" id="moduleView" hidden aria-label="Application">
  <div class="card moduleHero">
    <div class="eyebrow" id="moduleEyebrow">Météo locale</div>
    <h1 id="moduleHeading">Météo</h1>
    <div id="weatherContent"><div class="weatherReading"><span class="weatherTemperature" id="weatherTemp">—</span><span class="weatherIcon" id="weatherIcon" aria-hidden="true">☁</span></div><p class="weatherCondition" id="weatherCondition">En attente du relevé</p><div class="weatherMeta"><span>Vent<strong id="weatherWind">— km/h</strong></span><span id="weatherAge">Données indisponibles</span></div><p class="smallPrint">Données <a href="https://open-meteo.com/" target="_blank" rel="noopener noreferrer">Open-Meteo</a> · Conditions estimées pour le lieu choisi.</p></div>
    <div id="clockContent" hidden><div class="clockFace" id="moduleClock">--:--</div><p class="moduleLead" id="moduleClockDate">Synchronisation de l’heure…</p></div>
    <p class="moduleLead" id="animationContent" hidden>Choisissez votre ambiance. Laissez vivre les pixels.</p>
    <div id="widgetContent" hidden>
      <form id="widgetForm">
        <div id="messageFields" hidden><label class="field">Votre message<textarea id="messageText" rows="3" maxlength="120" placeholder="Bienvenue à la maison"></textarea></label><label class="field">Couleur<input type="color" id="messageColor" value="#a9d8c6"></label><p class="smallPrint">120 caractères maximum. Les accents sont simplifiés pour le panneau.</p></div>
        <div id="timerFields" hidden><p class="smallPrint">Après un redémarrage, le minuteur revient en pause au dernier état enregistré.</p><div class="formGrid"><label class="field">Minutes<input id="timerMinutes" type="number" min="0" max="99" value="5"></label><label class="field">Secondes<input id="timerSeconds" type="number" min="0" max="59" value="0"></label></div></div>
        <div id="scoreFields" hidden><div class="formGrid"><label class="field">Compétition<select id="scoreLeague"><option value="fra.1">Ligue 1</option><option value="eng.1">Premier League</option><option value="uefa.champions">Ligue des champions</option></select></label><label class="field">Date<input id="scoreDate" type="date"></label></div><button class="btn" id="scoreSearchBtn" type="button" style="margin-top:14px">Rechercher les matchs</button><label class="field">Match<select id="scoreEvent"><option value="">Recherchez une date</option></select></label><p class="smallPrint" id="scoreSearchStatus" role="status"></p><p class="smallPrint">Scores <a href="https://www.espn.com/soccer/" target="_blank" rel="noopener noreferrer">ESPN</a> · Suivi automatique chaque minute sur le panneau.</p></div>
        <div class="dialogActions" id="widgetSaveRow"><button class="btn primary" id="widgetSaveBtn" type="submit">Enregistrer</button></div>
        <p class="moduleFeedback" id="widgetFeedback" role="status" aria-live="polite"></p>
      </form>
      <div id="timerControls" hidden><p class="clockFace" id="timerReadout">05:00</p><p class="smallPrint" id="timerState">Prêt à démarrer</p><div class="serviceButtons"><button class="btn primary" type="button" data-timer="start">Démarrer</button><button class="btn" type="button" data-timer="pause">Pause</button><button class="btn" type="button" data-timer="resume">Reprendre</button><button class="btn" type="button" data-timer="reset">Réinitialiser</button></div></div>
      <p class="moduleLead" id="pixelInfo" hidden></p><button class="btn quiet" id="contentRemoveBtn" type="button" hidden style="margin-top:16px">Retirer de la bibliothèque</button>
    </div>
    <button class="btn" type="button" id="moduleSettingsBtn" style="align-self:flex-start">Modifier le lieu</button>
  </div>
  <div class="card moduleSide"><div><h2 class="cardTitle" id="moduleOptionsTitle">Sur votre panneau</h2><p class="moduleLead" id="moduleHint" style="margin-top:10px">Activez cette application pour l’afficher.</p><div class="visualChoices" id="visualChoices" hidden><button class="btn" data-visual="aurora" type="button" aria-pressed="false"><span class="visualDot"></span>Aurore boréale</button><button class="btn" data-visual="ocean" type="button" aria-pressed="false"><span class="visualDot"></span>Océan</button><button class="btn" data-visual="warp" type="button" aria-pressed="false"><span class="visualDot"></span>Voyage spatial</button><button class="btn" data-visual="lava" type="button" aria-pressed="false"><span class="visualDot"></span>Lave</button><button class="btn" data-visual="stars" type="button" aria-pressed="false"><span class="visualDot"></span>Étoiles</button><button class="btn" data-visual="rain" type="button" aria-pressed="false"><span class="visualDot"></span>Pluie rétro</button><button class="btn" data-visual="plain" type="button" aria-pressed="false"><span class="visualDot"></span><span id="plainVisualLabel">Minimal</span></button></div><p class="moduleFeedback" id="moduleFeedback" role="status" aria-live="polite"></p></div><div id="modulePreviewHost"></div></div>
</section>
<div class="aviationApp" id="aviationApp" hidden>
<div class="locationBar">
  <div><div class="eyebrow">Votre ciel, en direct</div><div class="location"><h1 id="placeBadge">Flight Board</h1><span class="demoBadge" id="demoBadge" hidden>Démonstration</span></div><p class="placeMeta" id="placeMeta">Connexion au panneau…</p></div>
  <div class="locationPicker"><select id="favoriteSelect" aria-label="Changer de lieu favori"><option value="">Lieux favoris</option></select><button class="btn" id="locationBtn" type="button">Modifier le lieu</button></div>
</div>
<div class="demoNotice" id="demoNotice">Avions fictifs · Les commandes modifient uniquement cet aperçu. <button class="btn quiet" id="liveBtn" type="button">Revenir au direct</button></div>
<main class="main">
  <section class="card skyCard" aria-labelledby="radarHeading">
    <div class="cardHead"><h2 class="cardTitle" id="radarHeading">À proximité</h2><div class="filterHead"><button class="btn filterButton" id="filtersBtn" type="button" aria-haspopup="dialog" aria-controls="filtersDialog">Filtres</button><span class="countBadge" id="count">En attente</span></div></div><p class="filterNotice" id="filterNotice" hidden></p>
    <div class="skyBody">
      <div class="radarBlock">
        <div class="radar" id="radar" aria-label="Position des avions autour du lieu choisi"><div class="ring r1"></div><div class="ring r2"></div><div class="crossH"></div><div class="crossV"></div><span class="cardinal north">N</span><span class="cardinal south">S</span><span class="cardinal east">E</span><span class="cardinal west">O</span><div class="center"></div><div class="planes" id="planeLayer"></div></div>
        <div class="radarCaption"><strong id="radarTitle">Recherche des avions…</strong><span id="radarSub">Touchez un avion pour l’explorer.</span></div>
      </div>
      <div class="nearby"><div class="listLabel">Les plus proches</div><div class="flightList" id="flightList" aria-label="Avions à proximité"></div><span class="subtle" id="updated">En attente des données</span><span class="subtle" id="radarCadence" hidden></span></div>
    </div>
  </section>
  <section class="card flightCard" aria-labelledby="flightHeading">
    <div class="cardHead"><h2 class="cardTitle" id="flightHeading">Le vol sélectionné</h2><span class="subtle" id="selectedState">—</span></div>
    <div class="details" id="details" hidden>
      <div class="flightHeader"><div class="logoFrame"><img id="logo" width="60" height="40" alt="" hidden><canvas id="logoPixels" width="24" height="16" aria-label="Logo de la compagnie" hidden></canvas><span id="logoFallback" aria-hidden="true"><svg viewBox="-15 -15 30 30"><use href="#planeIcon"/></svg></span></div><div style="min-width:0"><div class="callsign" id="callsign">—</div><div class="airline" id="airline">—</div><div class="type" id="type">—</div></div></div>
      <div class="route" id="route" aria-label="Provenance et destination"><div class="routeAirports" id="routeAirports" hidden><div class="routeAirport"><div class="routeCode" id="routeOriginCode">—</div><span class="routeCity" id="routeOriginCity"></span></div><span class="routeArrow" aria-hidden="true">→</span><div class="routeAirport"><div class="routeCode" id="routeDestinationCode">—</div><span class="routeCity" id="routeDestinationCity"></span></div></div><p class="routeNote" id="routeNote" role="status">Recherche du trajet…</p></div>
      <div class="stats"><div><div class="statLabel">Altitude</div><div class="statValue" id="altitude">—</div><div class="statUnit">ft</div></div><div><div class="statLabel">Vitesse sol</div><div class="statValue" id="speed">—</div><div class="statUnit">km/h</div></div><div><div class="statLabel">Distance</div><div class="statValue" id="distance">—</div><div class="statUnit">km</div></div></div>
    </div>
    <div class="empty" id="empty"><strong id="emptyTitle">Le ciel arrive.</strong><span id="emptyText">Connexion au panneau…</span></div>
    <div class="modeBlock"><p class="modeLabel" id="modeHint">Affichage du panneau</p><div class="modeButtons"><button class="btn" id="autoBtn" type="button" aria-pressed="false">Auto</button><button class="btn" id="sendBtn" type="button" disabled aria-pressed="false">Épingler ce vol</button><button class="btn" id="clockBtn" type="button" aria-pressed="false">Horloge</button></div></div>
    <div id="aviationPreviewHost"><details class="preview" id="previewToggle" open><summary><span>Aperçu LED</span><span id="displayMode">Auto · 128 × 64</span></summary><div class="previewBody"><canvas id="ledPreview" width="128" height="64" aria-label="Aperçu du panneau LED"></canvas><p class="previewInfo"><b id="previewStatus">Aperçu du panneau</b><span id="previewHelp">Aperçu du panneau 128 × 64.</span></p></div></details></div>
  </section>
</main>
</div>
<footer class="bottom"><span class="statusLine" id="message" role="status" aria-live="polite">Votre panneau. Votre univers.</span><span class="source" id="sourceLabel">by Christ_Kmusic · V4.1.2</span></footer>
</div>
<dialog class="dialog" id="filtersDialog" aria-labelledby="filtersTitle"><div class="dialogHead"><h2 class="dialogTitle" id="filtersTitle">Filtres aviation</h2><button class="btn quiet" id="filtersCloseBtn" type="button">Fermer</button></div><p class="dialogIntro">Filtres mémorisés pour le radar et le panneau.</p><form id="filtersForm"><label class="check"><input type="checkbox" id="filterLogo">Compagnies avec un logo uniquement</label><label class="field">Compagnie<select id="filterAirline"><option value="">Toutes les compagnies</option></select></label><div class="formGrid"><label class="field">Type d’appareil<input id="filterType" maxlength="4" pattern="[A-Za-z0-9]{2,4}" placeholder="Tous · ex. A320" autocapitalize="characters" autocomplete="off"></label><label class="field">Altitude maximale<select id="filterAltitude"><option value="0">Sans limite</option><option value="5000">5 000 ft</option><option value="10000">10 000 ft</option><option value="20000">20 000 ft</option><option value="30000">30 000 ft</option><option value="40000">40 000 ft</option><option value="60000">60 000 ft</option></select></label></div><p class="smallPrint">Les vols sans cette donnée sont exclus du filtre.</p><p class="dialogFeedback" id="filtersFeedback" role="status" aria-live="polite"></p><div class="dialogActions"><button class="btn" id="filtersResetBtn" type="button">Tout afficher</button><button class="btn primary" id="filtersApplyBtn" type="submit">Appliquer les filtres</button></div></form></dialog>
<dialog class="dialog" id="settingsDialog" aria-labelledby="settingsTitle">
<div class="dialogHead"><h2 class="dialogTitle" id="settingsTitle">Réglages</h2><button class="btn quiet" type="button" id="cancelBtn">Fermer</button></div>
<p class="dialogIntro">Vos réglages sont conservés après un redémarrage.</p>
<form id="settingsForm">
<details class="section" id="brightnessSection" open><summary>Luminosité du panneau</summary><div class="sectionBody"><label class="field">Luminosité de jour<input id="brightnessInput" type="range" min="10" max="100" step="5"><span id="brightnessValue">60 %</span></label><p class="smallPrint">Réglage en direct, mémorisé automatiquement. La nuit reste prioritaire.</p><p class="smallPrint" id="brightnessFeedback" role="status" aria-live="polite"></p></div></details>
<details class="section" id="locationSection"><summary>Lieu et rayon</summary><div class="sectionBody">
<label class="field">Un lieu prédéfini<select id="cityInput"><option value="niort">Niort</option><option value="cdg">Paris-CDG</option><option value="orly">Paris-Orly</option><option value="paris">Paris</option><option value="londres">Londres</option><option value="amsterdam">Amsterdam</option><option value="francfort">Francfort</option><option value="newyork">New York</option><option value="dubai">Dubaï</option><option value="custom">Autre lieu / coordonnées GPS</option></select></label>
<label class="field">Nom du lieu<input id="placeInput" maxlength="32" placeholder="Maison" required></label>
<div class="formGrid"><label class="field">Latitude<input id="latInput" type="number" step="any" min="-90" max="90" required></label><label class="field">Longitude<input id="lonInput" type="number" step="any" min="-180" max="180" required></label><label class="field">Rayon<select id="radiusInput"><option value="5">5 km</option><option value="10">10 km</option><option value="20">20 km</option><option value="30">30 km</option><option value="50">50 km</option></select></label></div></div></details>
<details class="section" id="fontsSection"><summary>Polices du panneau</summary><div class="sectionBody"><p class="smallPrint">Une police pour chaque application. Les aperçus montrent les vrais pixels du panneau. En aviation, l’horloge sans vol utilise la police Horloge.</p><div class="formGrid"><label class="field">Aviation<select id="fontAviationInput" data-font-preview="fontAviationPreview"><option value="classic">Pixel classique</option><option value="modern">Moderne</option><option value="digital">Numérique · chiffres à segments</option><option value="arcade">Arcade</option><option value="compact">Compacte</option></select><canvas id="fontAviationPreview" width="128" height="22" aria-label="Aperçu de la police Aviation" style="width:100%;max-width:256px;background:#000;image-rendering:pixelated;border-radius:4px"></canvas></label><label class="field">Horloge<select id="fontClockInput" data-font-preview="fontClockPreview"><option value="classic">Pixel classique</option><option value="modern">Moderne</option><option value="digital">Numérique · chiffres à segments</option><option value="arcade">Arcade</option><option value="compact">Compacte</option></select><canvas id="fontClockPreview" width="128" height="22" aria-label="Aperçu de la police Horloge" style="width:100%;max-width:256px;background:#000;image-rendering:pixelated;border-radius:4px"></canvas></label><label class="field">Météo<select id="fontWeatherInput" data-font-preview="fontWeatherPreview"><option value="classic">Pixel classique</option><option value="modern">Moderne</option><option value="digital">Numérique · chiffres à segments</option><option value="arcade">Arcade</option><option value="compact">Compacte</option></select><canvas id="fontWeatherPreview" width="128" height="22" aria-label="Aperçu de la police Météo" style="width:100%;max-width:256px;background:#000;image-rendering:pixelated;border-radius:4px"></canvas></label><label class="field">Messages<select id="fontMessageInput" data-font-preview="fontMessagePreview"><option value="classic">Pixel classique</option><option value="modern">Moderne</option><option value="digital">Numérique · chiffres à segments</option><option value="arcade">Arcade</option><option value="compact">Compacte</option></select><canvas id="fontMessagePreview" width="128" height="22" aria-label="Aperçu de la police Messages" style="width:100%;max-width:256px;background:#000;image-rendering:pixelated;border-radius:4px"></canvas></label><label class="field">Minuteur<select id="fontTimerInput" data-font-preview="fontTimerPreview"><option value="classic">Pixel classique</option><option value="modern">Moderne</option><option value="digital">Numérique · chiffres à segments</option><option value="arcade">Arcade</option><option value="compact">Compacte</option></select><canvas id="fontTimerPreview" width="128" height="22" aria-label="Aperçu de la police Minuteur" style="width:100%;max-width:256px;background:#000;image-rendering:pixelated;border-radius:4px"></canvas></label><label class="field">Scores<select id="fontScoreInput" data-font-preview="fontScorePreview"><option value="classic">Pixel classique</option><option value="modern">Moderne</option><option value="digital">Numérique · chiffres à segments</option><option value="arcade">Arcade</option><option value="compact">Compacte</option></select><canvas id="fontScorePreview" width="128" height="22" aria-label="Aperçu de la police Scores" style="width:100%;max-width:256px;background:#000;image-rendering:pixelated;border-radius:4px"></canvas></label></div></div></details>
<details class="section" id="nightSection"><summary>Horloge et nuit</summary><div class="sectionBody">
<label class="field">Animation de l’horloge par défaut<select id="clockVisualInput"><option value="lava">Lave</option><option value="aurora">Aurore</option><option value="ocean">Océan</option><option value="warp">Hyperespace</option><option value="stars">Étoiles</option><option value="rain">Matrix</option><option value="plain">Minimal · fond noir</option></select></label><p class="smallPrint">Utilisée aussi en aviation quand aucun vol ne s’affiche. Ce choix est conservé après redémarrage.</p>
<label class="field">Fuseau horaire<select id="timezoneInput"><option value="paris">Paris · Europe centrale</option><option value="london">Londres</option><option value="utc">UTC</option><option value="newyork">New York</option><option value="dubai">Dubaï</option></select></label>
<p class="smallPrint">Heure et changements de saison automatiques.</p>
<label class="check"><input type="checkbox" id="nightEnabled">Adapter l’affichage pendant la nuit</label>
<div class="formGrid"><label class="field">Début<input type="time" id="nightStart" value="22:00" required></label><label class="field">Fin<input type="time" id="nightEnd" value="07:00" required></label><label class="field">Pendant la nuit<select id="nightAction"><option value="dim">Réduire la luminosité</option><option value="sleep">Mettre en veille</option></select></label><label class="field">Luminosité de nuit<input type="range" id="nightBrightness" min="10" max="100" step="5"><span id="nightBrightnessValue">15 %</span></label></div><p class="smallPrint">Réglage en direct pendant la nuit, mémorisé automatiquement et limité à la luminosité du jour. En veille, le panneau reste éteint.</p><p class="smallPrint" id="nightBrightnessFeedback" role="status" aria-live="polite"></p>
<p class="smallPrint">En aviation, l’horloge apparaît quand aucun vol n’est disponible.</p>
</div></details>
<p class="dialogFeedback" id="settingsFeedback" role="status" aria-live="polite"></p>
<div class="dialogActions"><button class="btn primary" type="submit" id="saveSettingsBtn">Enregistrer les réglages</button></div>
</form>
<details class="section" id="favoritesSection"><summary>Lieux favoris</summary><p class="smallPrint">Enregistrez ce lieu. Un nom existant sera mis à jour.</p><div class="favoriteAdd"><label class="field">Nom du favori<input id="favoriteName" maxlength="32" placeholder="Maison"></label><button class="btn" id="favoriteSaveBtn" type="button">Ajouter aux favoris</button></div><div class="favoriteList" id="favoriteList"></div><p class="dialogFeedback" id="favoritesFeedback" role="status" aria-live="polite"></p></details>
<details class="section" id="systemSection"><summary>Connexion et mises à jour</summary><div class="serviceButtons"><button class="btn" id="wifiBtn" type="button" aria-haspopup="dialog" aria-controls="wifiDialog">Configurer le Wi-Fi</button><button class="btn" id="openUpdateBtn" type="button" aria-haspopup="dialog" aria-controls="updateDialog">Mettre à jour le programme</button></div><p class="smallPrint">ZCS CONTRÔLE V4.1.2 · Luminosité du panneau réglable de 10 à 100 %.</p></details>
</dialog>
<dialog class="dialog" id="updateDialog" aria-labelledby="updateTitle">
<div class="dialogHead"><h2 class="dialogTitle" id="updateTitle">Mise à jour</h2><button class="btn quiet" id="updateCloseBtn" type="button">Fermer</button></div>
<p class="dialogIntro">Mettez votre panneau à jour. Il redémarrera en conservant vos réglages.</p>
<div class="uploadInfo" id="systemInfo">Lecture du panneau…</div>
<button class="btn" id="checkUpdateBtn" type="button" disabled>Rechercher une mise à jour</button>
<div class="uploadInfo" id="releaseInfo" hidden><strong id="releaseVersion"></strong><p class="smallPrint" id="releaseNotes" style="white-space:pre-line;overflow-wrap:anywhere"></p></div>
<details class="section" id="manualUpdate"><summary>Installer un fichier manuellement</summary>
<label class="field">Programme (.bin)<input type="file" id="firmwareFile" accept=".bin,application/octet-stream"></label>
<p class="smallPrint">Fichier pour ESP32-S3. Gardez le panneau alimenté pendant l’installation.</p>
<p class="uploadInfo uploadFile" id="firmwareInfo" hidden></p>
</details>
<progress id="updateProgress" max="100" value="0" hidden aria-label="Progression de l’installation"></progress>
<p class="dialogFeedback" id="updateFeedback" role="status" aria-live="polite"></p>
<div class="dialogActions"><button class="btn primary" id="updateBtn" type="button" hidden disabled>Installer ce programme</button></div>
</dialog>
<dialog class="wifiDialog" id="wifiDialog" aria-labelledby="wifiTitle" aria-describedby="wifiIntro">
  <div class="wifiHead">
    <h2 class="dialogTitle" id="wifiTitle">Connexion Wi-Fi</h2>
    <button type="button" class="btn" id="wifiCloseBtn" aria-label="Fermer la configuration Wi-Fi">Fermer</button>
  </div>
  <p class="wifiHelp" id="wifiIntro">Réseau Wi-Fi 2,4 GHz. La connexion sera mémorisée.</p>
  <p class="wifiHelp" id="wifiCurrent">Lecture de la connexion…</p>
  <div class="wifiStatus" id="wifiFeedback" role="status" aria-live="polite" aria-atomic="true">Recherche du panneau…</div>
  <form id="wifiForm">
    <div class="wifiScanRow">
      <label class="field" for="wifiNetworks">Réseaux 2,4 GHz détectés
        <select id="wifiNetworks" aria-describedby="wifiScanHelp">
          <option value="">Choisir un réseau ou saisir son nom</option>
        </select>
      </label>
      <button type="button" class="btn" id="wifiScanBtn">Rechercher</button>
    </div>
    <p class="wifiHelp" id="wifiScanHelp" role="status" aria-live="polite">Réseau masqué ? Saisissez son nom ci-dessous.</p>
    <label class="field" for="wifiSsid">Nom du réseau
      <input id="wifiSsid" name="ssid" maxlength="32" autocomplete="off" autocapitalize="none" spellcheck="false" required aria-describedby="wifiSsidHelp">
    </label>
    <p class="wifiHelp" id="wifiSsidHelp">Le nom doit être exact.</p>
    <label class="wifiCheck"><input id="wifiOpen" type="checkbox">Réseau sans mot de passe</label>
    <label class="field" for="wifiPassword">Mot de passe du réseau
      <input id="wifiPassword" name="password" type="password" maxlength="64" autocomplete="new-password" autocapitalize="none" spellcheck="false" aria-describedby="wifiPasswordHelp">
    </label>
    <label class="wifiCheck"><input id="wifiShowPassword" type="checkbox">Afficher le mot de passe</label>
    <p class="wifiHelp" id="wifiPasswordHelp">Mot de passe effacé du formulaire après envoi.</p>
    <div class="dialogActions">
      <button type="button" class="btn" id="wifiStatusBtn">Actualiser</button>
      <button type="submit" class="btn primary" id="wifiSaveBtn">Connecter</button>
    </div>
  </form>
  <div class="wifiRescue wifiHelp" id="wifiRescue" hidden>
    <strong>Si la tablette perd la connexion</strong>
    <p>Rejoignez le Wi-Fi de secours <b id="wifiApSsid"></b> avec la clé <code id="wifiApPassword"></code>, puis ouvrez <a href="http://192.168.4.1/">http://192.168.4.1/</a>.</p>
    <p>Ou rejoignez le réseau du panneau, puis ouvrez <a href="http://flightboard.local/">http://flightboard.local/</a>.</p>
    <p>Ce Wi-Fi de secours peut s’arrêter après 60 secondes si le panneau est connecté et qu’aucune tablette ne l’utilise.</p>
  </div>
</dialog>
<script src="/api/update-hash.js?v=4.0.0"></script>
<script>
'use strict';
const el = id => document.getElementById(id);
// Every request, including its JSON body, has a deadline so a lost Wi-Fi
// connection cannot leave the interface permanently busy.
async function requestJson(path, options = {}, timeoutMs = 8000) {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), timeoutMs);

  try {
    const response = await fetch(path, {
      cache: 'no-store',
      ...options,
      signal: controller.signal
    });
    const json = await response.json();

    if (!response.ok) {
      const error = new Error(json.error || 'Erreur serveur ' + response.status);
      error.status = response.status;
      throw error;
    }

    return json;
  } catch (error) {
    if (controller.signal.aborted) {
      const timeout = new Error('Délai de réponse dépassé. Réessaie.');
      timeout.timeout = true;
      throw timeout;
    }
    throw error;
  } finally {
    clearTimeout(timer);
  }
}

// =====================================
// CONFIGURATION WI-FI DEPUIS LA TABLETTE
// =====================================

let wifiEpoch = 0;
let wifiStatusRun = 0;
let wifiScanRun = 0;
let wifiStatusTimer = null;
let wifiScanBusy = false;
let wifiStatusBusy = false;
let wifiSaving = false;
let wifiConnecting = false;
let wifiTargetSsid = '';
let wifiLastState = null;

// Garder le portail reactif pendant la saisie : le radar reprendra a la fermeture.
let wifiKeepaliveBusy = false;
setInterval(async () => {
  if (!wifiIsOpen() || wifiKeepaliveBusy) return;
  wifiKeepaliveBusy = true;
  try {
    await requestJson('/api/wifi/keepalive', {}, 4000);
  } catch (error) {
    // Le suivi de connexion affiche deja les instructions en cas de coupure.
  } finally {
    wifiKeepaliveBusy = false;
  }
}, 5000);

function wifiIsOpen() {
  return el('wifiDialog').open;
}

function wifiFeedback(message, tone = '') {
  el('wifiFeedback').textContent = message;
  el('wifiFeedback').dataset.tone = tone;
}

function updateWifiControls() {
  const busy = wifiSaving || wifiConnecting;
  ['wifiNetworks', 'wifiSsid', 'wifiOpen'].forEach(id => {
    el(id).disabled = busy;
  });
  el('wifiPassword').disabled = busy || el('wifiOpen').checked;
  el('wifiPassword').required = !el('wifiOpen').checked;
  el('wifiShowPassword').disabled = busy || el('wifiOpen').checked;
  el('wifiScanBtn').disabled = busy || wifiScanBusy;
  el('wifiScanBtn').textContent = wifiScanBusy ? 'RECHERCHE…' : 'RECHERCHER';
  el('wifiStatusBtn').disabled = wifiSaving || wifiStatusBusy;
  el('wifiSaveBtn').disabled = busy || wifiScanBusy;
  el('wifiSaveBtn').textContent = busy ? 'CONNEXION…' : 'CONNECTER';
  el('wifiForm').setAttribute('aria-busy', String(busy));
}

function clearWifiPassword() {
  el('wifiPassword').value = '';
  el('wifiPassword').type = 'password';
  el('wifiShowPassword').checked = false;
}

function showWifiState(state) {
  wifiLastState = state;
  el('wifiCurrent').textContent = state.connected
    ? 'Connecté à « ' + state.ssid + ' » · ' + state.ip
    : 'Le Flight Board n’est pas connecté à un réseau Wi-Fi.';

  // This is the device's setup key, never the home router's password.
  // Retain it in the open dialog if a later status request loses contact.
  if (state.apSsid && state.apPassword) {
    el('wifiApSsid').textContent = state.apSsid;
    el('wifiApPassword').textContent = state.apPassword;
    el('wifiRescue').hidden = false;
  }
}

function beginWifiStatus() {
  clearTimeout(wifiStatusTimer);
  const run = ++wifiStatusRun;
  wifiStatusBusy = false;
  pollWifiStatus(wifiEpoch, run, Date.now() + 60000);
}

async function pollWifiStatus(epoch, run, deadline) {
  if (!wifiIsOpen() || epoch !== wifiEpoch || run !== wifiStatusRun) return;
  wifiStatusBusy = true;
  updateWifiControls();
  let keepPolling = false;

  try {
    const state = await requestJson('/api/wifi', {}, Math.max(500, Math.min(8000, deadline - Date.now())));
    if (!wifiIsOpen() || epoch !== wifiEpoch || run !== wifiStatusRun) return;
    showWifiState(state);

    if (!wifiTargetSsid && state.state === 'connecting') {
      wifiTargetSsid = state.pendingSsid || '';
    }

    if (wifiTargetSsid && state.state === 'connected' && state.connected && state.ssid === wifiTargetSsid) {
      wifiFeedback('Connexion réussie à « ' + state.ssid + ' ». Le Flight Board a enregistré ce réseau.', 'success');
      wifiTargetSsid = '';
      wifiConnecting = false;
      refresh();
    } else if (state.state === 'failed') {
      wifiFeedback(state.message || 'Connexion impossible. Vérifie le réseau et son mot de passe, puis réessaie.', 'error');
      wifiTargetSsid = '';
      wifiConnecting = false;
    } else if (wifiTargetSsid || state.state === 'connecting') {
      wifiConnecting = true;
      keepPolling = true;
      wifiFeedback('Connexion à « ' + (wifiTargetSsid || state.pendingSsid || 'ton réseau') + ' » en cours. La tablette peut perdre la liaison : les instructions de secours sont ci-dessous.');
    } else {
      wifiConnecting = false;
      wifiFeedback(state.message || (state.connected ? 'Le Flight Board est connecté. Tu peux choisir un autre réseau.' : 'Choisis un réseau pour connecter le Flight Board à Internet.'), state.connected ? 'success' : '');
    }
  } catch (error) {
    if (!wifiIsOpen() || epoch !== wifiEpoch || run !== wifiStatusRun) return;
    keepPolling = Boolean(wifiTargetSsid || wifiConnecting);
    wifiFeedback(keepPolling
      ? 'Liaison interrompue pendant le changement de réseau. Rejoins le Wi-Fi indiqué ci-dessous, puis rouvre cette page pour vérifier la connexion.'
      : 'État Wi-Fi indisponible. ' + error.message, keepPolling ? '' : 'error');
  } finally {
    if (wifiIsOpen() && epoch === wifiEpoch && run === wifiStatusRun) {
      wifiStatusBusy = false;
      if (keepPolling && Date.now() < deadline) {
        wifiStatusTimer = setTimeout(() => pollWifiStatus(epoch, run, deadline), 1800);
      } else if (keepPolling) {
        wifiConnecting = false;
        wifiFeedback('La connexion n’a pas pu être confirmée dans le délai prévu. Rejoins le Wi-Fi de secours ou ton réseau habituel, puis utilise « Actualiser l’état ».', 'error');
      }
      updateWifiControls();
    }
  }
}

async function scanWifi(forceRefresh = false) {
  if (!wifiIsOpen() || wifiSaving || wifiConnecting) return;
  const epoch = wifiEpoch;
  const run = ++wifiScanRun;
  const deadline = Date.now() + 25000;
  wifiScanBusy = true;
  updateWifiControls();
  el('wifiScanHelp').textContent = 'Recherche des réseaux 2,4 GHz à proximité…';
  let first = true;

  try {
    while (wifiIsOpen() && epoch === wifiEpoch && run === wifiScanRun) {
      const remaining = deadline - Date.now();
      if (remaining <= 0) throw new Error('Recherche trop longue. Utilise « Rechercher » pour réessayer ou saisis le nom du réseau.');
      const result = await requestJson('/api/wifi/scan' + (first && forceRefresh ? '?refresh=1' : ''), {}, Math.min(8000, remaining));
      first = false;
      if (!wifiIsOpen() || epoch !== wifiEpoch || run !== wifiScanRun) return;
      if (result.error) throw new Error(result.error);
      if (!result.scanning) {
        const select = el('wifiNetworks');
        select.replaceChildren(new Option('Choisir un réseau ou saisir son nom', ''));
        const networks = Array.isArray(result.networks) ? result.networks : [];
        networks.forEach(network => {
          const option = new Option(network.ssid + ' · ' + network.rssi + ' dBm' + (network.open ? ' · ouvert' : ' · protégé'), network.ssid);
          option.dataset.open = String(Boolean(network.open));
          select.add(option);
        });
        el('wifiScanHelp').textContent = networks.length
          ? networks.length + ' réseau(x) détecté(s). Un réseau masqué peut être saisi ci-dessous.'
          : 'Aucun réseau détecté. Réessaie ou saisis le nom d’un réseau masqué.';
        return;
      }
      await new Promise(resolve => setTimeout(resolve, 1200));
    }
  } catch (error) {
    if (wifiIsOpen() && epoch === wifiEpoch && run === wifiScanRun) {
      el('wifiScanHelp').textContent = 'Recherche indisponible. ' + error.message;
    }
  } finally {
    if (wifiIsOpen() && epoch === wifiEpoch && run === wifiScanRun) {
      wifiScanBusy = false;
      updateWifiControls();
    }
  }
}

function openWifi(initialState = null) {
  if (wifiIsOpen()) return;
  el('settingsDialog').close();
  ++wifiEpoch;
  wifiScanBusy = false;
  wifiStatusBusy = false;
  wifiSaving = false;
  wifiConnecting = false;
  clearWifiPassword();
  if (initialState || wifiLastState) showWifiState(initialState || wifiLastState);
  el('wifiDialog').showModal();
  wifiFeedback('Lecture de la connexion Wi-Fi…');
  updateWifiControls();
  beginWifiStatus();
  scanWifi();
}

function wifiClosed() {
  ++wifiEpoch;
  ++wifiStatusRun;
  ++wifiScanRun;
  clearTimeout(wifiStatusTimer);
  wifiStatusBusy = false;
  wifiScanBusy = false;
  clearWifiPassword();
}

async function saveWifi(event) {
  event.preventDefault();
  if (wifiSaving || wifiConnecting || wifiScanBusy) return;
  const ssid = el('wifiSsid').value;
  const password = el('wifiPassword').value;
  const open = el('wifiOpen').checked;
  const size = value => new TextEncoder().encode(value).length;

  if (!ssid.length || size(ssid) > 32) {
    wifiFeedback('Le nom du réseau doit contenir de 1 à 32 octets.', 'error');
    el('wifiSsid').focus();
    return;
  }
  if (!open && !((size(password) >= 8 && size(password) <= 63) || /^[a-f0-9]{64}$/i.test(password))) {
    wifiFeedback('Saisis un mot de passe de 8 à 63 octets, ou une clé de 64 caractères hexadécimaux. Pour un réseau sans mot de passe, coche l’option « réseau ouvert ».', 'error');
    el('wifiPassword').focus();
    return;
  }

  const epoch = wifiEpoch;
  clearTimeout(wifiStatusTimer);
  ++wifiStatusRun;
  wifiTargetSsid = ssid;
  wifiSaving = true;
  wifiStatusBusy = false;
  clearWifiPassword();
  updateWifiControls();
  wifiFeedback('Envoi de la configuration pour « ' + ssid + ' »…');

  try {
    const state = await requestJson('/api/wifi', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ ssid, password: open ? '' : password, open })
    });
    if (!wifiIsOpen() || epoch !== wifiEpoch) return;
    showWifiState(state);
    wifiConnecting = true;
    wifiFeedback('Configuration reçue. Connexion à « ' + ssid + ' » en cours…');
  } catch (error) {
    if (!wifiIsOpen() || epoch !== wifiEpoch) return;
    if (error.status) {
      wifiTargetSsid = '';
      wifiConnecting = false;
      wifiFeedback(error.message + ' Saisis de nouveau le mot de passe pour réessayer.', 'error');
    } else {
      wifiConnecting = true;
      wifiFeedback('La réponse n’a pas été reçue. La configuration a peut-être été transmise. Vérification en cours…');
    }
  } finally {
    if (wifiIsOpen() && epoch === wifiEpoch) {
      wifiSaving = false;
      updateWifiControls();
      if (wifiConnecting) beginWifiStatus();
    }
  }
}

async function checkInitialWifi() {
  // This startup check runs once: dismissing the dialog never reopens it.
  const epoch = wifiEpoch;
  try {
    const state = await requestJson('/api/wifi');
    wifiLastState = state;
    if (!state.connected && epoch === wifiEpoch && !wifiIsOpen()) openWifi(state);
  } catch (error) {
    // The Wi-Fi button always permits a fresh status request.
    el('wifiBtn').title = 'État Wi-Fi indisponible : ouvrir pour réessayer';
  }
}

const DEMO = {
  version:'4.1.2',app:'aviation',visual:'lava',filters:{logoOnly:false,airline:'',type:'',maxAltitudeFt:0},totalCount:3,wifiConnected:true,radarOk:true,radiusKm:30,homeLat:46.324,homeLon:-.459,place:'Niort',ageSeconds:0,
  display:{mode:'auto',brightness:60,effectiveBrightness:60,sleeping:false,timeSynced:true,clock:'14:32',date:'15/10/2026',nightActive:false,activeHex:'demo-afr'},
  aircraft:[
    {hex:'demo-afr',callsign:'AFR1234',airline:'AIR FRANCE',type:'A320',lat:46.376,lon:-.535,track:124,trackAvailable:true,altitudeFt:34000,altitudeAvailable:true,speedKmh:820,speedAvailable:true,distanceKm:8,route:{status:'known',source:'demo',indicative:true,origin:{code:'CDG',city:'Paris',name:'Paris-Charles de Gaulle'},destination:{code:'BER',city:'Berlin',name:'Berlin Brandenburg'}}},
    {hex:'demo-ryr',callsign:'RYR4SG',airline:'RYANAIR',type:'B38M',lat:46.284,lon:-.258,track:310,trackAvailable:true,altitudeFt:38000,altitudeAvailable:true,speedKmh:780,speedAvailable:true,distanceKm:16.1,route:{status:'known',source:'demo',indicative:true,origin:{code:'BCN',city:'Barcelone',name:'Josep Tarradellas Barcelona-El Prat'},destination:{code:'BER',city:'Berlin',name:'Berlin Brandenburg'}}},
    {hex:'demo-ups',callsign:'UPS257',airline:'UPS AIRLINES',type:'B763',lat:46.185,lon:-.619,track:38,trackAvailable:true,altitudeFt:33000,altitudeAvailable:true,speedKmh:760,speedAvailable:true,distanceKm:19.8,route:{status:'known',source:'demo',indicative:true,origin:{code:'SDF',city:'Louisville',name:'Louisville Muhammad Ali'},destination:{code:'CGN',city:'Cologne',name:'Cologne Bonn'}}}
  ]
};
let demo=false,latest=null,currentSettings=null,selectedHex='',polling=false,lastStateAt=0,serverOnline=false,commandBusy=false;
let favorites=[],favoriteBusy=false,settingsBusy=false,settingsEpoch=0;
let systemInfo=null,firmwareReady=null,uploadBusy=false,firmwareValidationRun=0,updateEpoch=0;
let releaseInfo=null,releaseBusy=false,releaseController=null,releaseRun=0;
const logoCache=new Map(),pendingLogos=new Set(),logoFailed=new Map();
let logoImageId='',logoImageEpoch=0,logoImageFailedAt=0;
const routeCache=new Map(),routeLastRequest=new Map();
let routeBusy=false,routeEpoch=0,routeFocus='';
const CITIES={niort:{name:'Niort',lat:46.324,lon:-.459},cdg:{name:'Paris-CDG',lat:49.0097,lon:2.5479},orly:{name:'Paris-Orly',lat:48.7262,lon:2.3652},paris:{name:'Paris',lat:48.8566,lon:2.3522},londres:{name:'Londres',lat:51.5074,lon:-.1278},amsterdam:{name:'Amsterdam',lat:52.3676,lon:4.9041},francfort:{name:'Francfort',lat:50.1109,lon:8.6821},newyork:{name:'New York',lat:40.7128,lon:-74.006},dubai:{name:'Dubaï',lat:25.2048,lon:55.2708}};
const MODE_NAMES={auto:'Auto',manual:'Épinglé',selected:'Épinglé',clock:'Horloge'};
const fmt=(n,d=0)=>Number(n).toLocaleString('fr-FR',{maximumFractionDigits:d,minimumFractionDigits:d});
const key=p=>p.hex||p.callsign;
const data=()=>demo?DEMO:latest;

const LIBRARY_ITEMS=[
  {id:'weather',app:'weather',name:'Météo locale',description:'Le ciel de votre ville, animé au fil du temps.'},
  {id:'aviation',app:'aviation',name:'Flight Board',description:'Les avions autour de vous, leurs trajets et leurs logos.'},
  {id:'message',app:'message',name:'Votre message',description:'Un mot de bienvenue, une annonce, une pensée.'},
  {id:'timer',app:'timer',name:'Minuteur',description:'Un compte à rebours visible dans toute la pièce.'},
  {id:'score',app:'score',name:'Jour de match',description:'Votre match de football, suivi en direct avec ESPN.'},
  ...[['aurora','Aurore','Des voiles lumineux aux couleurs du Nord.'],['ocean','Océan','Des vagues bleues, tout en douceur.'],['warp','Hyperespace','Une traversée des étoiles en profondeur.'],['lava','Lave','Des contours fluides du bleu au turquoise.'],['stars','Étoiles','Un ciel étoilé aux reflets subtils.'],['rain','Matrix','Une pluie de pixels, esprit rétro.'],['plain','Minimal','L’essentiel, sur un fond noir.']].flatMap(([visual,name,description])=>['clock','animation'].map(app=>({id:app+'-'+visual,app,visual,name:app==='animation'&&visual==='plain'?'Écran noir':name,description})))
];
let libraryCategory='all',libraryQuery='',libraryFavorites=readLibraryFavorites(),libraryRenderKey='';
let contentItems=[],contentToken='',contentCapacity=14,contentLoaded=false,contentBusy=false,currentContentId='',storeMode=false,storeItems=[],storeBusy=false,storeRun=0;
const contentCache=new Map(),storeContentCache=new Map(),STORE_BASE='https://raw.githubusercontent.com/kmsha19/flightboard-updates/main/store/';
function contentItem(value,store=false){return{...value,id:(store?'store:':'pack:')+value.id,contentId:value.id,app:value.type,name:value.title,description:value.description||'Installé sur votre panneau.',store};}
function libraryItems(){return storeMode?storeItems.map(item=>contentItem(item,true)):[...LIBRARY_ITEMS,...contentItems.map(item=>contentItem(item))];}
function normalizeLibraryText(value){return String(value).normalize('NFD').replace(/[\u0300-\u036f]/g,'').toLowerCase().trim();}
function readLibraryFavorites(){try{const saved=JSON.parse(localStorage.getItem('zcs-library-favorites'));return Array.isArray(saved)?[...new Set(saved.filter(id=>typeof id==='string'&&(LIBRARY_ITEMS.some(item=>item.id===id)||/^pack:[a-z0-9_-]{1,32}$/.test(id))))].slice(0,64):[];}catch(error){return[];}}
function visibleLibraryItems(){const query=normalizeLibraryText(libraryQuery);return libraryItems().filter(item=>(libraryCategory==='all'||libraryCategory===item.app||libraryCategory==='widgets'&&['message','timer','score'].includes(item.app)||libraryCategory==='favorites'&&libraryFavorites.includes(item.id))&&normalizeLibraryText(item.name+' '+item.description+' '+APP_NAMES[item.app]).includes(query));}
function toggleLibraryFavorite(id){if(!libraryItems().some(item=>item.id===id))return;libraryFavorites=libraryFavorites.includes(id)?libraryFavorites.filter(value=>value!==id):[...libraryFavorites,id];try{localStorage.setItem('zcs-library-favorites',JSON.stringify(libraryFavorites));}catch(error){notify('Favori conservé pour cette session. Le stockage de ce navigateur est indisponible.');}renderLibrary();const cards=Array.from(el('libraryGrid').children),card=cards.find(node=>node.dataset.libraryId===id)||cards[0];(card?.querySelector('.libraryFavorite')||el('librarySearch')).focus({preventScroll:true});}
function selectedContentPack(){return(storeMode?storeContentCache:contentCache).get(currentContentId);}
function selectedContent(){return currentContentId?(storeMode?storeItems:contentItems).find(item=>item.id===currentContentId)||contentItems.find(item=>item.id===currentContentId):null;}
async function loadContent(){try{const value=await requestJson('/api/content');if(!Array.isArray(value.items)||value.items.length>14)throw new Error('Bibliothèque invalide.');contentItems=value.items.filter(item=>item&&/^[a-z0-9_-]{1,32}$/.test(item.id)&&['pixel','message','timer'].includes(item.type));contentToken=String(value.token||'');contentCapacity=Number(value.capacity)||14;contentLogos=value.logos||null;contentLoaded=true;ensureActiveContent();libraryRenderKey='';if(currentView==='home')renderLibrary();return value;}catch(error){contentNotice('Impossible de lire les contenus du panneau. Réessayez après reconnexion.');throw error;}}
function contentNotice(text){el('contentStatus').textContent=text;el('contentStatus').hidden=!text;}
async function openLibraryItem(id){const item=libraryItems().find(item=>item.id===id);if(!item)return;showModule(item.app);currentContentId=item.contentId||'';draftVisual=item.visual||null;widgetEditorKey='';render();if(item.contentId){try{await getContentPack(item);if(currentContentId===item.contentId){widgetEditorKey='';render();}}catch(error){el('widgetFeedback').textContent=error.message;}}}
async function activateLibraryItem(id){
  const item=libraryItems().find(item=>item.id===id);if(!item||demo||!serverOnline||appBusy||uploadBusy||contentBusy)return;
  if(item.store){await installContent(item);return;}
  if(['message','timer','score'].includes(item.app)&&!item.contentId){await openLibraryItem(id);return;}
  const payload={app:item.app};if(item.visual)payload.visual=item.visual;if(item.contentId)payload.contentId=item.contentId;appBusy=true;renderLibrary();
  try{await post('/api/app',payload);if(currentView===payload.app&&currentContentId===(payload.contentId||'')&&draftVisual===(payload.visual||null))draftVisual=null;notify(item.name+' · Affiché sur le panneau.');await refresh();}catch(error){notify(error.message);}finally{appBusy=false;render();}
}
function libraryThumbnail(canvas,item){
  const g=canvas.getContext('2d');g.fillStyle='#000';g.fillRect(0,0,128,64);
  if(item.contentId){const pack=(item.store?storeContentCache:contentCache).get(item.contentId);if(pack){drawWidgetPreview(g,item.app,pack,0);return;}}
  if(item.app==='clock'){drawIdleClock(g,{timeSynced:true,clock:'10:24'},3.2,item.visual);return;}
  if(item.app==='animation'){drawAmbient(g,item.visual,3.2);return;}
  if(item.app==='weather'){drawWeatherIcon(g,{code:2,isDay:true},0);canvasText(g,'METEO',50,20,'14px monospace','#edf0e9');canvasText(g,'VOTRE VILLE',51,40,'6px monospace','#a9d8c6');return;}
  if(item.app==='message'){drawWidgetPreview(g,'message',{text:'BONJOUR',color:0xafd8},0);return;}
  if(item.app==='timer'){drawWidgetPreview(g,'timer',{remainingSeconds:300,running:false},0);return;}
  if(item.app==='score'){canvasCenteredText(g,'JOUR DE MATCH',64,12,120,7,'#a9d8c6');canvasCenteredText(g,'-- : --',64,29,120,21,'#edf0e9');return;}
  if(item.app==='pixel'){for(let y=0;y<7;y++)for(let x=0;x<7;x++)if((x+y)%3!==0)panelRect(g,42+x*6,9+y*6,5,5,panelRgb565([0x5d9f,0x97fa,0xad5f][(x*y)%3]));return;}
  for(let x=12;x<116;x+=6)panelRect(g,x,40,3,1,'#486957');
  for(const [y,x,w] of [[12,63,2],[14,62,4],[16,61,6],[18,58,12],[20,53,22],[22,48,32],[24,48,32],[26,61,6],[28,61,6],[30,58,12],[32,57,14]])panelRect(g,x,y,w,2,'#c4f3d3');canvasCenteredText(g,'FLIGHT BOARD',64,49,118,7,'#edf0e9');
}
function renderLibrary(){
  const items=visibleLibraryItems(),renderKey=JSON.stringify([storeMode,libraryCategory,libraryQuery,libraryFavorites,items.map(i=>i.id+':'+i.version)]),grid=el('libraryGrid');
  if(renderKey!==libraryRenderKey){
    libraryRenderKey=renderKey;grid.replaceChildren();
    for(const item of items){
      const card=document.createElement('article');card.className='libraryCard';card.dataset.libraryId=item.id;
      card.innerHTML='<div class="cardArtwork"><canvas class="cardCanvas" width="128" height="64" aria-label="Illustration du contenu"></canvas><span class="libraryLiveBadge" hidden>Sur le panneau</span><button type="button" class="libraryFavorite"></button></div><div class="cardInfo"><p class="cardCategory"></p><h2 class="cardName"></h2><p class="cardDescription"></p><div class="cardActions"><button type="button" class="libraryOpen">Découvrir ↗</button><button type="button" class="libraryActivate">Afficher</button></div></div>';
      card.querySelector('.cardCategory').textContent=APP_NAMES[item.app];card.querySelector('.cardName').textContent=item.name;card.querySelector('.cardDescription').textContent=item.description;
      const favorite=card.querySelector('.libraryFavorite'),saved=libraryFavorites.includes(item.id);favorite.hidden=storeMode;favorite.textContent=saved?'★':'☆';favorite.setAttribute('aria-label',(saved?'Retirer des favoris : ':'Ajouter aux favoris : ')+item.name+' · '+APP_NAMES[item.app]);favorite.setAttribute('aria-pressed',String(saved));favorite.addEventListener('click',()=>toggleLibraryFavorite(item.id));
      card.querySelector('.libraryOpen').setAttribute('aria-label','Découvrir '+item.name+' · '+APP_NAMES[item.app]);card.querySelector('.libraryOpen').addEventListener('click',()=>openLibraryItem(item.id));card.querySelector('.libraryActivate').setAttribute('aria-label',(storeMode?'Ajouter ':'Afficher ')+item.name+' · '+APP_NAMES[item.app]);card.querySelector('.libraryActivate').addEventListener('click',()=>activateLibraryItem(item.id));
      libraryThumbnail(card.querySelector('canvas'),item);grid.append(card);
    }
  }
  const s=data(),app=panelApp(s),visual=s?.visual||s?.display?.visual||'lava';
  grid.querySelectorAll('[data-library-id]').forEach(card=>{const item=items.find(item=>item.id===card.dataset.libraryId),live=Boolean(serverOnline&&!demo&&s&&app===item.app&&(s.contentId||'')===(item.contentId||'')&&(!item.visual||item.visual===visual)),installed=contentItems.find(p=>p.id===item.contentId),compatible=!item.minFirmware||Boolean(stableVersion(s?.version)&&!newerVersion(item.minFirmware,s.version));card.classList.toggle('isLive',live);card.querySelector('.libraryLiveBadge').hidden=!live;const button=card.querySelector('.libraryActivate');button.disabled=demo||!serverOnline||appBusy||uploadBusy||contentBusy||(item.store?installed?.version===item.version||!compatible:live);button.textContent=item.store?!compatible?'Mise à jour requise':installed?.version===item.version?'Ajouté':installed?'Mettre à jour':'Ajouter':live?'À l’écran':appBusy?'En cours…':['message','timer','score'].includes(item.app)&&!item.contentId?'Configurer':'Afficher';});
  document.querySelectorAll('[data-library-filter]').forEach(button=>{const active=button.dataset.libraryFilter===libraryCategory;button.classList.toggle('active',active);button.setAttribute('aria-pressed',String(active));button.hidden=storeMode&&['weather','aviation','favorites'].includes(button.dataset.libraryFilter);});
  for(const [id,on] of[['libraryModeBtn',!storeMode],['storeModeBtn',storeMode]])setPressed(id,on);
  el('hubTitle').textContent=storeMode?'Le Store.':'La bibliothèque.';el('storeRefreshBtn').hidden=!storeMode;el('storeRefreshBtn').disabled=storeBusy;el('restoreLogosBtn').hidden=!storeMode||!contentLogos||contentLogos.installed;el('restoreLogosBtn').disabled=contentBusy||uploadBusy||appBusy||!serverOnline;el('contentCapacity').textContent=contentLoaded?contentItems.length+' / '+contentCapacity+' contenus installés':'';
  el('libraryCount').textContent=items.length+' contenu'+(items.length>1?'s':'');el('libraryEmpty').hidden=items.length>0;el('libraryEmpty').textContent=storeMode?'Le catalogue apparaîtra ici. Actualisez le Store pour réessayer.':libraryCategory==='favorites'&&!libraryFavorites.length?'Votre collection commence ici. Touchez l’étoile d’un contenu pour le retrouver.':'Aucun résultat. Essayez une autre recherche.';el('librarySubtitle').textContent=storeMode?'De nouvelles idées pour votre panneau.':libraryCategory==='favorites'?'Vos favoris, enregistrés sur cette tablette.':'Un écran. Toutes vos envies.';
}
function storeUrl(value){const url=releaseUrl(value);if(!url.href.startsWith(STORE_BASE)||url.search||!url.pathname.endsWith('.json')||url.pathname.includes('%'))throw new Error('Adresse de contenu non autorisée.');return url.href;}
function validateStore(value){if(value?.schema!==1||!Array.isArray(value.items)||value.items.length>64)throw new Error('Catalogue invalide.');const ids=new Set();return value.items.map(item=>{if(!item||!/^[a-z0-9_-]{1,32}$/.test(item.id)||ids.has(item.id)||!['pixel','message','timer'].includes(item.type)||typeof item.title!=='string'||new TextEncoder().encode(item.title).length>64||typeof item.description!=='string'||item.description.length>250||(!Number.isInteger(item.version)||item.version<1||item.version>65535)||!stableVersion(item.minFirmware)||!Number.isInteger(item.size)||item.size<2||item.size>49152||!/^[0-9a-f]{64}$/i.test(item.sha256))throw new Error('Contenu du catalogue invalide.');ids.add(item.id);return{...item,url:storeUrl(item.url),sha256:item.sha256.toLowerCase()};});}
function validateContentPack(pack,item){if(!pack||pack.id!==item.contentId||pack.type!==item.app||pack.version!==item.version||typeof pack.title!=='string'||new TextEncoder().encode(pack.title).length>64)throw new Error('Le contenu ne correspond pas au catalogue.');if(pack.type==='message'){if(typeof pack.text!=='string'||!pack.text.length||pack.text.length>120||/[^\x20-\x7e]/.test(pack.text)||!Number.isInteger(pack.color)||pack.color<0||pack.color>65535)throw new Error('Message invalide.');}else if(pack.type==='timer'){if(!Number.isInteger(pack.durationSeconds)||pack.durationSeconds<1||pack.durationSeconds>5999)throw new Error('Durée invalide.');}else{if(!Number.isInteger(pack.width)||pack.width<1||pack.width>64||!Number.isInteger(pack.height)||pack.height<1||pack.height>64||!Number.isInteger(pack.frameCount)||pack.frameCount<1||pack.frameCount>32||!Number.isInteger(pack.frameMs)||pack.frameMs<80||pack.frameMs>2000||!Array.isArray(pack.palette)||pack.palette.length<1||pack.palette.length>16||pack.palette.some(v=>!Number.isInteger(v)||v<0||v>65535)||typeof pack.pixels!=='string'||pack.pixels.length!==pack.width*pack.height*pack.frameCount||pack.pixels.length>65536||!/^[0-9a-f]+$/i.test(pack.pixels)||Array.from(pack.pixels).some(v=>parseInt(v,16)>=pack.palette.length))throw new Error('Animation pixel invalide.');}return pack;}
async function getContentPack(item){const cache=item.store?storeContentCache:contentCache,cached=cache.get(item.contentId);if(cached?.version===item.version)return cached;let pack;if(item.store){const bytes=await downloadReleaseBytes(storeUrl(item.url),item.size,20000,new AbortController());if(bytes.byteLength!==item.size||typeof sha256!=='function'||sha256(bytes).toLowerCase()!==item.sha256)throw new Error('Le contrôle du contenu a échoué.');pack=JSON.parse(new TextDecoder('utf-8',{fatal:true}).decode(bytes));}else pack=await requestJson('/api/content/data?id='+encodeURIComponent(item.contentId));validateContentPack(pack,item);cache.set(item.contentId,pack);return pack;}
async function openStore(force=false){storeMode=true;libraryCategory='all';showModule('home');contentNotice('');if(storeItems.length&&!force)return;if(storeBusy)return;const run=++storeRun;storeBusy=true;renderLibrary();contentNotice('Chargement du catalogue…');try{const bytes=await downloadReleaseBytes(STORE_BASE+'catalog.json',49152,15000,new AbortController()),items=validateStore(JSON.parse(new TextDecoder('utf-8',{fatal:true}).decode(bytes)));if(run!==storeRun)return;storeItems=items;libraryRenderKey='';contentNotice('Les contenus ajoutés restent disponibles sans Internet.');}catch(error){contentNotice('Store indisponible. Votre bibliothèque reste accessible. '+error.message);}finally{storeBusy=false;renderLibrary();}warmContentThumbnails();}
async function warmContentThumbnails(){const mode=storeMode;for(const item of visibleLibraryItems().filter(item=>item.contentId)){if(currentView!=='home'||storeMode!==mode)return;try{await getContentPack(item);const card=Array.from(el('libraryGrid').children).find(card=>card.dataset.libraryId===item.id);if(card)libraryThumbnail(card.querySelector('canvas'),item);}catch(error){/* The content detail shows a useful retryable error. */}}}
async function installContent(item){if(contentBusy||uploadBusy||appBusy||demo||!serverOnline)return;contentBusy=true;render();contentNotice('Téléchargement et vérification de '+item.name+'…');try{await loadContent();if(!contentToken)throw new Error('Rechargez la connexion au panneau.');if(contentItems.length>=contentCapacity&&!contentItems.some(p=>p.id===item.contentId))throw new Error('La bibliothèque est pleine. Retirez un contenu avant d’en ajouter un autre.');const bytes=await downloadReleaseBytes(storeUrl(item.url),item.size,20000,new AbortController());if(bytes.byteLength!==item.size||typeof sha256!=='function'||sha256(bytes).toLowerCase()!==item.sha256)throw new Error('Le contrôle du contenu a échoué.');validateContentPack(JSON.parse(new TextDecoder('utf-8',{fatal:true}).decode(bytes)),item);const body=new FormData();body.append('file',new Blob([bytes],{type:'application/json'}),item.contentId+'.json');await requestJson('/api/content/install?id='+encodeURIComponent(item.contentId)+'&sha256='+item.sha256+'&size='+item.size,{method:'POST',headers:{'X-Content-Token':contentToken},body},20000);contentCache.delete(item.contentId);await loadContent();contentNotice(item.name+' ajouté à votre bibliothèque. Le panneau conserve son affichage.');el('widgetFeedback').textContent='Ajouté. Retrouvez ce contenu dans Ma bibliothèque pour l’afficher.';}catch(error){contentNotice(error.message);el('widgetFeedback').textContent=error.message;}finally{contentBusy=false;render();}}
async function removeContent(){if(!currentContentId||contentBusy||uploadBusy||appBusy||demo||!serverOnline)return;const id=currentContentId;if(!confirm('Retirer ce contenu de la bibliothèque du panneau ?'))return;contentBusy=true;try{await loadContent();await requestJson('/api/content?id='+encodeURIComponent(id),{method:'DELETE',headers:{'X-Content-Token':contentToken}});contentCache.delete(id);await loadContent();showModule('home');notify('Contenu retiré.');await refresh();}catch(error){el('widgetFeedback').textContent=error.message;}finally{contentBusy=false;render();}}

const APP_NAMES={aviation:'Flight Board',weather:'Météo',clock:'Horloge',animation:'Animations',message:'Message',timer:'Minuteur',score:'Scores',pixel:'Pixel art'};
let currentView='home',appBusy=false,filtersBusy=false,draftVisual=null;
const AIRLINE_OPTIONS=[["logo_dah","AIR ALGÉRIE"],["logo_reu","AIR AUSTRAL"],["logo_aca","AIR CANADA"],["logo_fwi","AIR CARAÏBES"],["logo_vre","AIR CÔTE D’IVOIRE"],["logo_afr","AIR FRANCE"],["logo_mau","AIR MAURITIUS"],["logo_szn","AIR SÉNÉGAL"],["logo_tht","AIR TAHITI NUI"],["logo_tsc","AIR TRANSAT"],["logo_skk","ASKY"],["logo_baw","BRITISH AIRWAYS"],["logo_bel","BRUSSELS AIRLINES"],["logo_crc","CAMAIR-CO"],["logo_crl","CORSAIR"],["logo_bcs","DHL / EAT"],["logo_ezy","EASYJET"],["logo_uae","EMIRATES"],["logo_eth","ETHIOPIAN"],["logo_etd","ETIHAD"],["logo_fbu","FRENCH BEE"],["logo_ibe","IBERIA"],["logo_kqa","KENYA AIRWAYS"],["logo_klm","KLM"],["logo_dlh","LUFTHANSA"],["logo_lgl","LUXAIR"],["logo_lbt","NOUVELAIR"],["logo_qtr","QATAR AIRWAYS"],["logo_ram","ROYAL AIR MAROC"],["logo_rwd","RWANDAIR"],["logo_ryr","RYANAIR"],["logo_swr","SWISS"],["logo_tra","TRANSAVIA"],["logo_tar","TUNISAIR"],["logo_thy","TURKISH AIRLINES"],["logo_ups","UPS AIRLINES"]];
function panelApp(s){return APP_NAMES[s?.app||s?.display?.app]?s.app||s.display.app:'aviation';}
function activeFilters(s){const f=s?.filters||{};return Boolean(f.logoOnly||f.airline||f.type||Number(f.maxAltitudeFt)>0);}
function showModule(view){if(!APP_NAMES[view]&&view!=='home')return;if(demo&&view!=='aviation')setDemo(false);currentView=view;draftVisual=null;currentContentId='';widgetEditorKey='';el('moduleFeedback').textContent='';render();window.scrollTo({top:0,behavior:'auto'});}
function weatherDescription(w){const c=w?.code;if(!Number.isInteger(c))return['Données indisponibles','☁'];if(c===0)return[w?.isDay===false?'Ciel dégagé':'Ensoleillé',w?.isDay===false?'☾':'☀'];if(c===1||c===2)return['Éclaircies','☀'];if(c===3)return['Nuageux','☁'];if(c===45||c===48)return['Brouillard','≋'];if([51,53,55,56,57].includes(c))return['Bruine','☂'];if([61,63,65,66,67,80,81,82].includes(c))return['Pluie','☂'];if([71,73,75,77,85,86].includes(c))return['Neige','❄'];if([95,96,99].includes(c))return['Orage','ϟ'];return['Données indisponibles','☁'];}
function weatherAgeSeconds(w,s){const base=w?.ageSeconds;if(typeof base!=='number'||!Number.isFinite(base)||base<0)return -1;const elapsed=s===DEMO?0:Math.floor((Date.now()-lastStateAt)/1000);return Math.min(31536000,base+(Number.isFinite(elapsed)?Math.max(0,elapsed):0));}
let contentLogos=null;
const contentPending=new Set();
async function ensureActiveContent(){const id=latest?.contentId;if(!id||contentPending.has(id))return;const item=contentItems.find(p=>p.id===id);if(!item||contentCache.get(id)?.version===item.version)return;contentPending.add(id);try{await getContentPack(contentItem(item));drawPreview();}catch(error){/* Preserve the last known content and offer library reload. */}finally{contentPending.delete(id);}}
async function restoreLogos(){if(contentBusy||uploadBusy||appBusy||demo||!serverOnline)return;contentBusy=true;renderLibrary();try{await loadContent();const asset=contentLogos,url=asset&&releaseUrl(asset.url);if(!asset||!url||url.href!==STORE_BASE+'zcs-logos.bin'||!Number.isInteger(asset.size)||asset.size<1||asset.size>49152||!/^[a-f0-9]{64}$/i.test(asset.sha256)||!contentToken)throw new Error('Le pack de logos est indisponible.');contentNotice('Téléchargement des logos…');const bytes=await downloadReleaseBytes(url.href,asset.size,20000,new AbortController());if(bytes.byteLength!==asset.size||typeof sha256!=='function'||sha256(bytes).toLowerCase()!==asset.sha256.toLowerCase())throw new Error('Le contrôle des logos a échoué.');const body=new FormData();body.append('file',new Blob([bytes]),'zcs-logos.bin');await requestJson('/api/content/install?id=__logos&size='+asset.size+'&sha256='+asset.sha256,{method:'POST',headers:{'X-Content-Token':contentToken},body},20000);logoImageId='';logoImageFailedAt=0;await loadContent();contentNotice('Logos restaurés.');}catch(error){contentNotice('Logos non installés. '+error.message+' Réessayez lorsque le panneau est disponible.');}finally{contentBusy=false;render();}}

let widgetEditorKey='',widgetDirty=false,widgetDraft=null,widgetBusy=false,scoreBusy=false,scoreMatches=[],scoreSearchRun=0;
const WIDGET_APPS=['message','timer','score','pixel'];
function widgetView(){return WIDGET_APPS.includes(currentView);}
function asciiMessage(text){return String(text).normalize('NFD').replace(/[\u0300-\u036f]/g,'').toUpperCase();}
function widgetRemaining(w,s=data()){const n=Number(w?.remainingSeconds??w?.durationSeconds)||0;return Math.max(0,n-(w?.running&&s===latest?Math.max(0,Math.floor((Date.now()-lastStateAt)/1000)):0));}
function timeLabel(n){n=Math.max(0,Math.floor(n));return String(Math.floor(n/60)).padStart(2,'0')+':'+String(n%60).padStart(2,'0');}
function widgetPayload(){if(currentView==='message'){const text=asciiMessage(el('messageText').value).trim(),hex=parseInt(el('messageColor').value.slice(1),16),color=((hex>>19)&31)<<11|((hex>>10)&63)<<5|(hex>>3)&31;if(!text||text.length>120||/[^\x20-\x7e]/.test(text))throw new Error('Saisissez de 1 à 120 caractères simples, sans emoji.');return{app:'message',text,color};}if(currentView==='timer'){const minutes=Number(el('timerMinutes').value),seconds=Number(el('timerSeconds').value),durationSeconds=minutes*60+seconds;if(!Number.isInteger(minutes)||!Number.isInteger(seconds)||minutes<0||minutes>99||seconds<0||seconds>59||durationSeconds<1)throw new Error('Choisissez une durée de 1 seconde à 99 min 59 s.');return{app:'timer',durationSeconds};}const league=el('scoreLeague').value,date=el('scoreDate').value.replace(/-/g,''),eventId=el('scoreEvent').value;if(!['fra.1','eng.1','uefa.champions'].includes(league)||!/^\d{8}$/.test(date)||!/^\d{5,12}$/.test(eventId))throw new Error('Choisissez un match dans la liste.');return{app:'score',league,date,eventId};}
function updateWidgetDraft(){if(currentContentId||!widgetView()||currentView==='pixel')return;widgetDirty=true;try{const payload=widgetPayload();widgetDraft={...data()?.widget,...payload};if(currentView==='timer')Object.assign(widgetDraft,{remainingSeconds:payload.durationSeconds,running:false});if(currentView==='score'){const match=scoreMatches.find(m=>m.id===payload.eventId);if(match)Object.assign(widgetDraft,match.widget);}}catch(error){widgetDraft=null;}renderModules();drawPreview();}
function fillWidgetEditor(){const item=selectedContent(),pack=currentContentId?selectedContentPack():null,key=currentView+'|'+currentContentId+'|'+(pack?.version||'')+'|'+Boolean(data());if(widgetEditorKey===key)return;widgetEditorKey=key;widgetDirty=false;widgetDraft=null;el('widgetFeedback').textContent='';const w=pack||data()?.widget||{};if(currentView==='message'){el('messageText').value=w.text||'';const color=Number(w.color??0xafd8),rgb=[Math.round(((color>>11)&31)*255/31),Math.round(((color>>5)&63)*255/63),Math.round((color&31)*255/31)];el('messageColor').value='#'+rgb.map(v=>v.toString(16).padStart(2,'0')).join('');}else if(currentView==='timer'){const duration=Number(w.durationSeconds)||300;el('timerMinutes').value=Math.floor(duration/60);el('timerSeconds').value=duration%60;}else if(currentView==='score'){el('scoreLeague').value=w.league||'fra.1';const date=w.date&&/^\d{8}$/.test(w.date)?w.date:new Date().getFullYear()+String(new Date().getMonth()+1).padStart(2,'0')+String(new Date().getDate()).padStart(2,'0');el('scoreDate').value=date.slice(0,4)+'-'+date.slice(4,6)+'-'+date.slice(6,8);el('scoreEvent').replaceChildren(new Option(w.eventId?(w.home||'Domicile')+' — '+(w.away||'Extérieur'):'Recherchez une date',w.eventId||''));el('scoreSearchStatus').textContent='';}if(!currentContentId&&currentView!=='pixel')try{const payload=widgetPayload();widgetDraft={...w,...payload};if(currentView==='timer')Object.assign(widgetDraft,{remainingSeconds:payload.durationSeconds,running:false});}catch(error){widgetDraft=null;}}
function renderWidgetModule(blocked){const isWidget=widgetView();el('widgetContent').hidden=!isWidget;if(!isWidget)return;fillWidgetEditor();const item=selectedContent(),pack=currentContentId?selectedContentPack():null,installed=currentContentId&&contentItems.some(p=>p.id===currentContentId),w=data()?.widget||{},active=panelApp(data())===currentView&&(data()?.contentId||'')===currentContentId;
 el('weatherContent').hidden=true;el('clockContent').hidden=true;el('animationContent').hidden=true;el('visualChoices').hidden=true;el('moduleEyebrow').textContent=currentContentId?'Votre collection':'À votre façon';el('moduleHeading').textContent=item?.title||APP_NAMES[currentView];el('moduleOptionsTitle').textContent='Aperçu sur le panneau';el('moduleHint').textContent=currentContentId?'Ce contenu est conservé sur votre panneau après installation.':'Configurez ce contenu, puis choisissez « Afficher sur le panneau ».';el('moduleSettingsBtn').textContent='Luminosité et nuit';
 for(const kind of['message','timer','score'])el(kind+'Fields').hidden=currentView!==kind||Boolean(currentContentId);el('widgetForm').hidden=currentView==='pixel';el('widgetSaveRow').hidden=Boolean(currentContentId);el('widgetSaveBtn').disabled=blocked||widgetBusy||contentBusy||currentView==='score'&&!el('scoreEvent').value;document.querySelectorAll('#widgetForm input,#widgetForm textarea,#widgetForm select').forEach(input=>input.disabled=widgetBusy);el('scoreSearchBtn').disabled=scoreBusy||widgetBusy;el('timerControls').hidden=currentView!=='timer'||!active;el('timerReadout').textContent=timeLabel(widgetRemaining(w));el('timerState').textContent=w.running?'En cours':Number(w.remainingSeconds)===0?'Terminé':'En pause';document.querySelectorAll('[data-timer]').forEach(button=>{button.disabled=blocked||widgetBusy||contentBusy||!active||(button.dataset.timer==='pause'?!w.running:button.dataset.timer==='resume'?w.running||!Number(w.remainingSeconds):false);});
 el('pixelInfo').hidden=!currentContentId;el('pixelInfo').textContent=!pack?'Chargement du contenu…':currentView==='pixel'?pack.width+' × '+pack.height+' pixels · '+pack.frameCount+' images':currentView==='message'?pack.text:'Durée : '+timeLabel(pack.durationSeconds);el('contentRemoveBtn').hidden=!installed;el('contentRemoveBtn').disabled=blocked||contentBusy;
 if(currentContentId&&storeMode){const store=storeItems.find(p=>p.id===currentContentId);el('activateBtn').textContent=installed?'✓ Dans la bibliothèque':'Ajouter à la bibliothèque';el('activateBtn').disabled=blocked||contentBusy||Boolean(installed)||!store||Boolean(store.minFirmware&&(!stableVersion(data()?.version)||newerVersion(store.minFirmware,data().version)));}
 if(!currentContentId&&currentView==='score'&&!el('scoreEvent').value)el('activateBtn').disabled=true;
}
async function saveWidget(showMessage=true){if(widgetBusy||uploadBusy||contentBusy||demo||!serverOnline||currentContentId)return false;let payload;try{payload=widgetPayload();}catch(error){el('widgetFeedback').textContent=error.message;return false;}const editor=widgetEditorKey;widgetBusy=true;renderModules();try{await post('/api/widget',payload);if(widgetEditorKey===editor&&currentView===payload.app&&!currentContentId){widgetDirty=false;widgetDraft=null;}await refresh();if(showMessage&&widgetEditorKey===editor)el('widgetFeedback').textContent='Enregistré. L’application active reste inchangée.';return true;}catch(error){el('widgetFeedback').textContent=error.message;return false;}finally{widgetBusy=false;renderModules();}}
async function timerCommand(action){if(widgetBusy||contentBusy||uploadBusy||demo||!serverOnline)return;widgetBusy=true;renderModules();try{await post('/api/timer',{action});await refresh();}catch(error){el('widgetFeedback').textContent=error.message;}finally{widgetBusy=false;renderModules();}}
function resetScoreSearch(){++scoreSearchRun;scoreMatches=[];el('scoreEvent').replaceChildren(new Option('Recherchez cette date',''));el('scoreSearchStatus').textContent='';updateWidgetDraft();}
async function searchMatches(){if(scoreBusy)return;const league=el('scoreLeague').value,date=el('scoreDate').value.replace(/-/g,'');if(!['fra.1','eng.1','uefa.champions'].includes(league)||!/^\d{8}$/.test(date)){el('scoreSearchStatus').textContent='Choisissez une date.';return;}const run=++scoreSearchRun;scoreBusy=true;el('scoreSearchStatus').textContent='Recherche des matchs ESPN…';renderModules();try{const url='https://site.api.espn.com/apis/site/v2/sports/soccer/'+league+'/scoreboard?dates='+date,bytes=await downloadReleaseBytes(url,786432,15000,new AbortController()),result=JSON.parse(new TextDecoder('utf-8',{fatal:true}).decode(bytes));if(run!==scoreSearchRun)return;if(!Array.isArray(result.events))throw new Error('Réponse ESPN invalide.');scoreMatches=result.events.slice(0,100).map(event=>{const competitors=event.competitions?.[0]?.competitors||[],home=competitors.find(c=>c.homeAway==='home'),away=competitors.find(c=>c.homeAway==='away');if(!home?.team||!away?.team||!/^\d{5,12}$/.test(event.id))return null;const state=event.status?.type?.state,status=state==='pre'?'A VENIR':state==='post'?'TERMINE':event.status?.displayClock||'EN COURS',homeScore=Number(home.score),awayScore=Number(away.score);return{id:event.id,name:String(home.team.displayName)+' — '+String(away.team.displayName),widget:{home:asciiMessage(home.team.abbreviation||home.team.shortDisplayName).slice(0,12),away:asciiMessage(away.team.abbreviation||away.team.shortDisplayName).slice(0,12),homeScore:Number.isInteger(homeScore)?homeScore:0,awayScore:Number.isInteger(awayScore)?awayScore:0,status,available:Number.isInteger(homeScore)&&Number.isInteger(awayScore),stale:false}};}).filter(Boolean);el('scoreEvent').replaceChildren(new Option(scoreMatches.length?'Choisissez un match':'Aucun match ce jour',''));scoreMatches.forEach(match=>el('scoreEvent').add(new Option(match.name,match.id)));el('scoreSearchStatus').textContent=scoreMatches.length?scoreMatches.length+' match'+(scoreMatches.length>1?'s':'')+' trouvé'+(scoreMatches.length>1?'s':'')+'.':'Aucun match annoncé pour cette compétition et cette date.';updateWidgetDraft();}catch(error){if(run===scoreSearchRun)el('scoreSearchStatus').textContent='Liste ESPN indisponible. Vérifiez Internet puis réessayez.';}finally{scoreBusy=false;renderModules();}}

function renderModules(){
  const s=data(),app=panelApp(s),home=currentView==='home',aviation=currentView==='aviation',d=s?.display||{},blocked=appBusy||uploadBusy||contentBusy||widgetBusy||(!demo&&!serverOnline),visual=draftVisual||s?.visual||d.visual||'lava';
  el('hub').hidden=!home;el('aviationApp').hidden=!aviation;el('moduleView').hidden=home||aviation;el('moduleBar').hidden=home;el('demoBtn').hidden=!aviation;
  const title=APP_NAMES[currentView]||'ZCS CONTROL';el('moduleTitle').textContent=title;el('homeBtn').textContent=storeMode?'‹ Store':'‹ Bibliothèque';el('moduleView').setAttribute('aria-label',title);
  const onPanel=app===currentView&&(s?.contentId||'')===currentContentId&&!previewIsDraft();el('activateBtn').disabled=blocked||onPanel;el('activateBtn').textContent=appBusy?'En cours…':onPanel?'✓ Sur le panneau':'Afficher sur le panneau';
  document.querySelectorAll('[data-nav]').forEach(button=>{const active=button.dataset.nav===currentView;button.classList.toggle('active',active);if(active)button.setAttribute('aria-current','page');else button.removeAttribute('aria-current');});if(home)renderLibrary();
  document.querySelectorAll('[data-module]').forEach(button=>{button.querySelector('.moduleTag').hidden=!s||app!==button.dataset.module;});
  el('activeAppLabel').textContent=!s?'Connexion au panneau…':APP_NAMES[app]+(d.sleeping?' · En veille':'');el('hubPanelState').textContent=!serverOnline?'Le panneau est hors ligne.':d.panelReady===false?'Le pilote du panneau est indisponible.':'Vous pouvez explorer les applications sans interrompre cet affichage.';
  const host=el(home?'hubPreviewHost':aviation?'aviationPreviewHost':'modulePreviewHost');if(el('previewToggle').parentElement!==host)host.append(el('previewToggle'));if(home)el('previewToggle').open=true;
  el('sourceLabel').textContent=aviation?'Positions ADSB.lol · Trajets VRS':'by Christ_Kmusic · ZCS CONTRÔLE';
  const weather=currentView==='weather',clock=currentView==='clock';el('weatherContent').hidden=!weather;el('clockContent').hidden=!clock;el('animationContent').hidden=weather||clock;el('visualChoices').hidden=weather;
  el('moduleEyebrow').textContent=weather?'Météo locale':clock?'L’heure, simplement':'Pixels en mouvement';el('moduleHeading').textContent=weather?(s?.place||'Votre lieu'):title;
  el('moduleOptionsTitle').textContent=weather?'Le temps sur votre panneau':'Ambiance';el('moduleHint').textContent=onPanel?(weather?'Le décor s’anime avec les conditions : soleil, nuages, pluie ou neige. Les données se mettent à jour automatiquement.':'Cette ambiance est affichée sur votre panneau.'):weather?'Affichez Météo pour voir les conditions et leur animation : soleil, nuages, pluie ou neige.':'Choisissez une ambiance, puis affichez cette application sur le panneau.';
  el('moduleSettingsBtn').textContent=weather?'Modifier le lieu':clock?'Fuseau horaire et nuit':'Luminosité et nuit';el('moduleSettingsBtn').disabled=blocked;
  el('plainVisualLabel').textContent=currentView==='animation'?'Écran noir':'Minimal';
  document.querySelectorAll('[data-visual]').forEach(button=>{const on=button.dataset.visual===visual;button.classList.toggle('active',on);button.setAttribute('aria-pressed',String(on));button.disabled=blocked;});
  el('moduleClock').textContent=d.timeSynced?d.clock||'--:--':'--:--';el('moduleClockDate').textContent=d.timeSynced?d.date||'Heure synchronisée':'Synchronisation de l’heure…';
  const w=s?.weather||{},has=(w.available===true||['ready','stale'].includes(w.status))&&typeof w.temperatureC==='number'&&Number.isFinite(w.temperatureC),description=weatherDescription(w),seconds=weatherAgeSeconds(w,s),offline=!demo&&!serverOnline,stale=offline||w.stale===true||w.status==='stale'||seconds<0||seconds>=1800;
  el('weatherTemp').textContent=has?fmt(Math.round(w.temperatureC)||0,0)+'°':'—';el('weatherIcon').textContent=has?description[1]:'☁';el('weatherCondition').textContent=has?description[0]:w.status==='error'?'Météo temporairement indisponible':app==='weather'?'Chargement de la météo…':'Prêt à afficher la météo';
  el('weatherWind').textContent=has&&typeof w.windKmh==='number'&&Number.isFinite(w.windKmh)?fmt(w.windKmh)+' km/h':'— km/h';el('weatherAge').textContent=has?(offline?'Panneau hors ligne · ':'')+(stale?'Dernier relevé · ':'Actualisé · ')+(seconds>=0&&Number.isFinite(seconds)?seconds<60?'à l’instant':'il y a '+Math.floor(seconds/60)+' min':'âge inconnu'):w.status==='error'?'Nouvelle tentative automatique.':'Le lieu est partagé avec Flight Board.';
  renderWidgetModule(blocked);
  const filtered=activeFilters(s);el('filtersBtn').textContent=filtered?'Filtres actifs':'Filtres';el('filterNotice').hidden=!filtered;el('filterNotice').textContent='Filtres appliqués au radar et au panneau · '+aircraft(s).length+' / '+(s?.totalCount??aircraft(s).length)+' avions';el('filtersBtn').disabled=blocked;
}
async function activateModule(){if(appBusy||widgetBusy||contentBusy||uploadBusy||demo||!serverOnline||!APP_NAMES[currentView])return;if(currentContentId&&storeMode){const item=storeItems.find(p=>p.id===currentContentId);if(item)await installContent(contentItem(item,true));return;}const app=currentView,contentId=currentContentId,visual=draftVisual,payload={app};if(widgetView()&&!contentId&&app!=='pixel'&&!await saveWidget(false))return;if(currentView!==app||currentContentId!==contentId||draftVisual!==visual)return;if(contentId)payload.contentId=contentId;if((app==='clock'||app==='animation')&&visual)payload.visual=visual;appBusy=true;el('moduleFeedback').textContent='';renderModules();try{await post('/api/app',payload);if(currentView===payload.app&&draftVisual===(payload.visual||null))draftVisual=null;notify('Sur le panneau : '+APP_NAMES[app]+'.');await refresh();}catch(error){el('moduleFeedback').textContent=error.message;notify(error.message);}finally{appBusy=false;render();}}
async function chooseVisual(visual){if(appBusy||uploadBusy||!serverOnline)return;const view=currentView,chosenDraft=draftVisual;if(panelApp(data())!==currentView||previewIsDraft()){draftVisual=visual;el('moduleFeedback').textContent='Ambiance choisie. Appuyez sur « Afficher sur le panneau » pour l’activer.';render();return;}appBusy=true;el('moduleFeedback').textContent='';renderModules();try{await post('/api/visual',{visual});if(currentView===view&&draftVisual===chosenDraft)draftVisual=null;await refresh();}catch(error){el('moduleFeedback').textContent=error.message;}finally{appBusy=false;render();}}
function fillFilters(f){el('filterLogo').checked=Boolean(f.logoOnly);el('filterAirline').value=f.airline||'';el('filterType').value=f.type||'';const altitude=String(f.maxAltitudeFt||0),select=el('filterAltitude');if(!Array.from(select.options).some(o=>o.value===altitude))select.add(new Option(fmt(Number(altitude))+' ft',altitude));select.value=altitude;}
function openFilters(){if(uploadBusy||filtersBusy)return;fillFilters(data()?.filters||{});el('filtersFeedback').textContent='';el('filtersDialog').showModal();}
function syncDemoDisplay(){const planes=aircraft(DEMO),d=DEMO.display;if(d.mode==='clock'){d.activeHex='';return;}if(d.mode==='manual'||d.mode==='selected'){if(planes.some(p=>key(p)===d.activeHex))return;d.mode='auto';}d.activeHex=planes.length?key(planes[0]):'';}
async function saveFilters(reset=false){if(filtersBusy||uploadBusy||(!demo&&!serverOnline))return;const filters=reset?{logoOnly:false,airline:'',type:'',maxAltitudeFt:0}:{logoOnly:el('filterLogo').checked,airline:el('filterAirline').value,type:el('filterType').value.trim().toUpperCase(),maxAltitudeFt:Number(el('filterAltitude').value)};if(filters.type&&!/^[A-Z0-9]{2,4}$/.test(filters.type)){el('filtersFeedback').textContent='Le type doit contenir de 2 à 4 lettres ou chiffres, par exemple A320.';return;}filtersBusy=true;el('filtersApplyBtn').disabled=true;el('filtersResetBtn').disabled=true;try{if(demo){DEMO.filters=filters;syncDemoDisplay();notify('Filtres appliqués à la démonstration.');}else await post('/api/aviation/filters',filters);selectedHex='';el('filtersDialog').close();await refresh();}catch(error){el('filtersFeedback').textContent=error.message;}finally{filtersBusy=false;el('filtersApplyBtn').disabled=false;el('filtersResetBtn').disabled=false;render();}}

function notify(message){el('message').textContent=message;}
function age(s){return s===DEMO?0:Number(s?.ageSeconds)+Math.max(0,Math.floor((Date.now()-lastStateAt)/1000));}
function usable(s){return Boolean(s&&(demo||(serverOnline&&s.wifiConnected!==false&&s.radarOk&&Number(s.ageSeconds)>=0&&age(s)<=90)));}
function radarNotice(s){
  const notice=(label,title,detail)=>({label,title,detail});
  if(demo)return notice('Démonstration','Démonstration locale','Les avions affichés sont fictifs.');
  if(!serverOnline)return notice('Flight Board hors ligne','Flight Board hors ligne','Vérifiez la connexion entre la tablette et le Flight Board.');
  const r=s?.radar||{};
  if(s?.wifiConnected===false||r.error==='wifi')return notice('Wi-Fi à reconnecter','Wi-Fi à reconnecter','Ouvrez les réglages pour reconnecter le Flight Board.');
  if(r.status==='paused')return r.pauseReason==='wifi'?notice('Radar suspendu','Configuration Wi-Fi','Le radar reprendra quelques secondes après la fermeture de la configuration Wi-Fi.'):r.pauseReason==='update'?notice('Radar suspendu','Mise à jour en cours','Le radar reprendra après la mise à jour.'):notice('Radar suspendu','Relevés radar suspendus','En attente de la reprise des relevés.');
  if(usable(s))return notice('En direct','Positions à jour','Touchez un avion pour voir sa fiche.');
  if(r.status==='starting')return notice('Démarrage radar','Le radar démarre','Le premier relevé des positions se prépare.');
  if(r.status==='fetching')return notice('Relevé en cours','Recherche des positions','Le Flight Board attend la réponse du service radar.');
  if(r.status==='unavailable'||r.error==='worker')return notice('Radar indisponible','Le radar n’a pas pu démarrer','Redémarrez le Flight Board pour relancer le radar.');
  let result;
  if(r.error==='http'){
    const code=Number(r.httpStatus);
    result=code===429?notice('Radar limité','Limite temporaire du service','ADSB.lol limite temporairement les demandes. Le radar reprendra automatiquement à un rythme réduit.'):
      code>=500?notice('Service indisponible','Service radar indisponible','Le service radar rencontre une erreur (HTTP '+code+').'):
      notice('Erreur du service radar','Requête radar refusée','Le service radar a répondu HTTP '+code+'.');
  }else{
    const errors={
      http_begin:['Connexion radar impossible','Connexion au service radar impossible','Le Flight Board n’a pas pu ouvrir la connexion au service radar.'],
      network:['Connexion radar impossible','Connexion au service radar impossible','Le Flight Board n’a pas pu joindre le service radar.'],
      timeout:['Radar trop lent','Délai de réponse dépassé','Le service radar n’a pas terminé sa réponse à temps.'],
      empty_response:['Réponse radar vide','Réponse radar vide','Le service radar n’a transmis aucune position exploitable.'],
      json_empty:['Réponse radar vide','Réponse radar vide','Le service radar n’a transmis aucune position exploitable.'],
      json_incomplete:['Lecture interrompue','Lecture des positions interrompue','La réponse du service radar est arrivée incomplète.'],
      response_too_large:['Réponse trop grande','Réponse radar trop grande','Le relevé dépasse la capacité de lecture du Flight Board.'],
      json_invalid:['Données invalides','Données radar illisibles','Le format de la réponse radar n’a pas pu être lu.'],
      schema:['Données invalides','Données radar inattendues','La réponse du service radar ne contient pas les données attendues.'],
      json_memory:['Mémoire insuffisante','Mémoire radar insuffisante','Le Flight Board manque de mémoire pour lire les positions.'],
      memory:['Mémoire insuffisante','Mémoire radar insuffisante','Le Flight Board manque de mémoire pour charger les positions.']
    };
    if(errors[r.error])result=notice(...errors[r.error]);
    else if(r.status==='error')result=notice('Relevé échoué','Relevé radar impossible','Le dernier relevé des positions a échoué.');
    else if(s?.radarOk&&Number(s.ageSeconds)>=0&&age(s)>90)result=notice('Positions expirées','Positions à actualiser','Les dernières positions sont trop anciennes pour être affichées.');
    else result=notice('Radar en attente','Positions indisponibles','En attente de nouvelles positions.');
  }
  const retry=Number(r.retrySeconds);
  if(Number.isFinite(retry)&&retry>=0){const seconds=Math.max(0,Math.ceil(retry)-Math.max(0,Math.floor((Date.now()-lastStateAt)/1000)));result.detail+=' '+(seconds?'Nouvelle tentative dans '+seconds+' s.':'Nouvelle tentative attendue.');}
  return result;
}
function aircraft(s){const planes=Array.isArray(s?.aircraft)?s.aircraft:[];if(s!==DEMO)return planes;const f=s.filters||{},ids={AFR:'logo_afr',RYR:'logo_ryr',UPS:'logo_ups'};return planes.filter(p=>(!f.logoOnly||ids[p.callsign.slice(0,3)])&&(!f.airline||ids[p.callsign.slice(0,3)]===f.airline)&&(!f.type||p.type===f.type)&&(!f.maxAltitudeFt||(p.altitudeAvailable&&p.altitudeFt<=f.maxAltitudeFt)));}
function selectedPlane(s){const planes=aircraft(s);return selectedHex?planes.find(p=>key(p)===selectedHex)||null:planes[0]||null;}
async function post(path,body){return requestJson(path,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});}
function setPressed(id,on){el(id).classList.toggle('active',on);el(id).setAttribute('aria-pressed',String(on));}
// Load the tablet image separately from the 24 × 16 LED pixels. A response
// for a previous selection must never replace the current airline's logo.
function renderAirlineLogo(p){
  const id=String(p?.callsign||'').trim().toUpperCase();
  if(id===logoImageId&&(!logoImageFailedAt||Date.now()-logoImageFailedAt<30000)){if(logoImageFailedAt&&logoCache.has(id)){paintLogo(el('logoPixels'),logoCache.get(id));el('logoPixels').hidden=false;el('logoFallback').hidden=true;}return;}
  logoImageId=id;logoImageFailedAt=0;const epoch=++logoImageEpoch;
  const target=el('logo'),fallback=el('logoFallback');el('logoPixels').hidden=true;target.hidden=true;target.removeAttribute('src');target.alt='';fallback.hidden=false;
  if(!id)return;
  const source='/api/logo/image?flight='+encodeURIComponent(id)+'&v=2.0.5',image=new Image();
  image.onload=()=>{if(epoch!==logoImageEpoch)return;target.src=source;target.alt=p.airline?'Logo '+p.airline:'Logo de la compagnie';target.hidden=false;fallback.hidden=true;};
  image.onerror=()=>{if(epoch===logoImageEpoch){logoImageFailedAt=Date.now();if(logoCache.has(id)){paintLogo(el('logoPixels'),logoCache.get(id));el('logoPixels').hidden=false;fallback.hidden=true;}else fetchLogo(p);}};
  image.src=source;
}
function paintLogo(canvas,pixels){if(!Array.isArray(pixels)||pixels.length!==384)return;const g=canvas.getContext('2d'),img=g.createImageData(24,16);pixels.forEach((v,i)=>{img.data[i*4]=((v>>11)&31)*255/31;img.data[i*4+1]=((v>>5)&63)*255/63;img.data[i*4+2]=(v&31)*255/31;img.data[i*4+3]=255;});g.putImageData(img,0,0);}
async function fetchLogo(p){const id=p.callsign||'';if(!id||logoCache.has(id)||pendingLogos.has(id)||Date.now()-(logoFailed.get(id)||0)<30000)return;pendingLogos.add(id);try{const result=await requestJson('/api/logo?flight='+encodeURIComponent(id));if(Array.isArray(result.pixels)&&result.pixels.length===384){if(logoCache.size>=64)logoCache.delete(logoCache.keys().next().value);logoCache.set(id,result.pixels);}else logoFailed.set(id,Date.now());}catch(error){logoFailed.set(id,Date.now());}finally{pendingLogos.delete(id);renderDetails();drawPreview();}}
// Callsigns can change while the same transponder remains visible. Match both
// fields, and never apply a response to a different selection or demo session.
function routeKey(p){return String(p?.hex||'')+'|'+String(p?.callsign||'');}
function routeFor(p){
  if(!p)return {status:'not_requested'};
  if(p.route&&p.route.status!=='not_requested')return p.route;
  const cached=routeCache.get(routeKey(p));
  if(cached&&Date.now()-cached.at<(cached.route.status==='error'?30000:60000))return cached.route;
  return p.route||{status:'not_requested'};
}
function normalizeRoute(route){
  if(!route||!['not_requested','loading','known','unconfirmed','unknown','error'].includes(route.status))return {status:'error'};
  if(route.status!=='known')return {status:route.status,reason:String(route.reason||'')};
  const airport=value=>{if(!value||!/^[A-Z0-9]{3,4}$/.test(String(value.code||'')))return null;return {code:String(value.code),city:String(value.city||'').slice(0,100),name:String(value.name||'').slice(0,160)};};
  const origin=airport(route.origin),destination=airport(route.destination);
  return origin&&destination?{status:'known',source:route.source==='adsb.lol-vrs'?'adsb.lol-vrs':'adsbdb',indicative:true,origin,destination}:{status:'unknown'};
}
function renderRoute(p){
  const route=routeFor(p),known=route.status==='known'&&route.origin&&route.destination;
  el('routeAirports').hidden=!known;
  if(known){
    ['origin','destination'].forEach(side=>{const airport=route[side],prefix=side==='origin'?'routeOrigin':'routeDestination';el(prefix+'Code').textContent=airport.code;el(prefix+'City').textContent=airport.city||airport.name||'Aéroport non renseigné';el(prefix+'City').title=airport.name||airport.city||'';});
    el('route').setAttribute('aria-label','Provenance '+route.origin.code+', destination '+route.destination.code);
  }else el('route').setAttribute('aria-label','Provenance et destination');
  el('routeNote').textContent=known?(demo?'Trajet fictif · démonstration':'Itinéraire indicatif'):route.status==='unconfirmed'?'Trajet non confirmé':route.status==='unknown'?'Trajet non renseigné':route.status==='error'?'Trajet temporairement indisponible':'Recherche du trajet…';
  el('routeNote').title=known&&!demo?'Correspondance par indicatif · '+(route.source==='adsb.lol-vrs'?'VRS / ADSB.lol':'ADSBDB')+' · Le trajet de ce vol n’est pas confirmé en temps réel.':route.status==='unconfirmed'?'Le trajet proposé ne peut pas être retenu avec la position et la direction actuelles de cet avion.':'';
}
function routeCandidates(){
  const s=data();if(demo||uploadBusy||wifiIsOpen()||!usable(s)||panelApp(s)!=='aviation')return [];
  const selected=selectedPlane(s),active=s?.display?.mode!=='clock'&&!s?.display?.sleeping?previewPlane(s):null;
  return [selected,active].filter((p,i,list)=>p&&list.findIndex(other=>other&&routeKey(other)===routeKey(p))===i);
}
function syncRoutes(){
  const candidates=routeCandidates(),focus=candidates.map(routeKey).join('~');
  if(focus!==routeFocus){routeFocus=focus;++routeEpoch;}
  if(routeBusy||!candidates.length)return;
  const p=candidates.filter(plane=>['not_requested','loading'].includes(routeFor(plane).status)&&Date.now()-(routeLastRequest.get(routeKey(plane))||0)>=4000).sort((a,b)=>(routeLastRequest.get(routeKey(a))||0)-(routeLastRequest.get(routeKey(b))||0))[0];
  if(p)fetchRoute(p,routeEpoch);
}
async function fetchRoute(p,epoch){
  const identity=routeKey(p);routeBusy=true;routeLastRequest.set(identity,Date.now());
  if(routeLastRequest.size>80)routeLastRequest.delete(routeLastRequest.keys().next().value);
  const stillRelevant=()=>epoch===routeEpoch&&routeCandidates().some(plane=>routeKey(plane)===identity);
  try{
    const result=await requestJson('/api/route?hex='+encodeURIComponent(p.hex),{},6000);
    if(!stillRelevant()||result.hex!==p.hex||result.callsign!==p.callsign)return;
    const route=normalizeRoute(result.route);routeCache.set(identity,{route,at:Date.now()});
    const current=aircraft(latest).find(plane=>routeKey(plane)===identity);if(current)current.route=route;
    if(routeCache.size>80)routeCache.delete(routeCache.keys().next().value);
  }catch(error){
    if(stillRelevant()){const route={status:'error'};routeCache.set(identity,{route,at:Date.now()});const current=aircraft(latest).find(plane=>routeKey(plane)===identity);if(current)current.route=route;}
  }finally{
    routeBusy=false;
    if(stillRelevant()){renderRoute(selectedPlane(data()));drawPreview();}
  }
}
function project(p,s){let dlon=p.lon-s.homeLon;if(dlon>180)dlon-=360;if(dlon< -180)dlon+=360;const north=(p.lat-s.homeLat)*111.32,east=dlon*111.32*Math.cos(s.homeLat*Math.PI/180);return{x:50+east/s.radiusKm*48,y:50-north/s.radiusKm*48};}
function selectPlane(p){if(!usable(data()))return;selectedHex=key(p);render();}
function renderRadar(){
  const s=data(),good=usable(s),planes=good?aircraft(s):[],selected=good&&selectedPlane(s),notice=radarNotice(s);
  const focused=document.activeElement?.dataset?.plane,focusArea=document.activeElement?.closest('#flightList')?'flightList':'planeLayer';
  const scroll=el('flightList').scrollTop;
  el('planeLayer').replaceChildren();el('flightList').replaceChildren();
  planes.forEach(p=>{
    if(!Number.isFinite(p.lat)||!Number.isFinite(p.lon)||!Number.isFinite(s.radiusKm)||s.radiusKm<=0)return;
    const pos=project(p,s);if(Math.hypot(pos.x-50,pos.y-50)>48)return;
    const b=document.createElement('button');b.type='button';b.className='plane'+(selected&&key(p)===key(selected)?' selected':'');b.style.left=pos.x+'%';b.style.top=pos.y+'%';b.dataset.plane=key(p);b.setAttribute('aria-label','Voir '+(p.callsign||p.hex));b.setAttribute('aria-pressed',String(Boolean(selected&&key(p)===key(selected))));
    const svg=document.createElementNS('http://www.w3.org/2000/svg','svg');svg.setAttribute('viewBox','-15 -15 30 30');svg.setAttribute('aria-hidden','true');svg.style.transform='rotate('+(p.trackAvailable&&Number.isFinite(p.track)?p.track:0)+'deg)';
    const use=document.createElementNS('http://www.w3.org/2000/svg','use');use.setAttribute('href','#planeIcon');svg.append(use);const label=document.createElement('span');label.textContent=p.callsign||p.hex;b.append(svg,label);b.addEventListener('click',()=>selectPlane(p));el('planeLayer').append(b);
  });
  [...planes].sort((a,b)=>a.distanceKm-b.distanceKm).forEach(p=>{
    const row=document.createElement('button');row.type='button';row.className='flightRow'+(selected&&key(p)===key(selected)?' selected':'');row.dataset.plane=key(p);row.setAttribute('aria-pressed',String(Boolean(selected&&key(p)===key(selected))));
    const left=document.createElement('span'),name=document.createElement('strong'),company=document.createElement('small'),right=document.createElement('span'),unit=document.createElement('small');name.textContent=p.callsign||p.hex;company.textContent=p.airline||p.type||'Compagnie inconnue';left.append(name,company);right.className='flightDistance';right.textContent=fmt(p.distanceKm,1);unit.textContent='km';right.append(unit);row.append(left,right);row.setAttribute('aria-label',(p.callsign||p.hex)+', '+fmt(p.distanceKm,1)+' kilomètres');row.addEventListener('click',()=>selectPlane(p));el('flightList').append(row);
  });
  if(!planes.length){const empty=document.createElement('div');empty.className='listEmpty';empty.textContent=good?(activeFilters(s)&&Number(s.totalCount)>0?'Aucun vol ne correspond aux filtres. Modifiez-les pour élargir la sélection.':'Les prochains avions apparaîtront ici.'):notice.detail;el('flightList').append(empty);}
  el('flightList').scrollTop=scroll;
  if(focused){const target=Array.from(el(focusArea).querySelectorAll('button')).find(b=>b.dataset.plane===focused);target?.focus({preventScroll:true});}
  el('count').textContent=good?(activeFilters(s)?planes.length+' / '+(s.totalCount??planes.length):planes.length+' avion'+(planes.length>1?'s':'')):'En attente';
  el('radarTitle').textContent=good?(planes.length?'Dans un rayon de '+s.radiusKm+' km':activeFilters(s)&&Number(s.totalCount)>0?'Aucun vol dans les filtres':'Un ciel tranquille'):notice.title;
  el('radarSub').textContent=good?'Touchez un avion pour voir sa fiche.':notice.detail;
}
function renderDetails(){
  const s=data(),good=usable(s),p=good&&selectedPlane(s),mode=s?.display?.mode||'auto';
  el('details').hidden=!p;el('empty').hidden=Boolean(p);
  if(p){el('callsign').textContent=p.callsign||p.hex;el('airline').textContent=p.airline||'Compagnie inconnue';el('type').textContent=p.type||'Type non renseigné';el('altitude').textContent=p.altitudeAvailable?fmt(p.altitudeFt):'—';el('speed').textContent=p.speedAvailable?fmt(p.speedKmh):'—';el('distance').textContent=fmt(p.distanceKm,1);if(!logoCache.has(p.callsign))fetchLogo(p);}
  else{const notice=radarNotice(s);el('emptyTitle').textContent=!good?notice.title:selectedHex?'Ce vol a quitté le radar.':activeFilters(s)&&Number(s.totalCount)>0?'Aucun vol dans les filtres.':'Le ciel est tranquille.';el('emptyText').textContent=!good?notice.detail:selectedHex?'Choisissez un autre avion dans la liste.':activeFilters(s)&&Number(s.totalCount)>0?'Modifiez les filtres pour élargir votre sélection.':s?.display?.timeSynced?'L’horloge prend le relais en mode Auto.':'L’horloge apparaîtra dès sa synchronisation.';}
  renderAirlineLogo(p);renderRoute(p);
  el('selectedState').textContent=p&&panelApp(s)==='aviation'&&mode!=='clock'&&!s?.display?.sleeping&&s?.display?.activeHex===key(p)?(s?.display?.panelReady?'Sur le panneau':'Sur l’aperçu'):p?'Fiche du vol':'—';
  el('sendBtn').disabled=!p||commandBusy||uploadBusy;
  el('autoBtn').disabled=(!demo&&!serverOnline)||commandBusy||uploadBusy;el('clockBtn').disabled=el('autoBtn').disabled;
  setPressed('autoBtn',mode==='auto');setPressed('clockBtn',mode==='clock');setPressed('sendBtn',Boolean(p&&mode!=='auto'&&mode!=='clock'&&s?.display?.activeHex===key(p)));
  el('modeHint').textContent=demo?'Tester l’affichage dans la démo':panelApp(s)==='aviation'?'Affichage du panneau':'Ces commandes activent Flight Board sur le panneau';
}
function previewPlane(s){if(!usable(s))return null;return aircraft(s).find(p=>key(p)===s?.display?.activeHex)||null;}
function canvasText(g,text,x,y,font,color){return paintPanelText(g,panelTextLayout(text,Number(String(font).match(/(\d+)px/)?.[1])||7,128-x,g.panelFont),x,y,color);}
function panelAircraftType(type){return typeof type==='string'&&type.length>=2&&type.length<=4&&!/[^A-Z0-9]/.test(type)?type:'TYPE --';}
// Compact original bitmap glyphs: one bit is one LED, with integer enlargement.
const PANEL_FONT_5X7={
  A:'0E11111F111111',B:'1E11111E11111E',C:'0F10101010100F',D:'1E11111111111E',E:'1F10101E10101F',F:'1F10101E101010',G:'0F10101711110F',H:'1111111F111111',I:'1F04040404041F',J:'0702020212120C',K:'11121418141211',L:'1010101010101F',M:'111B1515111111',N:'11191915131311',O:'0E11111111110E',P:'1E11111E101010',Q:'0E11111115120D',R:'1E11111E141211',S:'0F10100E01011E',T:'1F040404040404',U:'1111111111110E',V:'11111111110A04',W:'11111115151B11',X:'11110A040A1111',Y:'11110A04040404',Z:'1F01020408101F',
  0:'0E11111111110E',1:'040C040404040E',2:'0E11010204081F',3:'1E01010601011E',4:'02060A121F0202',5:'1F10101E01011E',6:'0E10101E11110E',7:'1F010204080808',8:'0E11110E11110E',9:'0E11110F01010E',
  "!":'01010101010001',"'":'01010000000000',' ':'00000000000000','-':'00000007000000',':':'00010000010000','.':'00000000000001',',':'00000000000201','/':'01010202020404','>':'00040201020400','?':'0E110102040004'
};
const PANEL_FONT_3X5={A:'25755',B:'65656',C:'34443',D:'65556',E:'74747',F:'74744',G:'34553',H:'55755',I:'72227',J:'11152',K:'55655',L:'44447',M:'57755',N:'57555',O:'25552',P:'65644',Q:'25573',R:'65655',S:'34216',T:'72222',U:'55557',V:'55552',W:'55577',X:'55255',Y:'55222',Z:'71247',0:'25552',1:'26227',2:'61247',3:'61216',4:'55711',5:'74616',6:'34652',7:'71222',8:'25252',9:'25316',"!":'11101',"'":'11000',' ':'00000','-':'00700',':':'01010','.':'00001',',':'00011','/':'11244','>':'42124','?':'61020'};
const PANEL_FONT_MODERN={"A":"040A11111F1111","B":"1E11111E11111E","C":"0F10101010100F","D":"1E11111111111E","E":"1F10101E10101F","F":"1F10101E101010","G":"0F10101711110F","H":"1111111F111111","I":"0E04040404040E","J":"0702020212120C","K":"11121418141211","L":"1010101010101F","M":"111B1515111111","N":"11191513111111","O":"0E11111111110E","P":"1E11111E101010","Q":"0E11111115120D","R":"1E11111E141211","S":"0F10100E01011E","T":"1F040404040404","U":"1111111111110E","V":"11111111110A04","W":"11111115151B11","X":"11110A040A1111","Y":"11110A04040404","Z":"1F01020408101F","0":"0E11131519110E","1":"040C040404040E","2":"0E11010204081F","3":"1E01010601011E","4":"02060A121F0202","5":"1F10101E01011E","6":"0E10101E11110E","7":"1F010204080808","8":"0E11110E11110E","9":"0E11110F01010E"};
const PANEL_FONT_DIGITAL={"0":"0E11110011110E","1":"00010100010100","2":"0E01010E10100E","3":"0E01010E01010E","4":"0011110E010100","5":"0E10100E01010E","6":"0E10100E11110E","7":"0E010100010100","8":"0E11110E11110E","9":"0E11110E01010E"};
function panelGlyph(character,small,style="classic"){const glyphs=small?PANEL_FONT_3X5:PANEL_FONT_5X7,c=Object.prototype.hasOwnProperty.call(glyphs,character)?character:'?';let bits=glyphs[c];if(!small){if(style==='digital'&&PANEL_FONT_DIGITAL[c])bits=PANEL_FONT_DIGITAL[c];else if((style==='modern'||style==='digital')&&PANEL_FONT_MODERN[c])bits=PANEL_FONT_MODERN[c];else if(style==='arcade'&&/[A-Z0-9]/.test(c))bits=bits.match(/../g).map(row=>{const n=parseInt(row,16);return ((n|(n>>1))&31).toString(16).padStart(2,'0');}).join('');}return{bits,width:({"!":1,"'":1,' ':small?1:3,':':1,'.':1,',':small?1:2,'/':3,'>':3,'-':3})[c]||(small?3:5)};}
function panelTextWidth(text,small,scale){return Math.max(0,Array.from(text).reduce((width,c)=>width+panelGlyph(c,small).width+1,0)-1)*scale;}
function panelTextLayout(value,size,maxWidth,style="classic"){
  let text=String(value).normalize('NFD').replace(/[\u0300-\u036f]/g,'').toUpperCase().replace(/\s/g,' '),small=size<7||style==='compact',scale=Math.max(1,Math.floor(size/(small?5:7)));
  while(scale>1&&panelTextWidth(text,small,scale)>maxWidth)--scale;
  if(panelTextWidth(text,small,scale)>maxWidth){small=true;scale=1;}
  if(panelTextWidth(text,small,scale)>maxWidth)text='...';
  return{text,small,scale,width:panelTextWidth(text,small,scale),height:(small?5:7)*scale};
}
function paintPanelText(g,layout,x,y,color){
  x=Math.round(x);y=Math.round(y);let cursor=x;g.fillStyle=color;
  for(const c of layout.text){const glyph=panelGlyph(c,layout.small,g.panelFont),digits=layout.small?1:2,rows=layout.small?5:7;
    for(let row=0;row<rows;row++){const bits=parseInt(glyph.bits.slice(row*digits,(row+1)*digits),16);for(let col=0;col<glyph.width;col++)if(bits&(1<<(glyph.width-col-1)))g.fillRect(cursor+col*layout.scale,y+row*layout.scale,layout.scale,layout.scale);}
    cursor+=(glyph.width+1)*layout.scale;
  }
  return{...layout,x,y,color};
}
function canvasCenteredText(g,text,center,y,width,size,color){const layout=panelTextLayout(text,size,width,g.panelFont);return paintPanelText(g,layout,Math.round(center-layout.width/2),y,color);}
function previewMetric(g,label,value,unit,center,color){
  canvasCenteredText(g,label,center,37,38,6,'#a2b0b0');
  canvasCenteredText(g,value,center,44,38,10,color,8);
  canvasCenteredText(g,unit,center,57,38,6,'#a2b0b0');
}
const idleMotion=window.matchMedia('(prefers-reduced-motion: reduce)');
let previewAnimationTimer=null,idleFrameImage=null;
function previewIsDraft(){const s=data(),visual=s?.visual||s?.display?.visual||'lava';if(widgetView())return panelApp(s)!==currentView||(s?.contentId||'')!==currentContentId||widgetDirty||Boolean(storeMode&&currentContentId);return ['clock','animation','weather'].includes(currentView)&&(panelApp(s)!==currentView||Boolean(draftVisual&&draftVisual!==visual));}
function previewData(){const s=data();if(!s)return s;const draft=previewIsDraft(),app=draft?currentView:panelApp(s),id=draft?currentContentId:s.contentId,visual=draftVisual||s.visual||s.display?.visual||'lava';let widget=s.widget;if(WIDGET_APPS.includes(app)){const pack=id?(draft?selectedContentPack():contentCache.get(id)):null;widget=pack?{...(!draft?s.widget:{}),...pack,remainingSeconds:pack.durationSeconds,running:draft?false:Boolean(s.widget?.running)}:draft&&widgetDraft?widgetDraft:{...s.widget};if(app==='timer'&&!draft)widget={...widget,remainingSeconds:widgetRemaining(s.widget)};}if(!draft)return WIDGET_APPS.includes(app)?{...s,widget}:s;return{...s,app,visual,widget,contentId:currentContentId,display:{...s.display,app,visual,sleeping:false,effectiveBrightness:s.display?.sleeping?s.display?.brightness??60:s.display?.effectiveBrightness}};}

function previewVisible(){return !document.hidden&&el('previewToggle').open;}
function idlePreview(s){const d=s?.display||{},mode=d.mode||'auto',app=s?.app||d.app||'aviation';return Boolean(s&&(demo||serverOnline)&&!d.sleeping&&((app==='clock'||app==='animation'||app==='message'||app==='pixel')||(app==='weather'&&panelWeatherAnimated(s.weather))||(app==='aviation'&&(mode==='clock'||(mode==='auto'&&!previewPlane(s))))));}
function syncPreviewAnimation(){
  const s=previewData(),animate=previewVisible()&&!idleMotion.matches&&(['weather','message','pixel'].includes(s?.app||s?.display?.app)||(s?.visual||s?.display?.visual||'lava')!=='plain')&&idlePreview(s);
  if(!animate){if(previewAnimationTimer!==null)clearTimeout(previewAnimationTimer);previewAnimationTimer=null;return;}
  if(previewAnimationTimer===null)previewAnimationTimer=setTimeout(()=>{previewAnimationTimer=null;drawPreview();},80);
}
function panelRgb565(value){return 'rgb('+[((value>>11)&31)*255/31,((value>>5)&63)*255/63,(value&31)*255/31].map(Math.round).join(',')+')';}
function panelRect(g,x,y,w,h,color){const right=Math.min(128,x+w),bottom=Math.min(64,y+h);x=Math.max(0,x);y=Math.max(0,y);if(right>x&&bottom>y){g.fillStyle=color;g.fillRect(x,y,right-x,bottom-y);}}
const PANEL_WAVE=[0,3,6,9,12,15,17,20,22,24,26,27,29,30,30,31,31,31,30,30,29,27,26,24,22,20,17,15,12,9,6,3,0,-3,-6,-9,-12,-15,-17,-20,-22,-24,-26,-27,-29,-30,-30,-31,-31,-31,-30,-30,-29,-27,-26,-24,-22,-20,-17,-15,-12,-9,-6,-3];
function panelWave(phase){return PANEL_WAVE[(phase&255)>>2];}
function drawAtmosphere(g,visual,tick,clock){
  if(!idleFrameImage)idleFrameImage=g.createImageData(128,64);
  const pixels=idleFrameImage.data,t=tick%65536;
  for(let x=0;x<128;x++){
    const ridge=20+Math.trunc(panelWave(x*3+t)/3)+Math.trunc(panelWave(x*7-t)/8),ribbon=34+Math.trunc(panelWave(x*2-t)/3),swell=22+Math.trunc(panelWave(x*2+t)/7),middle=35+Math.trunc(panelWave(x*3-t*2)/5),shore=51+Math.trunc(panelWave(x*2+t*3)/5);
    for(let y=0;y<64;y++){
      let r=0,green=2,b=9;
      if(visual==='aurora'){let a=27-Math.abs(y-ridge)*2,c=20-Math.abs(y-ribbon)*2;a=a>0?Math.trunc(a*(48+panelWave(x*11+t))/64):0;c=Math.max(0,c);r=c*2;green=a*5+c;b+=a*3+c*5;}
      else if(y>=swell){green=18;b=38;if(y>=middle){green=31;b=64;}if(y>=shore){green=45;b=73;}if(y===swell||y===middle||y===shore){green+=35;b+=40;}if(Math.abs(x-96)<Math.trunc((y-16)/2)+2&&(y+Math.trunc(t/3))%9<2){r=25;green+=35;b+=45;}}
      else if((x-96)*(x-96)+(y-11)*(y-11)<30){r=98;green=140;b=162;}
      if(y<17&&(x*17+y*43)%311===0){r+=25;green+=30;b+=35;}
      if(clock&&y>=14&&y<50){const shade=y>=18&&y<46?1:2;r=Math.trunc(r*shade/4);green=Math.trunc(green*shade/4);b=Math.trunc(b*shade/4);}
      const i=(y*128+x)*4;pixels[i]=r;pixels[i+1]=green;pixels[i+2]=b;pixels[i+3]=255;
    }
  }g.putImageData(idleFrameImage,0,0);
}
function drawAmbient(g,visual,time,clock=false){
  const tick=Math.floor(time*1000/80);
  if(visual==='aurora'||visual==='ocean'){drawAtmosphere(g,visual,tick,clock);return;}
  if(visual!=='lava'){
    g.fillStyle='#000';g.fillRect(0,0,128,64);
    if(visual==='stars')for(let i=0;i<54;i++){const x=(i*37+11+Math.floor(tick/(2+i%5))%128)%128,y=(i*i*13+7)%64,color=(Math.floor(tick/8)+i)%5===0?0xffff:i%3===0?0x8ddf:0x426b;panelRect(g,x,y,1,1,panelRgb565(color));if(i%13===0)panelRect(g,x,y+1,1,1,panelRgb565(0x426b));}
    else if(visual==='rain')for(let i=0;i<19;i++){const x=i*7,head=(tick*(1+i%3)+i*11)%78-7;for(let j=0;j<8;j++)panelRect(g,x,head-j*2,2,1,panelRgb565(j===0?0xb7f7:(56-j*6)<<5));}
    else if(visual==='warp')for(let i=0;i<68;i++){const dx=(i*47+9)%129-64,dy=(i*29+3)%65-32,z=64-(tick*(1+i%2)+i*19)%64,x=64+Math.trunc(dx*16/z),y=32+Math.trunc(dy*16/z),tailX=64+Math.trunc(dx*16/(z+5)),tailY=32+Math.trunc(dy*16/(z+5)),color=panelRgb565(z<12?0xffff:i%4===0?0x999f:0x6cba);for(let j=0;j<=5;j++)panelRect(g,x+Math.trunc((tailX-x)*j/5),y+Math.trunc((tailY-y)*j/5),1,1,color);}
    return;
  }
  // Shared fixed-point fluid contours: no texture, floating point or network.
  if(!idleFrameImage)idleFrameImage=g.createImageData(128,64);
  const pixels=idleFrameImage.data,t=tick%65536,flow=phase=>{const f=phase&3;return panelWave(phase)*(4-f)+panelWave(phase+4)*f;};
  const centers=[[80+flow(t),128+flow(Math.trunc(t/2)+71)],[432+flow(Math.trunc(t/2)+34),384+flow(t+170)],[712+flow(t+128),96+flow(Math.trunc(t/2)+210)],[1024+flow(Math.trunc(t/2)+83),432+flow(t+29)]],colors=[[0,0,0],[8,24,160],[0,52,234],[0,108,255],[0,172,250],[0,225,222],[32,250,198],[0,0,0]];
  for(let y=0;y<64;y++)for(let x=0;x<128;x++){
    const px=x*8+Math.trunc(flow(y*3+t)/6),py=y*8+Math.trunc(flow(x*2-t)/6);
    let field=0;for(const [cx,cy] of centers){const dx=px-cx,dy=py-cy;field+=Math.trunc(691200/(dx*dx+dy*dy+4608));}
    const band=field<23?0:field<30?1:field<39?2:field<51?3:field<67?4:field<87?5:field<117?6:7,color=colors[band],i=(y*128+x)*4;
    pixels[i]=color[0];pixels[i+1]=color[1];pixels[i+2]=color[2];pixels[i+3]=255;
  }
  g.putImageData(idleFrameImage,0,0);
}
function drawIdleClock(g,d,time,visual='lava'){
  drawAmbient(g,visual,time,true);
  const synced=Boolean(d.timeSynced&&/^(?:[01]\d|2[0-3]):[0-5]\d$/.test(String(d.clock||'')));
  if(visual==='stars'||visual==='rain'||visual==='warp')panelRect(g,5,synced?16:12,118,32,'#000');
  if(visual==='lava')for(const dy of[-1,1])for(const dx of[-1,1])canvasCenteredText(g,synced?d.clock:'--:--',64+dx,(synced?18:14)+dy,116,30,'#000');
  canvasCenteredText(g,synced?d.clock:'--:--',64,synced?18:14,116,30,'#fff',30);
  if(!synced)canvasCenteredText(g,'ATTENTE HEURE',64,51,116,7,'#c3d9e6');
}

function panelWeatherLabel(code,day){return code===0?day?'CIEL DEGAGE':'NUIT CLAIRE':code===1||code===2?'ECLAIRCIES':code===3?'NUAGEUX':code===45||code===48?'BROUILLARD':code>=51&&code<=57?'BRUINE':(code>=71&&code<=77)||code===85||code===86?'NEIGE':[95,96,99].includes(code)?'ORAGE':(code>=61&&code<=67)||(code>=80&&code<=82)?'PLUIE':'METEO';}
function panelDisc(g,cx,cy,r,color){for(let y=-r;y<=r;y++)for(let x=-r;x<=r;x++)if(x*x+y*y<=r*r)panelRect(g,cx+x,cy+y,1,1,color);}
function panelWeatherAnimated(w){return Boolean(w?.available&&Number.isFinite(w.temperatureC)&&Number.isFinite(w.windKmh)&&[0,1,2,3,45,48,51,53,55,56,57,61,63,65,66,67,71,73,75,77,80,81,82,85,86,95,96,99].includes(w.code));}
function drawWeatherIcon(g,w,time){
  const tick=panelWeatherAnimated(w)?Math.floor(time*1000/80):0,t=tick%65536,code=w.code,drift=Math.trunc(panelWave(t*2)/15),blue=panelRgb565(0x8ddf),amber='#e6bf81',muted='#a2b0b0';
  if(code<3){panelDisc(g,23,27,8,w.isDay?amber:blue);if(!w.isDay){panelDisc(g,27,24,7,'#000');for(let i=0;i<5;i++){const glow=75+panelWave(t*3+i*47)*2;panelRect(g,7+(i*13)%33,13+(i*7)%26,1,1,'rgb('+glow+','+glow+','+(glow+35)+')');}}else{panelDisc(g,22,26,5,'#ffd060');for(let i=0;i<8;i++){const dx=panelWave(i*32+t*2),dy=panelWave(i*32+t*2+64);for(let r=11;r<=13;r++)panelRect(g,23+Math.trunc(dx*r/31),27+Math.trunc(dy*r/31),1,1,amber);}}}
  if(code===45||code===48){for(let i=0;i<5;i++)panelRect(g,10+Math.trunc(panelWave(t*2+i*53)/10),19+i*4,26,2,i%2?panelRgb565(0x426b):muted);return;}
  if(code>0){panelDisc(g,15+drift,28,5,muted);panelDisc(g,23+drift,24,7,muted);panelDisc(g,31+drift,29,6,muted);panelRect(g,12+drift,28,22,7,muted);panelRect(g,14+drift,33,19,2,panelRgb565(0x738e));
    if(code>=51)for(let i=0;i<4;i++){const snow=(code>=71&&code<=77)||code===85||code===86,x=12+i*7+(snow?Math.trunc(panelWave(t*3+i*63)/20):0),y=36+(Math.floor(tick/(snow?3:1))+i*3)%6;panelRect(g,x,y,1,1,blue);panelRect(g,x-1,y+1,1,1,blue);if(snow){panelRect(g,x+1,y+1,1,1,'#fff');panelRect(g,x,y+2,1,1,'#fff');}}
    if([95,96,99].includes(code)){const glow=120+panelWave(t*2)*2,bolt='rgb('+glow+','+Math.trunc(glow*3/4)+','+Math.trunc(glow/4)+')';panelRect(g,22,31,3,5,bolt);panelRect(g,20,35,4,3,bolt);panelRect(g,21,38,1,1,bolt);}
  }
}
function drawWidgetPreview(g,app,w,time){
  if(!['message','timer','score','pixel'].includes(app))return false;const muted='#a2b0b0',white='#edf0e9',mint='#a9d8c6',amber='#e6bf81';if(!w){canvasCenteredText(g,'CONTENU INDISPONIBLE',64,25,122,7,muted);return true;}
  if(app==='message'){
    canvasCenteredText(g,'MESSAGE',64,3,122,6,muted);
    if(typeof w.text!=='string'||!w.text.length||w.text.length>120||/[^\x20-\x7e]/.test(w.text)){canvasCenteredText(g,'AUCUN MESSAGE',64,25,122,7,muted);return true;}const text=w.text.toUpperCase(),small=g.panelFont==='compact',width=panelTextWidth(text,small,2),phase=Math.floor(time*1000/50);let cursor=width<=120?Math.floor((128-width)/2):128-phase%(width+128);const color=panelRgb565(Number(w.color??0xafd8));
    for(const c of text.slice(0,120)){const glyph=panelGlyph(c,small,g.panelFont),digits=small?1:2;for(let row=0;row<(small?5:7);row++){const bits=parseInt(glyph.bits.slice(row*digits,row*digits+digits),16);for(let col=0;col<glyph.width;col++)if(bits&(1<<(glyph.width-col-1)))panelRect(g,cursor+col*2,25+row*2,2,2,color);}cursor+=(glyph.width+1)*2;}return true;
  }
  if(app==='timer'){const n=Math.max(0,Number(w.remainingSeconds??w.durationSeconds)||0);canvasCenteredText(g,'MINUTEUR',64,3,122,6,muted);canvasCenteredText(g,n<=5999?timeLabel(n):'--:--',64,22,122,28,white);canvasCenteredText(g,n===0?'TERMINE':w.running?'EN COURS':'PAUSE',64,55,122,6,w.running&&n?mint:amber);return true;}
  if(app==='score'){const ascii=(s,n,empty=false)=>typeof s==='string'&&(empty||s.length>0)&&s.length<=n&&!/[^\x20-\x7e]/.test(s);if(!w.available||!ascii(w.home,12)||!ascii(w.away,12)||!ascii(w.status??'',24,true)||![w.homeScore,w.awayScore].every(n=>Number.isInteger(n)&&n>=0&&n<=999)){canvasCenteredText(g,'SCORE INDISPONIBLE',64,25,122,7,muted);return true;}canvasCenteredText(g,w.home||'',31,5,58,7,mint);canvasCenteredText(g,w.away||'',97,5,58,7,mint);canvasCenteredText(g,String(w.homeScore)+'-'+String(w.awayScore),64,22,122,21,white);canvasCenteredText(g,w.status||'',64,49,122,6,muted);if(w.stale)canvasCenteredText(g,'ANCIEN',64,58,122,6,amber);return true;}
  if(app==='pixel'){if(!Number.isInteger(w.width)||w.width<1||w.width>64||!Number.isInteger(w.height)||w.height<1||w.height>64||!Number.isInteger(w.frameCount)||w.frameCount<1||w.frameCount>32||!Number.isInteger(w.frameMs)||w.frameMs<80||w.frameMs>2000||!Array.isArray(w.palette)||w.palette.length<1||w.palette.length>16||typeof w.pixels!=='string'||w.pixels.length!==w.width*w.height*w.frameCount||w.pixels.length>65536||!/^[0-9a-f]+$/i.test(w.pixels)){canvasCenteredText(g,'PIXEL INDISPONIBLE',64,25,122,7,muted);return true;}const scale=Math.min(Math.floor(128/w.width),Math.floor(64/w.height)),ox=Math.floor((128-w.width*scale)/2),oy=Math.floor((64-w.height*scale)/2),frame=Math.floor(time*1000/w.frameMs)%w.frameCount,offset=frame*w.width*w.height;for(let y=0;y<w.height;y++)for(let x=0;x<w.width;x++){const color=w.palette[parseInt(w.pixels[offset+y*w.width+x],16)]??0;panelRect(g,ox+x*scale,oy+y*scale,scale,scale,panelRgb565(color));}return true;}
  return false;
}
function drawZcsPreview(g,s,d,time){
  const app=s?.app||d.app||'aviation',visual=s?.visual||d.visual||'lava';
  if(drawWidgetPreview(g,app,s?.widget,time))return true;
  if(app==='clock'){drawIdleClock(g,d,time,visual);return true;}
  if(app==='animation'){drawAmbient(g,visual,time);return true;}
  if(app!=='weather')return false;
  const w=s.weather;canvasText(g,w?.stale?'ANCIEN':'METEO',4,3,'6px monospace','#a2b0b0');
  if(d.timeSynced&&/^(?:[01]\d|2[0-3]):[0-5]\d$/.test(String(d.clock||'')))canvasText(g,d.clock,103,3,'6px monospace','#a2b0b0');
  if(!w||!w.available||typeof w.temperatureC!=='number'||!Number.isFinite(w.temperatureC)||typeof w.windKmh!=='number'||!Number.isFinite(w.windKmh)){canvasCenteredText(g,'METEO INDISPONIBLE',64,26,120,7,'#edf0e9');canvasCenteredText(g,'EN ATTENTE DU RELEVE',64,47,120,6,'#a2b0b0');return true;}
  const code=w.code;drawWeatherIcon(g,w,time);
  canvasCenteredText(g,Math.floor(w.temperatureC+.5)+' C',85,18,78,21,'#edf0e9');panelRect(g,3,44,122,1,'#20312c');canvasCenteredText(g,panelWeatherLabel(code,w.isDay),64,48,122,6,'#a9d8c6');canvasCenteredText(g,'VENT '+Math.floor(w.windKmh+.5)+' KM/H',64,57,122,6,'#a2b0b0');return true;
}

function drawPreview(){
  syncPreviewAnimation();
  const c=el('ledPreview'),s=previewData(),d=s?.display||{},mode=d.mode||'auto',app=s?.app||d.app||'aviation';
  const bright=Number(d.effectiveBrightness??d.brightness??60);c.style.filter='brightness('+Math.max(0,Math.min(1,bright/100))+')';
  el('displayMode').textContent=(app==='aviation'?'Flight Board · '+(MODE_NAMES[mode]||'Épinglé'):APP_NAMES[app]||'ZCS CONTROL')+' · 128 × 64';el('previewStatus').textContent=d.sleeping?'En veille':d.nightActive?'Luminosité de nuit · '+bright+' %':'Aperçu · '+bright+' %';el('previewHelp').textContent=d.sleeping?'Reprise automatique à la fin de la nuit.':d.panelReady===true?'Panneau : '+d.panelBrightness+' % · Modifiable dans les réglages.':d.panelReady===false?'Le pilote du panneau n’a pas pu démarrer.':'Aperçu du panneau 128 × 64.';
  if(previewIsDraft()){el('previewStatus').textContent='Aperçu de votre sélection';el('previewHelp').textContent=storeMode&&currentContentId?'Ajoutez ce contenu à votre bibliothèque pour l’afficher.':'Appuyez sur « Afficher sur le panneau » pour l’activer.';}
  if(!previewVisible())return;
  const g=c.getContext('2d');const fontApp=app==='clock'||(app==='aviation'&&(mode==='clock'||(mode==='auto'&&!previewPlane(s))))?'clock':app;g.panelFont=s?.fonts?.[fontApp]||'classic';g.fillStyle='#000';g.fillRect(0,0,128,64);g.textBaseline='top';
  if(d.sleeping)return;
  if(!s||(!demo&&!serverOnline)){canvasText(g,'ZCS CONTROL',20,9,'bold 11px monospace','#a9d8c6');canvasText(g,'CONNEXION',31,31,'10px monospace','#edf0e9');canvasText(g,'EN ATTENTE',31,46,'10px monospace','#a2b0b0');return;}
  if(drawZcsPreview(g,s,d,idleMotion.matches?0:performance.now()/1000))return;
  const p=previewPlane(s);
  if(mode==='clock'||(mode==='auto'&&!p)){
    drawIdleClock(g,d,idleMotion.matches?0:performance.now()/1000,s?.visual||d.visual||'lava');
    return;
  }
  if(!p){canvasText(g,'FLIGHT BOARD',23,10,'bold 10px monospace','#a9d8c6');canvasText(g,'VOL INDISPONIBLE',17,32,'9px monospace','#edf0e9');canvasText(g,'CHOISIR UN AVION',20,49,'8px monospace','#a2b0b0');return;}
  canvasText(g,(p.callsign||'INCONNU').slice(0,12),31,2,'bold 10px monospace','#a9d8c6');
  canvasText(g,panelAircraftType(p.type),31,11,'7px monospace','#a2b0b0');
  const logo=logoCache.get(p.callsign);if(logo){const scratch=document.createElement('canvas');scratch.width=24;scratch.height=16;paintLogo(scratch,logo);g.drawImage(scratch,3,2);}else fetchLogo(p);
  g.fillStyle='#20312c';g.fillRect(3,19,122,1);g.fillRect(3,35,122,1);
  const route=routeFor(p);
  if(route.status==='known'&&route.origin&&route.destination)canvasCenteredText(g,route.origin.code+' > '+route.destination.code,64,21,122,14,'#edf0e9',12);
  else canvasCenteredText(g,route.status==='loading'||route.status==='not_requested'?'RECHERCHE DU TRAJET':route.status==='error'?'TRAJET INDISPONIBLE':route.status==='unconfirmed'?'TRAJET NON CONFIRME':'TRAJET NON RENSEIGNE',64,24,122,8,'#a2b0b0');
  previewMetric(g,'DIST',Number.isFinite(p.distanceKm)?fmt(p.distanceKm,1):'--','KM',21,'#a9d8c6');
  previewMetric(g,'VIT',p.speedAvailable&&Number.isFinite(p.speedKmh)?String(Math.round(p.speedKmh)):'--','KM/H',64,'#edf0e9');
  previewMetric(g,'ALT',p.altitudeAvailable&&Number.isFinite(p.altitudeFt)?String(Math.round(p.altitudeFt)):'--','FT',107,'#e6bf81');
}
function renderConnection(){const s=data(),aviation=currentView==='aviation';el('connection').textContent=demo?'Démonstration':!serverOnline?'Panneau hors ligne':s?.wifiConnected===false?'Wi-Fi à reconnecter':aviation?radarNotice(s).label:'Panneau connecté';el('dot').style.background=demo?'var(--amber)':!serverOnline?'var(--error)':!aviation||usable(s)&&s?.radar?.status!=='paused'?'var(--accent)':'var(--amber)';}
function render(){
  const s=data();renderModules();renderConnection();
  renderBrightnessStatus();
  if(s){el('placeBadge').textContent=s.place||'Flight Board';el('placeMeta').textContent='Rayon de '+s.radiusKm+' km · '+(demo?'Avions fictifs':serverOnline?'Suivi automatique du ciel':'Dernier lieu connu');}
  const interval=Number(s?.radar?.intervalSeconds);el('radarCadence').hidden=demo||!Number.isFinite(interval)||interval<=0;el('radarCadence').textContent='Actualisation : '+Math.ceil(interval)+' s';
  el('updated').textContent=demo?'Données de démonstration':usable(s)?'Actualisé il y a '+age(s)+' s':'En attente des données';
  el('favoriteSelect').disabled=demo||favoriteBusy||!serverOnline||uploadBusy;el('locationBtn').disabled=demo||!serverOnline||uploadBusy;
  renderRadar();renderDetails();drawPreview();syncRoutes();
}
async function refresh(){if(polling||uploadBusy||wifiIsOpen())return;polling=true;try{latest=await requestJson('/api/state');lastStateAt=Date.now();serverOnline=true;ensureActiveContent();}catch(error){serverOnline=false;}finally{polling=false;render();}}
function setDemo(on){++routeEpoch;routeFocus='';demo=on;selectedHex='';setPressed('demoBtn',demo);el('demoBtn').setAttribute('aria-label',demo?'Quitter la démonstration':'Activer la démonstration');el('demoBtn').title=demo?'Quitter la démonstration':'Activer la démonstration';el('demoBadge').hidden=!demo;el('demoNotice').classList.toggle('visible',demo);notify(demo?'Démonstration locale · Aucun réglage du Flight Board n’est modifié.':'Mode direct · Données ADSB.lol.');render();if(!demo)refresh();}
async function sendSelected(){const s=data(),p=usable(s)&&selectedPlane(s);if(!p||commandBusy||uploadBusy)return;if(demo){DEMO.app='aviation';DEMO.display.mode='manual';DEMO.display.activeHex=key(p);notify(p.callsign+' épinglé dans la démo.');render();return;}commandBusy=true;renderDetails();try{await post('/api/display/select',{hex:key(p)});notify(p.callsign+(s?.display?.panelReady?' épinglé sur le panneau.':' épinglé sur l’aperçu.'));await refresh();}catch(error){notify(error.message);}finally{commandBusy=false;renderDetails();}}
async function setMode(mode){if(commandBusy||uploadBusy||(!demo&&!serverOnline))return;if(demo){DEMO.app='aviation';DEMO.display.mode=mode;syncDemoDisplay();notify('Mode '+MODE_NAMES[mode]+' dans la démo.');render();return;}commandBusy=true;renderDetails();try{await post('/api/display/mode',{mode});notify(mode==='auto'?'Mode Auto · Le vol le plus proche s’affiche, puis l’horloge prend le relais.':'Mode Horloge activé.');await refresh();}catch(error){notify(error.message);}finally{commandBusy=false;renderDetails();}}
function cityMatching(lat,lon){return Object.keys(CITIES).find(k=>Math.abs(CITIES[k].lat-Number(lat))<.0002&&Math.abs(CITIES[k].lon-Number(lon))<.0002)||'custom';}
function selectCity(){const k=el('cityInput').value,city=CITIES[k];el('latInput').readOnly=Boolean(city);el('lonInput').readOnly=Boolean(city);if(city){el('placeInput').value=city.name;el('latInput').value=city.lat;el('lonInput').value=city.lon;}}
function minuteString(value){value=Number(value)||0;return String(Math.floor(value/60)).padStart(2,'0')+':'+String(value%60).padStart(2,'0');}
function minutes(value){const parts=value.split(':').map(Number);return parts[0]*60+parts[1];}
function nightControls(){const on=el('nightEnabled').checked;['nightStart','nightEnd','nightAction'].forEach(id=>el(id).disabled=!on);renderBrightnessStatus();}
const BRIGHTNESS_FIELDS={brightness:['brightnessInput','brightnessValue','brightnessFeedback'],nightBrightness:['nightBrightness','nightBrightnessValue','nightBrightnessFeedback']};
let brightnessNext=null,brightnessWork=null,brightnessSending=null,brightnessErrors={};
function renderBrightnessStatus(s=latest?.display){
  const blocked=demo||uploadBusy||settingsBusy||!serverOnline;
  for(const [key,[input,,feedback]] of Object.entries(BRIGHTNESS_FIELDS)){
    el(input).disabled=blocked||(key==='nightBrightness'&&(!el('nightEnabled').checked||el('nightAction').value==='sleep'));
    if(brightnessNext?.[key]!==undefined||brightnessSending?.[key]!==undefined){el(feedback).textContent='Application en direct…';continue;}
    if(brightnessErrors[key]){el(feedback).textContent=brightnessErrors[key];continue;}
    if(s&&s[key]===Number(el(input).value))el(feedback).textContent=s.brightnessSaveError?'Valeur reçue, mais mémorisation impossible. Nouvelle tentative automatique.':s.brightnessPending?'Valeur reçue · mémorisation…':'Mémorisée automatiquement.';
  }
}
function queueBrightness(key='brightness'){
  if(demo||uploadBusy||settingsBusy||!serverOnline)return;
  if(!BRIGHTNESS_FIELDS[key]||(key==='nightBrightness'&&(!el('nightEnabled').checked||el('nightAction').value==='sleep')))return;
  const [input,value]=BRIGHTNESS_FIELDS[key];
  brightnessNext={...brightnessNext,[key]:Number(el(input).value)};
  el(value).textContent=brightnessNext[key]+' %';delete brightnessErrors[key];
  flushBrightness();
  renderBrightnessStatus();
}
function flushBrightness(){
  if(brightnessWork)return brightnessWork;
  brightnessWork=(async()=>{
    let result=null;
    try{
      while(brightnessNext!==null){
        const values=brightnessNext;brightnessNext=null;brightnessSending=values;
        renderBrightnessStatus();
        try{
          result=await post('/api/display/brightness',values);
          for(const key of Object.keys(values)){
            if(brightnessNext?.[key]===undefined&&currentSettings)currentSettings[key]=result[key];
            delete brightnessErrors[key];
          }
        }catch(error){for(const key of Object.keys(values))brightnessErrors[key]='Modification non confirmée. Bougez le curseur pour réessayer. '+error.message;}
        brightnessSending=null;renderBrightnessStatus(result);
        // One request at a time; intermediate positions are replaced by the latest.
        await new Promise(resolve=>setTimeout(resolve,150));
      }
    }finally{
      brightnessWork=null;renderBrightnessStatus(result);
      if(!el('settingsDialog').open&&Object.keys(brightnessErrors).length)notify(Object.values(brightnessErrors)[0]);
    }
  })();
  return brightnessWork;
}
function renderFontSamples(){document.querySelectorAll('[data-font-preview]').forEach(select=>{const canvas=el(select.dataset.fontPreview),g=canvas.getContext('2d');g.panelFont=select.value;g.fillStyle='#000';g.fillRect(0,0,128,22);canvasCenteredText(g,'ABC 12:34',64,4,124,14,'#fff');});}
function fillSettings(s){
  el('fontAviationInput').value=s.fontAviation||'classic';
  el('fontClockInput').value=s.fontClock||'classic';
  el('fontWeatherInput').value=s.fontWeather||'classic';
  el('fontMessageInput').value=s.fontMessage||'classic';
  el('fontTimerInput').value=s.fontTimer||'classic';
  el('fontScoreInput').value=s.fontScore||'classic';
  renderFontSamples();
  brightnessErrors={};el('brightnessFeedback').textContent='';el('nightBrightnessFeedback').textContent='';
  el('placeInput').value=s.place;el('latInput').value=s.lat;el('lonInput').value=s.lon;el('cityInput').value=cityMatching(s.lat,s.lon);el('latInput').readOnly=el('cityInput').value!=='custom';el('lonInput').readOnly=el('cityInput').value!=='custom';el('radiusInput').value=String(s.radiusKm);el('brightnessInput').value=s.brightness;el('brightnessValue').textContent=s.brightness+' %';el('clockVisualInput').value=s.visual||'lava';el('timezoneInput').value=s.timezone||'paris';el('nightEnabled').checked=Boolean(s.nightEnabled);el('nightStart').value=minuteString(s.nightStart??1320);el('nightEnd').value=minuteString(s.nightEnd??420);el('nightBrightness').value=s.nightBrightness??15;el('nightBrightnessValue').textContent=(s.nightBrightness??15)+' %';el('nightAction').value=s.nightAction||'dim';el('favoriteName').value=s.place;nightControls();
}
async function openSettings(){if(demo){notify('Revenez au direct pour modifier les réglages du Flight Board.');return;}if(uploadBusy||el('settingsDialog').open)return;const epoch=++settingsEpoch;try{await brightnessWork;const s=await requestJson('/api/settings');if(epoch!==settingsEpoch||demo)return;currentSettings=s;fillSettings(s);renderBrightnessStatus(s);el('settingsFeedback').textContent='';el('favoritesFeedback').textContent='';el('settingsDialog').showModal();loadFavorites();}catch(error){notify(error.message);}}
async function saveSettings(event){
  event.preventDefault();if(demo||settingsBusy||uploadBusy)return;const payload={fontAviation:el('fontAviationInput').value,fontClock:el('fontClockInput').value,fontWeather:el('fontWeatherInput').value,fontMessage:el('fontMessageInput').value,fontTimer:el('fontTimerInput').value,fontScore:el('fontScoreInput').value,visual:el('clockVisualInput').value,place:el('placeInput').value.trim(),lat:Number(el('latInput').value),lon:Number(el('lonInput').value),radiusKm:Number(el('radiusInput').value),brightness:Number(el('brightnessInput').value),timezone:el('timezoneInput').value,nightEnabled:el('nightEnabled').checked,nightStart:minutes(el('nightStart').value),nightEnd:minutes(el('nightEnd').value),nightBrightness:Number(el('nightBrightness').value),nightAction:el('nightAction').value};
  if(!payload.place||new TextEncoder().encode(payload.place).length>32){el('settingsFeedback').textContent='Le nom du lieu doit tenir sur 32 octets.';return;}
  if(!el('latInput').value||!el('lonInput').value||!Number.isFinite(payload.lat)||!Number.isFinite(payload.lon)||Math.abs(payload.lat)>90||Math.abs(payload.lon)>180){el('settingsFeedback').textContent='Vérifiez les coordonnées du lieu.';return;}
  if(payload.nightEnabled&&(!Number.isInteger(payload.nightStart)||!Number.isInteger(payload.nightEnd)||payload.nightStart===payload.nightEnd)){el('nightSection').open=true;el('settingsFeedback').textContent='Choisissez deux horaires différents pour le début et la fin de la nuit.';return;}
  settingsBusy=true;el('saveSettingsBtn').disabled=true;renderBrightnessStatus();el('settingsFeedback').textContent='Enregistrement…';
  try{await brightnessWork;await post('/api/settings',payload);brightnessErrors={};currentSettings=payload;selectedHex='';el('settingsDialog').close();notify('Réglages enregistrés. Le ciel se met à jour.');await refresh();renderFavorites();}catch(error){el('settingsFeedback').textContent=error.message;}finally{settingsBusy=false;el('saveSettingsBtn').disabled=false;renderBrightnessStatus();}
}
function renderFavorites(){
  const select=el('favoriteSelect'),s=latest;select.replaceChildren(new Option('Lieux favoris',''));
  favorites.forEach(f=>select.add(new Option(f.name+' · '+f.radiusKm+' km',String(f.id))));
  const matching=favorites.find(f=>s&&Math.abs(f.lat-s.homeLat)<.00001&&Math.abs(f.lon-s.homeLon)<.00001&&f.radiusKm===s.radiusKm);if(matching)select.value=String(matching.id);
  el('favoriteList').replaceChildren();favorites.forEach(f=>{const row=document.createElement('div');row.className='favoriteItem';const name=document.createElement('span');name.textContent=f.name;const meta=document.createElement('small');meta.textContent=f.radiusKm+' km';name.append(meta);const remove=document.createElement('button');remove.type='button';remove.className='btn quiet';remove.textContent='Retirer';remove.setAttribute('aria-label','Retirer '+f.name+' des favoris');remove.disabled=favoriteBusy||demo;remove.addEventListener('click',()=>removeFavorite(f.id));row.append(name,remove);el('favoriteList').append(row);});el('favoriteSaveBtn').disabled=favoriteBusy||demo;select.disabled=favoriteBusy||demo||!serverOnline||uploadBusy;
}
async function loadFavorites(){try{const result=await requestJson('/api/favorites');favorites=Array.isArray(result.favorites)?result.favorites:[];renderFavorites();}catch(error){el('favoritesFeedback').textContent='Favoris indisponibles. '+error.message;}}
async function addFavorite(){if(demo||favoriteBusy||!currentSettings)return;const name=el('favoriteName').value.trim();if(!name||new TextEncoder().encode(name).length>32){el('favoritesFeedback').textContent='Saisissez un nom de 1 à 32 octets.';return;}favoriteBusy=true;renderFavorites();try{await post('/api/favorites',{name,lat:currentSettings.lat,lon:currentSettings.lon,radiusKm:currentSettings.radiusKm});el('favoritesFeedback').textContent='Lieu actuel enregistré sous « '+name+' ».';await loadFavorites();}catch(error){el('favoritesFeedback').textContent=error.message;}finally{favoriteBusy=false;renderFavorites();}}
async function removeFavorite(id){if(demo||favoriteBusy)return;favoriteBusy=true;renderFavorites();try{await requestJson('/api/favorites?id='+encodeURIComponent(id),{method:'DELETE'});el('favoritesFeedback').textContent='Favori retiré.';await loadFavorites();}catch(error){el('favoritesFeedback').textContent=error.message;}finally{favoriteBusy=false;renderFavorites();}}
async function useFavorite(){const id=el('favoriteSelect').value;if(!id||demo||favoriteBusy)return;favoriteBusy=true;renderFavorites();try{currentSettings=await post('/api/favorites/use',{id});selectedHex='';notify('Lieu changé. Recherche des avions à proximité…');await refresh();}catch(error){notify(error.message);}finally{favoriteBusy=false;renderFavorites();}}
const UPDATE_FAMILY='FlightBoard.ESP32S3.Application.v2';
function stableVersion(value){
  if(typeof value!=='string'||! /^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)$/.test(value))return null;
  const parts=value.split('.').map(Number);return parts.every(Number.isSafeInteger)?parts:null;
}
function newerVersion(next,current){const a=stableVersion(next),b=stableVersion(current);if(!a||!b)throw new Error('Version du programme non reconnue. Utilisez un fichier manuel.');for(let i=0;i<3;i++){if(a[i]!==b[i])return a[i]>b[i];}return false;}
function releaseUrl(value){
  if(typeof value!=='string'||value!==value.trim()||value.length>2048||value.includes('\\'))throw new Error('Adresse de mise à jour invalide.');
  let url;try{url=new URL(value);}catch(error){throw new Error('Adresse de mise à jour invalide.');}
  if(url.protocol!=='https:'||url.username||url.password||url.hash||/%(?:2f|5c|25)/i.test(url.pathname))throw new Error('L’adresse de mise à jour doit être une URL HTTPS directe, sans identifiants.');
  return url;
}
function validateRelease(value,info){
  if(!value||typeof value!=='object'||Array.isArray(value)||value.schema!==1||!stableVersion(value.version))throw new Error('Le catalogue de mises à jour est invalide.');
  if(info.firmwareFamily!==UPDATE_FAMILY||value.firmwareFamily!==UPDATE_FAMILY)throw new Error('Cette mise à jour n’est pas compatible avec ce Flight Board.');
  if(!Number.isSafeInteger(value.size)||value.size<32||!Number.isSafeInteger(info.maxFirmwareBytes)||value.size>info.maxFirmwareBytes)throw new Error('Le programme proposé est trop grand ou sa taille est invalide.');
  if(typeof value.sha256!=='string'||! /^[a-f0-9]{64}$/i.test(value.sha256)||typeof value.notes!=='string'||value.notes.length>4000)throw new Error('Le catalogue de mises à jour est incomplet.');
  const manifest=releaseUrl(info.releaseManifestUrl),binary=releaseUrl(value.url),directory=manifest.pathname.slice(0,manifest.pathname.lastIndexOf('/')+1);
  if(binary.origin!==manifest.origin||!binary.pathname.startsWith(directory)||! /\.bin$/i.test(binary.pathname))throw new Error('Le programme doit provenir du même serveur et du même dossier que le catalogue.');
  return{schema:1,version:value.version,firmwareFamily:value.firmwareFamily,url:binary.href,size:value.size,sha256:value.sha256.toLowerCase(),notes:value.notes,manifestUrl:manifest.href};
}
function cancelReleaseRequest(){++releaseRun;if(releaseController)releaseController.abort();releaseController=null;releaseBusy=false;}
function updateIsCurrent(epoch,run=releaseRun){return epoch===updateEpoch&&run===releaseRun&&el('updateDialog').open&&!demo;}
function updateControls(){
  const ready=Boolean(systemInfo?.otaSupported&&!systemInfo.updateBusy),busy=uploadBusy||releaseBusy;
  el('checkUpdateBtn').disabled=!ready||busy||!systemInfo.releaseManifestUrl;
  el('firmwareFile').disabled=!ready||busy;
  el('updateCloseBtn').disabled=uploadBusy;
  el('updateBtn').hidden=!firmwareReady&&!releaseInfo&&!uploadBusy&&el('updateBtn').dataset.reload!=='true';
  if(el('updateBtn').dataset.reload==='true'){el('updateBtn').disabled=busy;return;}
  el('updateBtn').disabled=!ready||busy||(!firmwareReady&&!releaseInfo);
  el('updateBtn').textContent=releaseInfo&&!firmwareReady?'Installer la version '+releaseInfo.version:'Installer ce programme';
}
async function downloadReleaseBytes(url,maxBytes,timeoutMs,controller,progress){
  let timedOut=false,reader;
  const timer=setTimeout(()=>{timedOut=true;controller.abort();},timeoutMs);
  try{
    const response=await fetch(url,{mode:'cors',credentials:'omit',redirect:'error',cache:'no-store',referrerPolicy:'no-referrer',signal:controller.signal});
    if(response.redirected||response.type==='opaqueredirect')throw new Error('Le lien de mise à jour n’est pas disponible. Réessayez plus tard.');
    if(!response.ok)throw new Error(response.status===404?'La mise à jour n’est pas disponible à cette adresse. Réessayez plus tard.':'Le service de mises à jour est indisponible. Réessayez plus tard.');
    const declared=response.headers.get('Content-Length');
    if(declared&&/^\d+$/.test(declared)&&Number(declared)>maxBytes)throw new Error('Le téléchargement dépasse la taille autorisée.');
    if(!response.body?.getReader)throw new Error('Ce navigateur ne permet pas le téléchargement vérifié. Utilisez un fichier manuel.');
    reader=response.body.getReader();const chunks=[];let total=0;
    for(;;){const part=await reader.read();if(controller.signal.aborted)throw new Error('Téléchargement annulé.');if(part.done)break;total+=part.value.byteLength;if(total>maxBytes)throw new Error('Le téléchargement dépasse la taille autorisée.');chunks.push(part.value);if(progress)progress(total);}
    const bytes=new Uint8Array(total);let offset=0;for(const chunk of chunks){bytes.set(chunk,offset);offset+=chunk.byteLength;}return bytes.buffer;
  }catch(error){
    if(timedOut)throw new Error('Le téléchargement a pris trop de temps. Réessayez.');
    if(error instanceof TypeError)throw new Error('Impossible de joindre le service de mises à jour. Vérifiez Internet sur la tablette, puis réessayez.');
    throw error;
  }finally{clearTimeout(timer);if(reader)try{await reader.cancel();}catch(error){/* Already closed or aborted. */}}
}
async function openUpdate(){
  if(demo||uploadBusy)return;cancelReleaseRequest();el('settingsDialog').close();el('updateDialog').showModal();const epoch=++updateEpoch;
  systemInfo=null;firmwareReady=null;releaseInfo=null;++firmwareValidationRun;
  el('manualUpdate').open=false;el('updateBtn').dataset.reload='false';el('firmwareFile').value='';el('firmwareInfo').hidden=true;el('releaseInfo').hidden=true;el('updateProgress').hidden=true;el('updateFeedback').textContent='';el('systemInfo').textContent='Lecture du Flight Board…';updateControls();
  try{const result=await requestJson('/api/system');if(!updateIsCurrent(epoch))return;systemInfo=result;el('systemInfo').textContent='Version installée : '+result.version;
    if(!result.otaSupported)el('updateFeedback').textContent='Cette installation nécessite une mise à jour par USB.';
    else if(result.updateBusy)el('updateFeedback').textContent='Une mise à jour est déjà en cours. Réessayez après le redémarrage.';
    else if(!result.releaseManifestUrl)el('updateFeedback').textContent='Les mises à jour à distance ne sont pas encore configurées. Vous pouvez installer un fichier manuellement.';
  }catch(error){if(updateIsCurrent(epoch))el('updateFeedback').textContent=error.message;}finally{if(updateIsCurrent(epoch))updateControls();}
}
async function checkRemoteUpdate(){
  if(demo||uploadBusy||releaseBusy||!systemInfo?.otaSupported||systemInfo.updateBusy||!systemInfo.releaseManifestUrl)return;
  cancelReleaseRequest();const epoch=updateEpoch,run=++releaseRun,controller=new AbortController();releaseController=controller;releaseBusy=true;releaseInfo=null;firmwareReady=null;++firmwareValidationRun;
  el('firmwareFile').value='';el('firmwareInfo').hidden=true;el('releaseInfo').hidden=true;el('updateProgress').hidden=true;el('updateFeedback').textContent='Recherche d’une mise à jour…';updateControls();
  try{const url=releaseUrl(systemInfo.releaseManifestUrl),bytes=await downloadReleaseBytes(url.href,16384,15000,controller);if(!updateIsCurrent(epoch,run))return;
    let value;try{value=JSON.parse(new TextDecoder('utf-8',{fatal:true}).decode(bytes));}catch(error){throw new Error('Le catalogue de mises à jour est illisible.');}
    const release=validateRelease(value,systemInfo);
    if(!newerVersion(release.version,systemInfo.version)){el('updateFeedback').textContent='Votre Flight Board est à jour (version '+systemInfo.version+').';return;}
    releaseInfo=release;el('releaseVersion').textContent='Version '+release.version+' disponible';el('releaseNotes').textContent=release.notes;el('releaseInfo').hidden=false;el('updateFeedback').textContent='Prête à installer. Vos réglages seront conservés.';
  }catch(error){if(updateIsCurrent(epoch,run))el('updateFeedback').textContent=error.message;}finally{if(updateIsCurrent(epoch,run)){releaseBusy=false;releaseController=null;updateControls();}}
}
async function validateFirmware(){
  if(uploadBusy||releaseBusy)return;cancelReleaseRequest();releaseInfo=null;el('releaseInfo').hidden=true;
  const run=++firmwareValidationRun,epoch=updateEpoch;firmwareReady=null;el('firmwareInfo').hidden=true;el('updateFeedback').textContent='';updateControls();const file=el('firmwareFile').files[0];if(!file||!systemInfo?.otaSupported||systemInfo.updateBusy)return;
  if(!/\.bin$/i.test(file.name)){el('updateFeedback').textContent='Choisissez un fichier .bin du programme Flight Board.';return;}
  if(file.size<32||!Number.isFinite(systemInfo.maxFirmwareBytes)||file.size>systemInfo.maxFirmwareBytes){el('updateFeedback').textContent='Le fichier est vide ou trop grand pour ce Flight Board.';return;}
  try{const prefix=new Uint8Array(await file.slice(0,32).arrayBuffer());if(run!==firmwareValidationRun||!updateIsCurrent(epoch))return;if(prefix[0]!==0xE9){el('updateFeedback').textContent='Ce fichier ne semble pas être un programme ESP32. Choisissez le bon fichier .bin.';return;}firmwareReady=file;el('firmwareInfo').textContent=file.name+' · '+fmt(file.size/1024)+' Ko';el('firmwareInfo').hidden=false;el('updateFeedback').textContent='Fichier prêt. Lancez l’installation quand vous êtes prêt.';updateControls();}catch(error){if(run===firmwareValidationRun&&updateIsCurrent(epoch))el('updateFeedback').textContent='Impossible de lire ce fichier. Sélectionnez-le de nouveau.';}
}
async function installRemoteFirmware(){
  if(demo||uploadBusy||releaseBusy||!releaseInfo||!systemInfo?.otaSupported||!el('updateDialog').open)return;
  const release=releaseInfo,epoch=updateEpoch,run=++releaseRun,controller=new AbortController();releaseController=controller;releaseBusy=true;el('updateProgress').value=0;el('updateProgress').hidden=false;el('updateFeedback').textContent='Téléchargement de la version '+release.version+'…';updateControls();
  try{const bytes=await downloadReleaseBytes(release.url,release.size,60000,controller,count=>{if(updateIsCurrent(epoch,run))el('updateProgress').value=Math.round(count/release.size*100);});if(!updateIsCurrent(epoch,run))return;
    if(bytes.byteLength!==release.size||new Uint8Array(bytes)[0]!==0xE9)throw new Error('Le programme téléchargé est incomplet ou invalide.');
    el('updateFeedback').textContent='Vérification du programme…';
    if(typeof sha256!=='function')throw new Error('La vérification du programme est indisponible. Rechargez la page.');
    if(sha256(bytes).toLowerCase()!==release.sha256)throw new Error('Le contrôle du programme a échoué. Aucun fichier n’a été installé.');
    firmwareReady=new File([bytes],'FlightBoard-'+release.version+'.bin',{type:'application/octet-stream'});releaseBusy=false;releaseController=null;
    await installFirmware(release);
  }catch(error){if(updateIsCurrent(epoch,run)){firmwareReady=null;el('updateFeedback').textContent=error.message;el('updateProgress').hidden=true;}}
  finally{if(updateIsCurrent(epoch,run)){releaseBusy=false;releaseController=null;updateControls();}}
}
function uploadFirmware(file,info){return new Promise((resolve,reject)=>{const xhr=new XMLHttpRequest();xhr.open('POST','/api/update');xhr.timeout=120000;xhr.setRequestHeader('X-Update-Token',info.updateToken);xhr.setRequestHeader('X-Firmware-Size',String(file.size));xhr.upload.onprogress=event=>{if(event.lengthComputable){const percent=Math.min(95,Math.round(event.loaded/event.total*95));el('updateProgress').value=percent;el('updateFeedback').textContent=percent>=95?'Programme transmis. Vérification par le Flight Board…':'Envoi du programme · '+percent+' %';}};xhr.onerror=()=>reject(new Error('Connexion interrompue. Reconnectez la tablette puis vérifiez la version du Flight Board.'));xhr.ontimeout=()=>reject(new Error('Le délai d’installation est dépassé. Vérifiez la connexion avant de réessayer.'));xhr.onload=()=>{let result;try{result=JSON.parse(xhr.responseText);}catch(error){reject(new Error('Réponse inattendue. Vérifiez la version du Flight Board avant de réessayer.'));return;}if(xhr.status<200||xhr.status>=300||result.ok===false){reject(new Error(result.error||'Installation refusée par le Flight Board.'));return;}resolve(result);};const body=new FormData();body.append('firmware',file,file.name);xhr.send(body);});}
async function waitForRestart(previous){const deadline=Date.now()+60000;while(Date.now()<deadline){await new Promise(resolve=>setTimeout(resolve,2500));try{const now=await requestJson('/api/system',{},3500);if(now.updateToken!==previous.updateToken||now.uptimeSeconds<previous.uptimeSeconds||now.version!==previous.version)return now;}catch(error){/* Le redémarrage coupe normalement la connexion. */}}throw new Error('Programme transmis, mais le redémarrage n’a pas pu être confirmé. Reconnectez la tablette puis rechargez la page.');}
async function installFirmware(release=null){
  if(demo||uploadBusy||!firmwareReady||!systemInfo?.otaSupported||!el('updateDialog').open)return;const file=firmwareReady;let previous=systemInfo;uploadBusy=true;updateControls();el('updateProgress').value=0;el('updateProgress').hidden=false;el('updateFeedback').textContent='Installation en cours. Gardez le Flight Board alimenté.';render();
  try{previous=await requestJson('/api/system');if(!previous.otaSupported||previous.updateBusy||file.size>previous.maxFirmwareBytes)throw new Error('Le Flight Board n’est pas prêt pour ce programme. Fermez puis rouvrez la mise à jour.');
    if(release){const checked=validateRelease(release,previous);if(checked.manifestUrl!==release.manifestUrl||!newerVersion(checked.version,previous.version))throw new Error('La version ou le serveur du Flight Board a changé. Recherchez de nouveau la mise à jour.');}
    await uploadFirmware(file,previous);el('updateProgress').value=100;el('updateFeedback').textContent='Programme installé. Le Flight Board redémarre…';const updated=await waitForRestart(previous);if(release&&updated.version!==release.version){firmwareReady=null;releaseInfo=null;el('updateBtn').dataset.reload='true';el('updateBtn').textContent='Recharger l’interface';throw new Error('Le Flight Board a redémarré en version '+(updated.version||'inconnue')+', au lieu de '+release.version+'. Rechargez l’interface avant de réessayer.');}el('updateFeedback').textContent='Mise à jour terminée · Version '+updated.version+'. Rechargez la page pour ouvrir la nouvelle interface.';el('updateBtn').textContent='Recharger l’interface';el('updateBtn').dataset.reload='true';firmwareReady=null;releaseInfo=null;notify('Le Flight Board a redémarré. Rechargez la page pour afficher sa nouvelle interface.');}
  catch(error){el('updateFeedback').textContent=error.message;firmwareReady=null;el('firmwareFile').value='';el('firmwareInfo').hidden=true;}
  finally{uploadBusy=false;updateControls();refresh();}
}
function closeUpdate(){++updateEpoch;++firmwareValidationRun;cancelReleaseRequest();firmwareReady=null;releaseInfo=null;}

AIRLINE_OPTIONS.forEach(([id,name])=>el('filterAirline').add(new Option(name,id)));
document.querySelectorAll('[data-nav]').forEach(button=>button.addEventListener('click',()=>{if(button.dataset.nav==='home'){storeMode=false;contentNotice('');}showModule(button.dataset.nav);}));
el('libraryModeBtn').addEventListener('click',()=>{storeMode=false;libraryCategory='all';contentNotice('');showModule('home');loadContent().then(warmContentThumbnails).catch(()=>{});});
el('storeModeBtn').addEventListener('click',()=>openStore());el('storeRefreshBtn').addEventListener('click',()=>openStore(true));el('restoreLogosBtn').addEventListener('click',restoreLogos);el('contentRemoveBtn').addEventListener('click',removeContent);
el('widgetForm').addEventListener('submit',event=>{event.preventDefault();saveWidget();});
['messageText','messageColor','timerMinutes','timerSeconds'].forEach(id=>el(id).addEventListener('input',updateWidgetDraft));
el('scoreLeague').addEventListener('change',resetScoreSearch);el('scoreDate').addEventListener('change',resetScoreSearch);el('scoreSearchBtn').addEventListener('click',searchMatches);el('scoreEvent').addEventListener('change',updateWidgetDraft);
document.querySelectorAll('[data-timer]').forEach(button=>button.addEventListener('click',()=>timerCommand(button.dataset.timer)));
el('librarySearch').addEventListener('input',event=>{libraryQuery=event.target.value;renderLibrary();});
document.querySelectorAll('[data-library-filter]').forEach(button=>button.addEventListener('click',()=>{libraryCategory=button.dataset.libraryFilter;renderLibrary();warmContentThumbnails();}));
document.querySelectorAll('[data-module]').forEach(button=>button.addEventListener('click',()=>showModule(button.dataset.module)));
document.querySelectorAll('[data-visual]').forEach(button=>button.addEventListener('click',()=>chooseVisual(button.dataset.visual)));
el('homeBtn').addEventListener('click',()=>showModule('home'));el('activateBtn').addEventListener('click',activateModule);
el('moduleSettingsBtn').addEventListener('click',()=>{el('locationSection').open=currentView==='weather';el('nightSection').open=currentView!=='weather';openSettings();});
el('filtersBtn').addEventListener('click',openFilters);el('filtersCloseBtn').addEventListener('click',()=>el('filtersDialog').close());el('filtersForm').addEventListener('submit',event=>{event.preventDefault();saveFilters();});el('filtersResetBtn').addEventListener('click',()=>saveFilters(true));

el('demoBtn').addEventListener('click',()=>setDemo(!demo));el('liveBtn').addEventListener('click',()=>setDemo(false));el('sendBtn').addEventListener('click',sendSelected);el('autoBtn').addEventListener('click',()=>setMode('auto'));el('clockBtn').addEventListener('click',()=>setMode('clock'));
document.querySelectorAll('[data-font-preview]').forEach(select=>select.addEventListener('change',renderFontSamples));
el('settingsBtn').addEventListener('click',openSettings);el('locationBtn').addEventListener('click',()=>{el('locationSection').open=true;openSettings();});el('cancelBtn').addEventListener('click',()=>el('settingsDialog').close());el('settingsDialog').addEventListener('close',()=>++settingsEpoch);el('settingsForm').addEventListener('submit',saveSettings);el('cityInput').addEventListener('change',selectCity);
el('brightnessInput').addEventListener('input',()=>queueBrightness('brightness'));el('nightBrightness').addEventListener('input',()=>queueBrightness('nightBrightness'));el('nightEnabled').addEventListener('change',nightControls);el('nightAction').addEventListener('change',nightControls);el('favoriteSelect').addEventListener('change',useFavorite);el('favoriteSaveBtn').addEventListener('click',addFavorite);
el('fullscreenBtn').addEventListener('click',async()=>{try{if(document.fullscreenElement)await document.exitFullscreen();else if(document.documentElement.requestFullscreen)await document.documentElement.requestFullscreen();else notify('Le plein écran n’est pas disponible dans ce navigateur.');}catch(error){notify('Le navigateur n’a pas autorisé le plein écran.');}});document.addEventListener('fullscreenchange',()=>{el('fullscreenBtn').setAttribute('aria-label',document.fullscreenElement?'Quitter le plein écran':'Passer en plein écran');});
el('wifiBtn').addEventListener('click',()=>{if(!demo)openWifi();});el('wifiCloseBtn').addEventListener('click',()=>el('wifiDialog').close());el('wifiDialog').addEventListener('close',()=>{wifiClosed();refresh();});el('wifiForm').addEventListener('submit',saveWifi);el('wifiScanBtn').addEventListener('click',()=>scanWifi(true));el('wifiStatusBtn').addEventListener('click',beginWifiStatus);el('wifiShowPassword').addEventListener('change',()=>el('wifiPassword').type=el('wifiShowPassword').checked?'text':'password');el('wifiOpen').addEventListener('change',()=>{clearWifiPassword();updateWifiControls();});el('wifiNetworks').addEventListener('change',()=>{const option=el('wifiNetworks').selectedOptions[0];if(!option||!option.value)return;el('wifiSsid').value=option.value;el('wifiOpen').checked=option.dataset.open==='true';clearWifiPassword();updateWifiControls();});el('wifiSsid').addEventListener('input',()=>{el('wifiNetworks').value='';el('wifiOpen').checked=false;updateWifiControls();});
el('openUpdateBtn').addEventListener('click',openUpdate);el('updateCloseBtn').addEventListener('click',()=>{if(!uploadBusy)el('updateDialog').close();});el('updateDialog').addEventListener('cancel',event=>{if(uploadBusy)event.preventDefault();});el('updateDialog').addEventListener('close',closeUpdate);el('checkUpdateBtn').addEventListener('click',checkRemoteUpdate);el('firmwareFile').addEventListener('change',validateFirmware);el('updateBtn').addEventListener('click',()=>el('updateBtn').dataset.reload==='true'?location.reload():releaseInfo&&!firmwareReady?installRemoteFirmware():installFirmware());
el('previewToggle').addEventListener('toggle',drawPreview);document.addEventListener('visibilitychange',drawPreview);
if(idleMotion.addEventListener)idleMotion.addEventListener('change',drawPreview);else idleMotion.addListener(drawPreview);
refresh();loadFavorites();loadContent().then(warmContentThumbnails).catch(()=>{});checkInitialWifi();setInterval(refresh,4000);setInterval(()=>{drawPreview();if(!demo&&latest&&!usable(latest))render();},1000);

</script>
</body>
</html>
)FBPAGE";
