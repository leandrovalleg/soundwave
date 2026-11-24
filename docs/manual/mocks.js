// Vector recreations of the SoundWave interface for the manual (same fonts, dot icons and colours as the app).
const RED = '#e11d1d', INK = '#111', INK2 = '#8a8a8a', LINE = '#e2e2e2', SUB = '#f2f2f2', BG = '#efefef';
const G = {
  play: '1......121....12221..122222312221..121....1......', stop: '1111111122222112222211222221122222112222211111111',
  rec: '..111...13331.133333113333311333331.13331...111..', loop: '..3.....3111111.3...11.....11...3.1111113.....3..',
  browser: '111111112....112....112....112....112....11111111', panel: '11111111.....11.....11111111122222112222211111111',
  timeline: '1111............33333.......111..............2222', grid: '111.333111.333111.333.......222.111222.111222.111',
  plus: '...1......1......1...1113111...1......1......1...', gear: '2..1..2.11111..1...1.11.3.11.1...1..11111.2..1..2',
  exportUp: '...3.....333...3.3.3....3......1...1.....11111111', power: '...1....1.1.1.1..1..11.....11.....1.1...1...111..',
  window: '111111111111111.....11.....11.....11.....11111111', arrowL: '...1.....1.....1.....1111111.1.......1.......1...',
  arrowR: '...1.......1.......1.1111111.....1.....1.....1...', dup: '1111...1..1...1..11111111..1...1..1...1..1...1111',
  cross: '1.....1.1...1...1.1.....1.....1.1...1...1.1.....1',
};
const T5 = { // plugin type icons (5x5), as in the library tiles
  SINTE: '111111.1.11.1.111111.....', 'EQ / FILTRO': '.....1...1.1.1...1.......', REVERB: '111111...11.1.11...111111',
  DELAY: '.....1.1.11.1.1..........', PITCH: '..1...111...1....1....1..', 'DINÁMICA': '1....1.1..1.1.11.1.111111',
  'MODULACIÓN': '......1...1.1.1...1......', 'DISTORSIÓN': '1...1.1.1...1...1.1.1...1',
};
const glyph = (name, color = INK, px = 2.2, gap = 3.1) => {
  const b = G[name]; let s = `<svg width="${gap * 7}" height="${gap * 7}" style="display:block">`;
  for (let i = 0; i < 49; i++) if (b[i] !== '.') s += `<rect x="${(i % 7) * gap}" y="${Math.floor(i / 7) * gap}" width="${px}" height="${px}" rx="${px * .3}" fill="${color}" opacity="${b[i] === '2' ? .45 : 1}"/>`;
  return s + '</svg>';
};
const glyph5 = (type, color, gap = 4, px = 3) => {
  const b = T5[type]; let s = `<svg width="${gap * 5}" height="${gap * 5}" style="display:block">`;
  for (let i = 0; i < 25; i++) if (b[i] === '1') s += `<rect x="${(i % 5) * gap}" y="${Math.floor(i / 5) * gap}" width="${px}" height="${px}" rx="1" fill="${color}"/>`;
  return s + '</svg>';
};
const knob = (v, size = 34, red = false) => { // v 0..1, 270° arc like the app
  const r = size / 2 - 3, c = size / 2, a0 = 135, a1 = 135 + 270 * v, P = a => [c + r * Math.cos(a * Math.PI / 180), c + r * Math.sin(a * Math.PI / 180)];
  const arc = (a, b) => { const [x0, y0] = P(a), [x1, y1] = P(b); return `M${x0} ${y0}A${r} ${r} 0 ${b - a > 180 ? 1 : 0} 1 ${x1} ${y1}`; };
  const [dx, dy] = [c + (r - 7) * Math.cos(a1 * Math.PI / 180), c + (r - 7) * Math.sin(a1 * Math.PI / 180)];
  return `<svg width="${size}" height="${size}"><path d="${arc(a0, 405)}" stroke="${LINE}" stroke-width="2.4" fill="none" stroke-linecap="round"/>` +
    (v > 0.01 ? `<path d="${arc(a0, a1)}" stroke="${red ? RED : INK}" stroke-width="2.4" fill="none" stroke-linecap="round"/>` : '') +
    `<circle cx="${dx}" cy="${dy}" r="2" fill="${INK}"/></svg>`;
};
const rnd = (seed => () => (seed = (seed * 16807) % 2147483647) / 2147483647)(7);
const wave = (w, h, kind) => { // waveform drawn as mirrored bars
  let p = ''; const n = Math.floor(w / 1.6);
  for (let i = 0; i < n; i++) {
    const t = i / n; let a;
    if (kind === 'kick') { const ph = (t * 4) % 1; a = Math.exp(-ph * 7) * (0.6 + 0.4 * rnd()); }
    else if (kind === 'hats') { const ph = (t * 8) % 1; a = ph < 0.08 ? 0.5 + 0.4 * rnd() : 0.04 * rnd(); }
    else if (kind === 'bass') a = 0.55 + 0.1 * Math.sin(t * 60);
    else a = 0.45 + 0.4 * rnd() * (0.6 + 0.4 * Math.sin(t * 9));
    const y = a * h / 2; p += `M${(i * 1.6).toFixed(1)} ${(h / 2 - y).toFixed(1)}V${(h / 2 + y).toFixed(1)}`;
  }
  return `<svg width="${w}" height="${h}"><path d="${p}" stroke="#4a4a4a" stroke-width="1"/></svg>`;
};
const pill = (txt, o = {}) => `<span class="m-pill ${o.on ? 'on' : ''} ${o.red ? 'red' : ''} ${o.soft ? 'soft' : ''}" style="${o.w ? 'width:' + o.w + 'px;' : ''}${o.style || ''}">${o.g ? glyph(o.g, o.gc || (o.on ? '#fff' : INK)) : ''}${txt ? `<span>${txt}</span>` : ''}${o.combo ? '<i class="chev"></i>' : ''}</span>`;
const combo = (txt, w) => pill(txt, { w, combo: true, style: 'justify-content:space-between' });

