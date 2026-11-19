# J-106 – Juno-106-style polyphonic synth (VST3)

6-voice polyphonic synth modelled on the Roland Juno-106 voice architecture, plus built-in
saturation, compressor and reverb, pitch wheel / mod wheel, and 41 factory-style presets.
Windows (x64) and macOS (universal: Apple Silicon + Intel) VST3.

## Getting the plugin without compiling anything
1. Make a free account at github.com and create a new **private** repository.
2. Upload everything in this folder (drag & drop onto the repo page). The `.github` folder is
   hidden on macOS (Finder: Cmd+Shift+.). If it won't upload: in GitHub choose *Add file > Create new file*,
   type `.github/workflows/build.yml` as the name, and paste the contents of `build.yml.copy.txt`.
3. Open the **Actions** tab > **Build J-106** > **Run workflow**. It takes roughly 10-20 minutes.
4. Open the finished run and download **J-106-Windows** and/or **J-106-macOS** from *Artifacts*.
5. Install:
   * Windows: run `J-106-Setup-Windows.exe` (SmartScreen: *More info > Run anyway*; the installer is unsigned).
   * macOS: open `J-106-macOS.pkg` (right-click > Open the first time). If your DAW doesn't list it, run in Terminal:
     `sudo xattr -cr /Library/Audio/Plug-Ins/VST3/J-106.vst3`
6. Rescan plugins in your DAW. Load it as an instrument track.

If the build fails, open the failed step in the Actions run and send me the log text.

## Layout
* `Source/Engine.h`  – all DSP (no JUCE dependency, unit-tested in `tests/`)
* `Source/Presets.h` – patches (edit freely; values are 0..1 slider positions)
* `Source/Plugin*.{h,cpp}` – JUCE wrapper and UI
