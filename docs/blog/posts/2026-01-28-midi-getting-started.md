---
blogpost: true
date: 2026-01-28
author: dadamachines
category: Tutorial
tags: midi, getting-started
---

# Getting Started with MIDI on TBD-16

The TBD-16 features a standard 5-pin DIN MIDI input and thru port,
making it easy to integrate into any hardware setup. This post walks you
through the basics.

## Connecting MIDI

1. Connect a MIDI cable from your controller's MIDI OUT to the TBD-16's
   MIDI IN port.
2. Open the web UI and navigate to the plugin you want to control.
3. Use the MIDI mapping section to assign CCs to parameters.

## MIDI mapping tips

- **Use CC learn**: Click a parameter knob, then move a physical knob on
  your controller. The TBD-16 will auto-detect the CC number.
- **Channel filtering**: Each plugin slot can listen on a specific MIDI
  channel, so you can control two plugins independently.
- **Note triggers**: Some plugins (like BBeats and Bjorklund) respond to
  MIDI note-on messages for triggering events.

## What about MIDI over USB?

The TBD-16 currently supports traditional 5-pin DIN MIDI. USB MIDI support
is on the roadmap for a future firmware update.

For the full reference, see the [hardware documentation](../../hardware/index).