const M = {};
M.topbar = () => `<div class="m-top">
 <div class="r1"><span class="menu">ARCHIVO</span><span class="menu">EDICIÓN</span><span class="menu">CREAR</span><span class="menu">REPRODUCCIÓN</span><span class="menu">VER</span><span class="menu">AYUDA</span>
  <span style="flex:1"></span>${pill('BROWSER', { g: 'browser', soft: 1 })}${pill('PANEL', { g: 'panel', soft: 1 })}${pill('TIMELINE', { g: 'timeline', soft: 1 })}${pill('GRID', { g: 'grid' })}${pill('TRACK', { g: 'plus' })}${pill('EXPORT', { g: 'exportUp' })}${pill('SETTINGS', { g: 'gear' })}</div>
 <div class="r2"><span class="logo">SOUNDWAVE</span>${pill('PLAY', { g: 'play' })}${pill('TAP')}${pill('<b class="bpm">120.00</b> <small>BPM</small>', { w: 88 })}${combo('4/4', 64)}${pill('CLICK', { g: null, style: 'gap:6px', combo: true }).replace('<span>CLICK', '<span><i class="m2"></i>CLICK')}${combo('1 BAR', 72)}<i class="sep"></i>${pill('STOP', { g: 'stop' })}${pill('REC', { g: 'rec', gc: RED })}<b class="pos">3.4.2</b><i class="sep"></i>${pill('LOOP', { g: 'loop', gc: RED, red: 1 })}${combo('LOOP LEN', 82)}<span class="tiny">2 &gt; 6</span>
  <span style="flex:1"></span>${combo('KEY', 70)}${combo('MAJOR', 92)}<span class="tiny">CPU 6%</span></div></div>`;

