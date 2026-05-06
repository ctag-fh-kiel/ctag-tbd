# How to evaluate jitter on tbd16

- Set up track 2 fmkick with the following settings, to generate audible clicks:
  - carFreq 100%
  - carDec 0%
  - atkPit 0%
  - swdDev 0%
  - ModFrq 100%
  - ModDec 100%
  - ModFB 0%
  - Index 0
  - Level +3dB
  - Pan hard left (so that you can pan other sequencer tracks hard right in order to test a busy project with many plugins)
- Set compressor mix to 0%
- Fill up track 2 with 16ths steps
- Start play and record a stereo .wav file.

## Evaluating jitter with jitter-evaluation.py

The script `tests/timing/jitter-evaluation.py` analyses the recording and
produces a plain-text report and a vector PDF visualisation.

### Requirements

```bash
pip install numpy scipy matplotlib
```

### Typical usage

The kick is panned hard **left**, so use `--channel left` (the default):

```bash
python3 tests/timing/jitter-evaluation.py my-recording.wav
```

This creates:
- `my-recording-report.txt` — plain-text statistics
- `my-recording-analysis.pdf` — multi-panel vector PDF

### All options

```
python3 jitter-evaluation.py <input.wav> [options]

  --channel {left,right}   Audio channel to analyse.         [default: left]
  --steps-per-beat N       Steps per quarter-note beat.      [default: 4  = 16th-note grid]
  --threshold T            Onset threshold (fraction of peak amplitude, 0–1). [default: 0.10]
  --min-distance-ms D      Minimum gap between detected onsets in ms.  [default: 50]
  --output-prefix PREFIX   Custom prefix for output files.
```

### What the report contains

| Metric | Description |
|--------|-------------|
| **★ Mean BPM** | Average BPM across all step intervals ± std (key result) |
| **★ Mean Jitter** | Average timing deviation from ideal ± std in ms (key result) |
| **Reference BPM** | BPM derived from the median inter-onset interval (IOI) |
| **Min / Max / Median BPM** | BPM distribution boundaries |
| **Min / Max / Median jitter** | Jitter distribution boundaries (ms) |
| **Peak-to-peak jitter** | Worst-case total timing spread (ms) |
| **Filtered (*) metrics** | All of the above recalculated with IQR outliers removed |

### Troubleshooting

| Symptom | Try |
|---------|-----|
| "too few onsets detected" | Lower `--threshold` (e.g. `0.05`) |
| WARNING: channel looks silent | Use `--channel right` |
| Wrong BPM (double/half) | Adjust `--steps-per-beat` or `--min-distance-ms` |

