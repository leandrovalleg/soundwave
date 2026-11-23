# SoundWave — especificación de la app (DAW)

Estética: sistema de diseño SoundWave (plugins/SWDesign.h): gris #EFEFEF con rejilla de puntos,
tarjetas blancas redondeadas, negro #111, un único acento rojo, Doto + Space Mono.

## Vistas (Tab alterna entre ellas)
- **TIMELINE** (arreglo, horizontal): pistas en filas, regla de compases arriba, clips en el tiempo,
  automatización por pista. Inspector de canal a la izquierda (estilo Logic).
- **GRID** (lanzador de clips, vertical): columnas = pistas, filas = escenas.
  - Clic en celda: lanza el clip, cuantizado al compás (Global Quantize en la barra superior).
  - Botón de escena a la derecha: lanza la fila entera.
  - Botón stop por pista y "stop all".
  - Grabar desde Grid directo al Timeline.
  - Debajo de cada columna: entrada (MIDI/audio from), monitor In/Auto/Off, salida, envíos A/B, fader, mute/solo/arm.
  - Pistas de retorno (A, B, …) y Master a la derecha.
- Panel inferior: editor del clip seleccionado (piano roll / audio) o cadena de dispositivos de la pista.
- Panel izquierdo (oculto/visible): navegador.

## Navegador
- Colecciones (favoritos con colores).
- Categorías: Sonidos, Baterías, Instrumentos, Efectos, Efectos MIDI, Plugins (VST2/VST3), Clips, Samples, Plantillas.
- Lugares: Librería del usuario, Proyecto actual y **carpetas propias** ("Añadir carpeta…").
- Búsqueda, preescucha y arrastrar a pista/celda.

## Plugins
- Preferencias → Plugins: carpetas VST3 por defecto + carpetas propias (añadir/quitar), escanear/re-escanear,
  lista en caché, plugins que fallan quedan marcados y se saltan.

## Barra superior
Tap, BPM, compás, metrónomo, cuantización global, tonalidad/escala del proyecto, posición, play/stop/grabar,
loop, CPU, entrada/salida MIDI.

## Atajos
Tab: Timeline/Grid · Espacio: play/stop · R: grabar · Ctrl+T: pista audio · Ctrl+Shift+T: pista MIDI ·
Ctrl+D: duplicar · B: lápiz · Ctrl+E: dividir clip · Ctrl+L: loop · Ctrl+F: buscar · Ctrl+Alt+B: navegador.

## Exportar (EXPORT)
- Formato: WAV o MP3.
- WAV: 16 / 24 / 32 bit float, dither opcional (16/24 bit).
- MP3: 320 / 256 / 192 / 128 kb/s CBR, VBR V0 / V2 / V4 (vía libmp3lame, LGPL, cargada en tiempo de ejecución;
  en Windows se distribuye libmp3lame.dll junto al .exe).
- Frecuencia: 44.1 / 48 / 88.2 / 96 kHz (MP3 máx. 48 kHz).
- Rango: canción entera o región de loop.
- Normalizar: off, pico 0 dB, pico -1 dB, -14 LUFS (streaming), -9 LUFS.
- Incluir o no los efectos del master.

## Audio (SETTINGS > AUDIO)
- Driver (ASIO / WASAPI / DirectSound en Windows; ALSA/JACK en Linux; CoreAudio en macOS).
- Tarjeta de entrada y salida, canales de entrada/salida activos, frecuencia de muestreo, tamaño de buffer.
- Entrada/salida MIDI.

## Licencia
GPLv3 (compatible con Tracktion Engine GPLv3 y JUCE AGPLv3).