const PLUG = [['CRUSH', 'DISTORSIÓN'], ['ECHO', 'DELAY'], ['FLUX', 'EQ / FILTRO'], ['GATE', 'DINÁMICA'], ['GLOW', 'DISTORSIÓN'], ['JET', 'MODULACIÓN'], ['ORBIT', 'MODULACIÓN'], ['PRISM', 'EQ / FILTRO'], ['ROOM', 'REVERB'], ['SPACE', 'REVERB'], ['SQUEEZE', 'DINÁMICA'], ['SWAY', 'MODULACIÓN']];
M.libPlugins = () => `<div class="m-card m-lib"><h4>LIBRARY</h4>
 <div class="tabs">${pill('SAMPLES')}${pill('PLUGINS', { on: 1 })}${pill('SOUNDWAVE')}</div>
 <div class="row">${pill('BUSCAR', { style: 'flex:1;justify-content:flex-start;color:#aaa;border-radius:10px' })}${combo('TODOS', 96)}</div>
 <div class="maker">SOUNDWAVE</div>
 <div class="tiles">${PLUG.map(([n, t], i) => `<div class="tile ${i === 2 ? 'sel' : ''}">${glyph5(t, i === 2 ? '#fff' : RED)}<b>${n}</b><small>${t}</small></div>`).join('')}</div>
 <div class="hint">ARRASTRA UN PLUGIN A UNA PISTA</div></div>`;
const SAMPLES = ['@helpsisleet - Bounce', 'African Dreams', 'Bachata', '01._Percussion_Loop', 'DIASPORA_requinto_percussion_1', 'DIASPORA_requinto_percussion_2', '02._Percussion_Loop', 'CYMATICS', 'DR DRE SOUND KIT', 'euphoria-jersey-club', 'The Sauce Kit Vol.1', 'UVST Drum Kit'];
const folder = c => `<svg width="18" height="14"><rect x="1" y="3" width="16" height="10" rx="1" fill="none" stroke="${c}" stroke-dasharray="1.6 1.6" stroke-width="1.6"/><rect x="1" y="1" width="7" height="3" rx="1" fill="none" stroke="${c}" stroke-dasharray="1.6 1.6" stroke-width="1.6"/></svg>`;
const fileIc = `<svg width="16" height="14">${[2, 5, 8, 11, 14].map((x, i) => `<rect x="${x - 1}" y="${7 - [2, 5, 3, 6, 2][i]}" width="2" height="${[4, 10, 6, 12, 4][i]}" rx="1" fill="#555"/>`).join('')}</svg>`;
M.libSamples = () => `<div class="m-card m-lib"><h4>LIBRARY</h4>
 <div class="tabs">${pill('SAMPLES', { on: 1 })}${pill('PLUGINS')}${pill('SOUNDWAVE')}</div>
 <div class="lab2">PLACES</div><div class="row">${combo('F:\\Drumkits', 170)}${pill('+', { w: 30 })}${pill('-', { w: 30 })}</div>
 <div class="files">${SAMPLES.map((n, i) => { const lvl = [0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0, 0][i], f = i < 4 || i > 5; return `<div class="file ${i === 4 ? 'sel' : ''}" style="padding-left:${10 + lvl * 16}px">${f ? folder(RED) : fileIc}<span>${n}</span></div>`; }).join('')}</div>
 <span style="flex:1"></span><div>${pill('PREVIEW', { on: 1, w: 110 })}</div></div>`;

const hdr = (n, name, o = {}) => `<div class="m-hdr ${o.sel ? 'sel' : ''}">${o.ret ? '' : `<i class="stripe" style="background:${o.midi ? RED : (o.sel ? '#fff' : INK)}"></i>`}
 <div class="t"><span>${o.ret ? 'RET' : String(n).padStart(2, '0')}&nbsp;&nbsp;${name}</span>${o.inst ? `<em>${o.inst}</em>` : ''}${o.ret ? '<i class="dot"></i>' : ''}</div>
 <div class="b">${['M', 'S', 'FX ' + (o.fx || 0)].map(x => `<span class="tg">${x}</span>`).join('')}${o.ret ? '' : `<span class="tg ${o.arm ? 'arm' : ''}">o</span><small>${o.midi ? 'MIDI' : 'NO IN'}</small>`}</div></div>`;
