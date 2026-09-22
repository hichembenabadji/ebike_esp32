const { createCanvas, GlobalFonts } = require('@napi-rs/canvas');
const fs = require('fs');
const FD = __dirname + "/";
const FA = __dirname + "/../../.pio/libdeps/jc4827w543/lvgl/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff";
GlobalFonts.registerFromPath(FD + "SpaceGrotesk-Bold.ttf", "SGB");
GlobalFonts.registerFromPath(FD + "SpaceGrotesk-Medium.ttf", "SGM");
GlobalFonts.registerFromPath(FD + "SpaceMono-Regular.ttf", "SMR");
GlobalFonts.registerFromPath(FD + "SpaceMono-Bold.ttf", "SMB");
GlobalFonts.registerFromPath(FA, "FA");

const W = 480, H = 272, SCALE = 2;
const C = { bg:'#000000', card:'#000000', cardHi:'#333333', border:'#4A4A4A', text:'#FFE600', label:'#FFC800',
  green:'#00E676', orange:'#FF8C00', red:'#FF1744', track:'#2E2E2E', dashOff:'#3A3A3A', rowSel:'#0B2308' };
const ICO = { sun:'', tl:'', tr:'', warn:'', horn:'', leaf:'', bike:'', bolt:'',
  batt:'', ruler:'', stopw:'', chev:'' };
const MET = { SGB:{asc:0.984, desc:0.292}, SGM:{asc:0.984, desc:0.292}, SMR:{asc:1.12, desc:0.361}, SMB:{asc:1.12, desc:0.361}, FA:{asc:0.9, desc:0.1} };
const lh = (fam, size) => (MET[fam].asc + MET[fam].desc) * size;

const cv = createCanvas(W * SCALE, H * SCALE);
const ctx = cv.getContext('2d');
ctx.scale(SCALE, SCALE);

function rrect(x, y, w, h, r, fill, stroke, sw = 1) {
  ctx.beginPath(); ctx.roundRect(x, y, w, h, r);
  if (fill) { ctx.fillStyle = fill; ctx.fill(); }
  if (stroke) { ctx.lineWidth = sw; ctx.strokeStyle = stroke; ctx.stroke(); }
}
function textW(txt, fam, size, ls = 0) { ctx.font = `${size}px ${fam}`; return ctx.measureText(txt).width + ls * Math.max(0, txt.length - 1); }
function drawText(txt, fam, size, x, baseline, color, ls = 0, align = 'left') {
  ctx.font = `${size}px ${fam}`; ctx.fillStyle = color; ctx.textBaseline = 'alphabetic';
  const w = textW(txt, fam, size, ls);
  let cx = align === 'left' ? x : align === 'center' ? x - w / 2 : x - w;
  if (ls === 0) { ctx.textAlign = 'left'; ctx.fillText(txt, cx, baseline); return; }
  for (const ch of txt) { ctx.fillText(ch, cx, baseline); cx += ctx.measureText(ch).width + ls; }
}
const lblTop    = (t, fam, s, x, yTop, col, ls = 0, al = 'left') => drawText(t, fam, s, x, yTop + MET[fam].asc * s, col, ls, al);
const lblCenter = (t, fam, s, cx, cy, col, ls = 0) => lblTop(t, fam, s, cx, cy - lh(fam, s) / 2, col, ls, 'center');
const lblBottom = (t, fam, s, x, yBot, col, ls = 0, al = 'left') => lblTop(t, fam, s, x, yBot - lh(fam, s), col, ls, al);

function card(x, y, w, h, pad) { rrect(x, y, w, h, 12, C.card, C.border); return { cx: x + 1 + pad, cy: y + 1 + pad, cw: w - 2 - 2 * pad, ch: h - 2 - 2 * pad }; }
function caption(cx, cy, icon, text) { lblTop(icon, 'FA', 12, cx, cy, C.label); lblTop(text, 'SGM', 12, cx + 18, cy, C.label, 1); }
function dashes(x, y, count, level, w, h, gap) { for (let k = 0; k < count; k++) rrect(x + k * (w + gap), y, w, h, h / 2, k < level ? C.green : C.dashOff); }
const rad = d => d * Math.PI / 180;
const battColor = b => b > 20 ? C.green : b > 10 ? C.orange : C.red;
const battStatus = b => b > 20 ? 'OK' : b > 10 ? 'LOW' : 'CRITICAL';

