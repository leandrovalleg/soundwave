# SoundWave

DAW minimalista y colección de plugins VST3 propios. Código abierto bajo GPLv3.

## Descargar para Windows
GitHub → pestaña **Actions** → último "Windows build" → **Artifacts** → `SoundWave-windows` (zip).
- `SoundWave/SoundWave.exe` — la app (MP3 incluido, no hay que instalar nada).
- `VST3/*.vst3` — copia estas carpetas a `C:\Program Files\Common Files\VST3`.

## Contenido
- `app/` — la app: TIMELINE y GRID (Tab), navegador de librerías, plugins VST3, exportar WAV/MP3, ajustes de audio.
- `plugins/` — Prism (EQ), Squeeze (compresor), Flux (EQ dinámico), Glow (válvula), Wall (limitador), Gate,
  Space (reverb), Room (convolución), Echo (delay), Swirl (chorus), Jet (flanger), Orbit (phaser),
  Sway (auto-pan), Crush (bitcrusher), Tune (afinación), Tide (sinte wavetable).
- `plugins/SWDesign.h` — sistema de diseño compartido. `docs/app-spec.md` — especificación.

## Compilar
```
git clone --recursive https://github.com/leandrovalleg/soundwave
cmake -B build && cmake --build build --config Release
cmake -S app -B app/build && cmake --build app/build --config Release
```

## Licencias
SoundWave: GPLv3. JUCE (AGPLv3), Tracktion Engine (GPLv3), Doto y Space Mono (OFL), LAME (LGPL).