const io = (ret, o = {}) => `<div class="m-io">${ret ? '' : `<div class="l">${combo('NO INPUT', 122)}<div class="mon">${pill('IN')}${pill('AUTO')}${pill('OFF', { on: 1 })}</div></div>`}
 <div class="rr"><div class="kn">${knob(.5, 24)}${ret ? '' : `${knob(o.a || 0, 24, true)}<b>A</b>${knob(o.b || 0, 24, true)}<b>B</b>`}<span style="flex:1"></span>${ret ? '' : pill('o', { w: 28 })}</div>
 <div class="vol"><i style="width:${o.vol || 82}%"></i></div><div class="out"><small>OUT &gt; MAIN</small><b>0.0 dB</b></div></div></div>`;
const clip = (x, w, name, kind, o = {}) => `<div class="m-clip ${o.sel ? 'sel' : ''} ${o.midi ? 'midi' : ''}" style="left:${x}px;width:${w}px"><small>${name}</small>${kind === 'midi' ? midiMini(w - 12) : wave(w - 12, 34, kind)}</div>`;
const midiMini = w => { let s = `<svg width="${w}" height="34">`; [[0, 20, 6], [.12, 16, 9], [.25, 26, 5], [.38, 12, 10], [.55, 22, 6], [.7, 8, 12], [.85, 18, 8]].forEach(([x, y, l]) => s += `<rect x="${x * w}" y="${y}" width="${l}" height="2.4" rx="1" fill="${RED}"/>`); return s + '</svg>'; };
M.ruler = (w = 1150) => { const bars = [...Array(9)].map((_, i) => `<span style="left:${190 + i * 120}px">${i + 1}</span>`).join('');
  return `<div class="m-ruler" style="width:${w}px"><div class="ov"><div class="clips">${[[0, .18], [.02, .2], [.0, .3], [.05, .4]].map(([a, b], i) => `<i style="left:${a * 100}%;width:${(b - a) * 100}%;top:${2 + i * 2.5}px;background:${i === 3 ? RED : '#9a9a9a'}"></i>`).join('')}</div><div class="view"></div><i class="ph"></i></div>
  <div class="loop"><b style="left:${190 + 120}px;width:${120 * 4}px">LOOP</b></div><div class="bars">${bars}<span class="io">I/O</span></div><i class="playhead" style="left:${190 + 2.6 * 120}px"></i></div>`; };
M.tracks = (o = {}) => {
  const rows = [
    [1, 'TRACK 1', { sel: 1, fx: 2 }, [[30, 120, 'KICK 120', 'kick']], { a: .55 }],
    [2, 'TRACK 2', {}, [[30, 120, 'BASS C', 'bass']], { a: .2, b: .35 }],
    [3, 'TRACK 3', { arm: 1 }, [[30, 120, 'HATS', 'hats']], {}],
    [4, 'TRACK 4', {}, [[30, 240, 'PAD AM', 'pad']], { a: .7 }],
    [5, 'MIDI 1', { midi: 1, inst: 'SW TIDE', fx: 1 }, [[390, 120, 'MIDI', 'midi', { midi: 1 }]], { b: .4 }],
  ];
  return `<div class="m-tracks">${o.ruler ? M.ruler() : ''}<div class="lanes">${rows.map(([n, name, h, clips, iv]) => `<div class="m-row">${hdr(n, name, h)}<div class="lane">${clips.map(c => clip(...c)).join('')}</div>${io(false, iv)}</div>`).join('')}
  <div class="m-row">${hdr(0, 'A REVERB', { ret: 1, fx: 1 })}<div class="lane"></div>${io(true)}</div></div><i class="playhead" style="left:${190 + 2.6 * 120}px"></i></div>`;
};
M.io = () => `<div class="m-iozoom">${io(false, { a: .55, b: .25 })}</div>`;
M.piano = () => { let keys = '', grid = '', notes = '';
  for (let i = 0; i < 18; i++) { const blk = [1, 3, 6, 8, 10].includes(i % 12); keys += `<i class="${blk ? 'b' : 'w'}"></i>`; grid += `<i class="${blk ? 'dk' : ''}"></i>`; }
  [[1, 9, 3], [3.4, 7, 2], [6, 5, 4], [10, 9, 2], [12.2, 4, 3], [15, 8, 2], [17.5, 3, 4], [21, 11, 3], [24, 6, 2]].forEach(([x, y, l]) => notes += `<b style="left:${x * 26}px;top:${y * 15 + 1}px;width:${l * 26 - 2}px"></b>`);
  return `<div class="m-card m-piano"><div class="tools"><small>GRID</small>${combo('1/16', 90)}<small>LENGTH</small>${combo('4 BARS', 96)}${pill('QUANTISE', { w: 92 })}${pill('OCT -', { w: 64 })}${pill('OCT +', { w: 64 })}<span style="flex:1"></span><h4>MIDI</h4></div>
  <div class="pr"><div class="keys">${keys}</div><div class="nt">${grid}<div class="beats">${[...Array(28)].map((_, i) => `<i style="left:${i * 26}px;${i % 4 === 0 ? 'background:#cfcfcf' : ''}"></i>`).join('')}</div>${notes}<i class="ph" style="left:${16 * 26}px"></i></div></div></div>`; };