function drawDashboard(d) {
  ctx.fillStyle = C.bg; ctx.fillRect(0, 0, W, H);
  let c = card(6, 6, 468, 56, 4);
  const tb = [[ICO.sun,'LIGHT',d.headlight,C.green],[ICO.tl,'LEFT',d.left||d.hazard,C.orange],[ICO.tr,'RIGHT',d.right||d.hazard,C.orange],[ICO.warn,'WARNING',d.hazard,C.orange],[ICO.horn,'HORN',false,C.text]];
  tb.forEach(([ico, lab, on, onCol], i) => {
    const bx = c.cx + i * 92, by = c.cy, col = on ? onCol : C.label;
    if (i === d.pressed) rrect(bx, by, 88, 46, 10, C.cardHi);
    lblTop(ico, 'FA', 20, bx + 44, by + 3, col, 0, 'center');
    lblBottom(lab, 'SGM', 12, bx + 44, by + 46 - 4, col, 1, 'center');
  });
  c = card(6, 68, 126, 94, 7);
  caption(c.cx, c.cy, ICO.leaf, 'ASSIST');
  lblTop(ICO.chev, 'FA', 12, c.cx + c.cw, c.cy, C.label, 0, 'right');
  lblTop(d.mode.name, 'SGB', 34, c.cx, c.cy + 16, C.text);
  dashes(c.cx, c.cy + 66, 3, d.mode.level, 22, 4, 5);
  c = card(6, 168, 126, 98, 7);
  caption(c.cx, c.cy, ICO.batt, 'BATTERY');
  const bcol = battColor(d.batt);
  const rowB = c.cy + 14 + 44;
  lblBottom(String(d.batt), 'SGB', 34, c.cx, rowB, C.text);
  const nw = textW(String(d.batt), 'SGB', 34);
  lblBottom('%', 'SGB', 16, c.cx + nw + 4, rowB - 6, C.label);
  rrect(c.cx, c.cy + 59, 110, 6, 3, C.track); rrect(c.cx, c.cy + 59, 110 * d.batt / 100, 6, 3, bcol);
  lblBottom('STATUS', 'SGM', 12, c.cx, c.cy + c.ch, C.label, 1);
  lblBottom(battStatus(d.batt), 'SGM', 12, c.cx + c.cw, c.cy + c.ch, bcol, 1, 'right');
  c = card(138, 68, 210, 198, 0);
  const gx = c.cx + 104, gy = c.cy + 105, R = 80, AW = 14;
  const arc = (a0, a1, col) => { ctx.beginPath(); ctx.lineWidth = AW; ctx.lineCap = 'round'; ctx.strokeStyle = col; ctx.arc(gx, gy, R - AW / 2, rad(a0), rad(a1)); ctx.stroke(); };
  arc(135, 405, C.track);
  arc(338, 405, C.red);
  if (d.speed > 0) arc(135, 135 + 270 * d.speed / 60, C.green);
  for (let i = 0; i <= 6; i++) {
    const v = i * 10, a = rad(135 + 270 * v / 60), cs = Math.cos(a), sn = Math.sin(a);
    ctx.beginPath(); ctx.lineWidth = 2; ctx.lineCap = 'butt'; ctx.strokeStyle = C.label;
    ctx.moveTo(Math.round(gx + 62 * cs), Math.round(gy + 62 * sn)); ctx.lineTo(Math.round(gx + 84 * cs), Math.round(gy + 84 * sn)); ctx.stroke();
    const lx = Math.round(gx + 92 * cs) - 15, ly = Math.round(gy + 92 * sn) - 9;
    lblTop(String(v), 'SMR', 12, lx + 15, ly, v >= 45 ? C.red : C.label, 0, 'center');
  }
  lblCenter(String(Math.round(d.speed)), 'SGB', 72, gx, gy - 16, C.text);
  lblCenter('KM/H', 'SGB', 16, gx, gy + 19, C.label, 2);
  lblCenter(`ODO ${Math.floor(d.odo)} km`, 'SMR', 12, gx, gy + 37, C.label);
  c = card(348, 68, 126, 94, 7);
  caption(c.cx, c.cy, ICO.ruler, 'TRIP');
  const rowT = c.cy + 22 + 44;
  const ts = d.trip.toFixed(1);
  lblBottom(ts, 'SGB', 34, c.cx, rowT, C.text);
  lblBottom('km', 'SGB', 16, c.cx + textW(ts, 'SGB', 34) + 4, rowT - 6, C.label);
  c = card(348, 168, 126, 98, 7);
  caption(c.cx, c.cy, ICO.stopw, 'TIME');
  lblTop(d.time, 'SMB', 20, c.cx, c.cy + 18, C.text);
  lblBottom('AVG', 'SGM', 12, c.cx, c.cy + c.ch - 2, C.label, 1);
  lblBottom(`${d.avg} km/h`, 'SGB', 16, c.cx + c.cw, c.cy + c.ch, C.text, 0, 'right');
}

function drawOverlay(modeIdx) {
  ctx.fillStyle = C.bg; ctx.fillRect(0, 0, W, H);
  rrect(90, 8, 300, 256, 16, C.card, C.label);
  const sx = 90 + 1 + 8, sy = 8 + 1 + 8;
  lblTop('ASSIST MODE', 'SGB', 16, sx, sy, C.text, 1);
  const modes = [['ECO','Maximum range',ICO.leaf,1],['TOUR','Balanced power / range',ICO.bike,2],['SPORT','Maximum power',ICO.bolt,3]];
  modes.forEach(([n, desc, ico, lvl], i) => {
    const rx = sx, ry = sy + 24 + i * 58, sel = i === modeIdx;
    rrect(rx, ry, 282, 52, 10, sel ? C.rowSel : C.card, sel ? C.green : C.border, sel ? 2 : 1);
    lblCenter(ico, 'FA', 20, rx + 10 + 10, ry + 26, C.label);
    lblTop(n, 'SGB', 20, rx + 44, ry + 5, C.text);
    lblTop(desc, 'SGM', 12, rx + 44, ry + 31, C.label);
    if (sel) lblCenter('CURRENT', 'SGM', 12, rx + 282 - 10 - textW('CURRENT','SGM',12,1) / 2, ry + 26 - 11, C.green, 1);
    dashes(rx + 282 - 10 - 50, ry + 26 + 10 - 2, 3, lvl, 14, 4, 4);
  });
  const cy0 = sy + 24 + 174;
  rrect(sx, cy0, 282, 40, 10, null, C.label);
  lblCenter('CANCEL', 'SGB', 16, sx + 141, cy0 + 20, C.text, 2);
}

const OUT = __dirname + '/../../docs/';
const data = { speed: 38, batt: 84, mode: { name: 'TOUR', level: 2 }, trip: 12.4, odo: 19018, time: '00:28:14', avg: 21, headlight: true, hazard: false, left: false, right: false, pressed: -1 };
drawDashboard(data);
fs.writeFileSync(OUT + 'apercu_tableau_de_bord.png', cv.toBuffer('image/png'));
drawDashboard({ ...data, speed: 52, batt: 14, mode: { name: 'SPORT', level: 3 }, trip: 123.4, hazard: true, headlight: false, pressed: 3 });
fs.writeFileSync(OUT + 'apercu_alerte_warning.png', cv.toBuffer('image/png'));
drawOverlay(1);
fs.writeFileSync(OUT + 'apercu_selecteur_mode.png', cv.toBuffer('image/png'));
console.log('ok');