const devCard = (name, params, vals, sub = 'EFFECT') => `<div class="m-card m-dev"><h4>${name}</h4><small class="sub">${sub}</small>
 <div class="ks">${params.map((p, i) => `<div>${knob(vals[i], 42)}<span>${p}</span></div>`).join('')}</div>
 <div class="bar"><span>${glyph('power', RED, 1.7, 2.3)}${glyph('window', INK, 1.7, 2.3)}</span><span>${glyph('arrowL', INK, 1.7, 2.3)}${glyph('arrowR', INK, 1.7, 2.3)}</span><span>${glyph('dup', INK, 1.7, 2.3)}${glyph('cross', RED, 1.7, 2.3)}</span></div></div>`;
M.devices = () => `<div class="m-devs"><div class="ttl">DEVICES&nbsp;&nbsp;‹&nbsp;TRACK 1&nbsp;›</div><div class="cards">
 ${devCard('SW SQUEEZE', ['THRESHOLD', 'RATIO', 'ATTACK', 'RELEASE', 'MAKEUP', 'MIX'], [.45, .3, .2, .5, .35, .8])}
 ${devCard('SW SPACE', ['SIZE', 'DECAY', 'PRE-DELAY', 'LOW CUT', 'DAMP', 'MIX'], [.7, .55, .2, .3, .5, .35])}
 <div class="add">${pill('+ ADD', { w: 96 })}${pill('PASTE', { w: 96 })}</div></div></div>`;
M.grid = () => { const tr = ['TRACK 1', 'TRACK 2', 'TRACK 3', 'TRACK 4', 'MIDI 1'];
  const strip = (i) => `<div class="m-card strip"><div class="row">${pill('STOP', { style: 'flex:1' })}${pill('o', { w: 30, red: i === 2, on: i === 2 })}</div>
   <div class="row" style="justify-content:space-around">${knob([.6, .2, 0, .7, .3][i], 34, true)}${knob([.2, .4, 0, 0, .5][i], 34, true)}</div><div class="ab"><span>A</span><span>B</span></div>
   ${i === 0 ? pill('SW SQUEEZE', { w: 110 }) : ''}${i === 4 ? pill('SW TIDE', { w: 110 }) : ''}${pill('+ FX', { w: 110 })}<div class="fader"><i></i><b style="top:${[30, 40, 25, 45, 35][i]}%"></b></div><div class="row">${pill('M', { style: 'flex:1' })}${pill('S', { style: 'flex:1' })}</div></div>`;
  const slot = (c, r) => { const on = (c === 0 && r < 3) || (c === 1 && r === 0) || (c === 3 && r < 2) || (c === 4 && r === 1); const play = c === 0 && r === 0;
    return `<div class="slot ${on ? 'full' : ''} ${play ? 'play' : ''}">${on ? `<small>${['KICK', 'BASS', 'HATS', 'PAD', 'MIDI'][c]} ${r + 1}</small>` : '·'}</div>`; };
  const ret = (n, red) => `<div class="m-card retstrip"><h4 style="color:${red ? RED : INK}">${n}</h4>${pill(n.includes('REVERB') ? 'REVERB' : n.includes('DELAY') ? 'DELAY' : 'WALL', { w: 96 })}${pill('+ FX', { w: 96 })}<div class="fader tall"><i></i><b style="top:30%"></b></div><small>0.0 dB</small><div class="row">${pill('M', { style: 'flex:1' })}${pill('S', { style: 'flex:1' })}</div></div>`;
  return `<div class="m-grid"><div class="left"><div class="m-card slots"><div class="cols">${tr.map((t, c) => `<div class="col"><h5>${c + 1} ${t}</h5>${[...Array(8)].map((_, r) => slot(c, r)).join('')}</div>`).join('')}</div></div>
   <div class="strips">${tr.map((_, i) => strip(i)).join('')}</div></div>
   <div class="scenes"><small>SCENES</small>${[...Array(8)].map((_, i) => pill('&gt; ' + (i + 1), { w: 70 })).join('')}${pill('STOP ALL', { w: 70 })}</div>
   ${ret('A REVERB', 1)}${ret('B DELAY', 1)}${ret('MASTER', 0)}</div>`; };
M.export = () => `<div class="m-card m-exp"><h4>EXPORT&nbsp;&nbsp;‹&nbsp;AUDIO&nbsp;›</h4>
 ${[['FORMAT', 'WAV'], ['BIT DEPTH', '24 bit'], ['SAMPLE RATE', '48000 Hz'], ['MP3 QUALITY', '320 kb/s CBR'], ['RANGE', 'Whole song'], ['NORMALISE', 'Off']].map(([a, b]) => `<div class="er"><small>${a}</small>${combo(b, 220)}</div>`).join('')}
 <div class="er" style="justify-content:flex-start;gap:60px"><span class="sw"><i></i>DITHER</span><span class="sw"><i></i>INCLUDE MASTER FX</span></div>
 <div style="display:flex;justify-content:flex-end;margin-top:20px">${pill('EXPORT', { w: 120 })}</div></div>`;
M.splash = () => `<div class="m-card m-splash"><div class="top"><span class="big">0.8</span><span class="beta">BETA</span></div>
 <div class="ld">${[...Array(26)].map((_, i) => `<i style="background:${i < 17 ? INK : i === 17 ? RED : SUB}"></i>`).join('')}</div>
 <div class="bt"><img src="img/icon.svg"><div><b>SOUNDWAVE</b><small>CARGANDO · BUILD NOV 24 2026</small></div></div></div>`;
M.tour = () => /* hole shows the real library */ `<div class="m-tour"><div class="dim"></div><div class="hole"><div style="transform:scale(.88);transform-origin:0 0;width:266px">${M.libSamples()}</div></div><div class="m-card tc"><div class="prog">${[...Array(10)].map((_, i) => `<i style="background:${i === 1 ? RED : i < 1 ? INK : LINE}"></i>`).join('')}<small>02 / 10</small></div>
 <div class="tb">${glyph('browser', RED, 5.2, 7.4)}<div><h4>LIBRERÍA</h4><p>Añade tus carpetas de samples con +. Haz clic en un sonido para escucharlo y arrástralo a una pista.</p></div></div>
 <div class="tbtn">${pill('SALTAR', { w: 80 })}<span style="flex:1"></span>${pill('ATRÁS', { w: 80 })}${pill('SIGUIENTE', { w: 110 })}</div></div></div>`;
M.overview = () => `<div class="m-full">${M.topbar()}<div class="body"><div class="side">${M.libPlugins()}</div><div class="main">${M.tracks({ ruler: 1 })}<div class="bottom">${M.piano()}${M.devices()}</div></div></div></div>`;

// render every placeholder and scale it to fit its card (stays vector in the PDF)
document.querySelectorAll('[data-m]').forEach(el => {
  const [name, arg] = el.dataset.m.split(':');
  el.innerHTML = `<div class="m-scale">${M[name](arg)}</div>`;
});
window.fitMocks = () => document.querySelectorAll('[data-m]').forEach(el => {
  const inner = el.firstElementChild, w = inner.scrollWidth, h = inner.scrollHeight;
  const s = Math.min(el.clientWidth / w, el.clientHeight / h);
  inner.style.transform = `scale(${s})`;
  inner.style.marginLeft = (el.clientWidth - w * s) / 2 + 'px';
  inner.style.marginTop = (el.clientHeight - h * s) / 2 + 'px';
});
document.fonts.ready.then(fitMocks);
