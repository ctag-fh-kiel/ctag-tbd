Plugins
=======

The TBD-16 ships with **50+ audio plugins** — synthesizers, effects, drum
machines, granular processors, and more. Every plugin can be loaded, swapped,
and tweaked in real time from the web interface.

.. tip::
   Use the search box below to filter by name, type, or description.
   Click any plugin name to read the full documentation.

.. raw:: html

   <div class="plugin-toolbar">
     <input type="text" id="pluginSearch" placeholder="Search plugins…" />
     <div class="plugin-filters">
       <button class="filter-btn active" data-filter="all">All</button>
       <button class="filter-btn" data-filter="Synth Voice">Synth</button>
       <button class="filter-btn" data-filter="FX">FX</button>
       <button class="filter-btn" data-filter="Oscillator">Oscillator</button>
       <button class="filter-btn" data-filter="Filter">Filter</button>
       <button class="filter-btn" data-filter="Drums">Drums</button>
       <button class="filter-btn" data-filter="Noise">Noise</button>
       <button class="filter-btn" data-filter="Sampler">Sampler</button>
       <button class="filter-btn" data-filter="Modulation">Modulation</button>
     </div>
   </div>

   <table class="plugin-table" id="pluginTable">
     <thead>
       <tr>
         <th class="sortable" data-sort="name">Plugin ↕</th>
         <th class="sortable" data-sort="channels">Ch ↕</th>
         <th class="sortable" data-sort="type">Type ↕</th>
         <th>Description</th>
         <th class="sortable" data-sort="midi">MIDI API ↕</th>
       </tr>
     </thead>
     <tbody>
       <tr data-type="FX">
         <td><a href="10_antique.html">Antique</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Audio artefacts from record players and tapes</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Oscillator">
         <td><a href="11_apcpp.html">APCpp</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-oscillator">Oscillator</span></td>
         <td>Atari Punk Console plus plus</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Drums">
         <td><a href="12_bbeats.html">BBeats</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-drums">Drums</span></td>
         <td>Two ByteBeat generators that can be mixed</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Synth Voice">
         <td><a href="13_bjorklung.html">Bjorklund</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-synth">Synth Voice</span></td>
         <td>Synth voice with oscillator, filters, envelopes and euclidean sequencer</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="14_cdalay.html">CDelay</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Modified port of Tesselode's Cocoa Delay</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="15_claude.html">Claude</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Port of Mutable Instruments Clouds</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="16_Crushendo.html">Crushendo</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>A simple bit crusher / sample rate reducer</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="17_cstrip.html">CStrip</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Channel Strip / Mixing Console, ported from AirWindows</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Noise">
         <td><a href="18_dust.html">Dust</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-noise">Noise</span></td>
         <td>Makes crackling noises</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="19_echorus.html">EChorus</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Modified AirWindows Ensemble Chorus effect</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="20_every_trim.html">Every Trim</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Modified AirWindows Every Trim effect</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="21_fbdlyline.html">FBDlyLine</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>A simple delay line, more a coding example</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Synth Voice">
         <td><a href="22_formantor.html">Formantor</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-synth">Synth Voice</span></td>
         <td>Phase Distortion oscillator, Squarewave oscillator with optional pulse width modulation, a vowel-filter, a resonator, a tremolo and a volume EG</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Synth Voice">
         <td><a href="23_freakwaves.html">Freakwaves</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-synth">Synth Voice</span></td>
         <td>Synth based on two wavetable-oscillators with different CV/Gate inputs</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="24_freeverb.html">Freeverb</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>The original Freeverb by Jezar at Dreampoint</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="25_gverb.html">G-Verb</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Feedback Delay Network reverb by Juhana Sadeharju Kouhia</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Drums">
         <td><a href="26_hihat1.html">HiHat1</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-drums">Drums</span></td>
         <td>Simple hihat model based on noise shaping</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Synth Voice">
         <td><a href="27_karpuskl.html">Karpuskl</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-synth">Synth Voice</span></td>
         <td>Plucked String Synth, Fuzz/Sustainer, Ringmodulator, Volume-EG and CV Pitch Tuner</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Oscillator">
         <td><a href="28_macosc.html">MacOsc</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-oscillator">Oscillator</span></td>
         <td>Port of Mutable Instruments Braids</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="29_michorus.html">MiChorus</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Chorus effect from Mutable Instruments modules</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="30_midifu.html">MiDifu</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Diffuser effect from Mutable Instruments modules</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="31_miensemble.html">MiEnsemble</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Ensemble effect from Mutable Instruments modules</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="32_mipshift.html">MiPShift</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Pitch shift effect</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="33_miverb.html">MiVerb</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Reverb from Mutable Instruments, reduced implementation of Datorro Paper</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="34_miverb2.html">MiVerb2</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Reverb effect offering more controls</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Filter">
         <td><a href="35_moogfilters.html">MoogFilters</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-filter">Filter</span></td>
         <td>Various analog Moog Ladder Filter models</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Filter">
         <td><a href="36_msxxnoise.html">MSxxNoise</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-filter">Filter</span></td>
         <td>Filter design modeled after the Sallen Key Filter</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Noise">
         <td><a href="37_pink_noise.html">Pink Noise</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-noise">Noise</span></td>
         <td>Simple pink noise, code example</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="38_plate_reverb.html">Plate Reverb</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Freeverb 3 implementation of Datorro algorithm</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Oscillator">
         <td><a href="39_polypad.html">PolyPad</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-oscillator">Oscillator</span></td>
         <td>Dub-like minor pads, 24 voice polyphony at the tip of a finger press</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="40_progenreverb.html">ProgenReverb</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Extremely lush Griesinger Lexicon reverb</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Sampler">
         <td><a href="41_recnplay.html">RecNPlay</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-sampler">Sampler</span></td>
         <td>Synth, step sequencer + bitcrusher</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Synth Voice">
         <td><a href="42_retroactor.html">Retroactor</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-synth">Synth Voice</span></td>
         <td>A simple yet flexible to control feedback / drone-noise generator</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Sampler">
         <td><a href="43_rompler.html">Rompler</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-sampler">Sampler</span></td>
         <td>A mono sample playback device</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="44_sdelay.html">SDelay</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Simple stereo / ping pong delay</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Modulation">
         <td><a href="45_simple_vca.html">Simple VCA</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-modulation">Modulation</span></td>
         <td>Simple VCA code example</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Oscillator">
         <td><a href="46_sine_source.html">Sine Source</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-oscillator">Oscillator</span></td>
         <td>Sine oscillator with envelopes, good for bass drums and other stuff</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="47_spacefx.html">SpaceFX</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Self-oscillating filter with whitenoise, modulation, delay and reverb</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Synth Voice">
         <td><a href="48_subbotnik.html">Subbotnik</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-synth">Synth Voice</span></td>
         <td>Synth voice inspired by West Coast synthesis à la Morton Subotnick</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Synth Voice">
         <td><a href="49_subsynth.html">SubSynth</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-synth">Synth Voice</span></td>
         <td>Single sub synth</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Synth Voice">
         <td><a href="50_talkbox.html">Talkbox</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-synth">Synth Voice</span></td>
         <td>A vocoder instrument with an internal synth as the carrier signal</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="FX">
         <td><a href="51_tape_delay.html">Tape Delay</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-fx">FX</span></td>
         <td>Port of AirWindows Tape Delay effect</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Drums">
         <td><a href="52_tbdaits.html">TBDaits</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-drums">Drums</span></td>
         <td>Port of Mutable Instruments Plaits</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Oscillator">
         <td><a href="53_tbdeeps.html">TBDeeps</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-oscillator">Oscillator</span></td>
         <td>Port of Sheep, a wavetable synthesizer</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Oscillator">
         <td><a href="54_tbdings.html">TBDings</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-oscillator">Oscillator</span></td>
         <td>Port of Mutable Instruments Rings</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Synth Voice">
         <td><a href="55_vctrsnt.html">VctrSnt</a></td>
         <td>Stereo</td>
         <td><span class="type-badge type-synth">Synth Voice</span></td>
         <td>Vector Synthesizer inspired by the Sequential Circuits Prophet VS</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Utility">
         <td><a href="56_void.html">Void</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-utility">Utility</span></td>
         <td>Does nothing except saving CPU cycles. Assign to a second channel to free processing power</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
       <tr data-type="Oscillator">
         <td><a href="57_wtosc.html">WTOsc</a></td>
         <td>Mono</td>
         <td><span class="type-badge type-oscillator">Oscillator</span></td>
         <td>User defined wave-table oscillator using 1MiB of wavetable ROM</td>
         <td><span class="midi-status midi-planned" title="Planned">—</span></td>
       </tr>
     </tbody>
   </table>

   <p class="plugin-count"><span id="visibleCount">48</span> of 48 plugins shown</p>

   <script>
   document.addEventListener('DOMContentLoaded', function() {
     const search = document.getElementById('pluginSearch');
     const table = document.getElementById('pluginTable');
     const rows = table.querySelectorAll('tbody tr');
     const filterBtns = document.querySelectorAll('.filter-btn');
     const countEl = document.getElementById('visibleCount');
     let activeFilter = 'all';

     function filterRows() {
       const q = search.value.toLowerCase();
       let visible = 0;
       rows.forEach(function(row) {
         const text = row.textContent.toLowerCase();
         const type = row.getAttribute('data-type');
         const matchSearch = !q || text.indexOf(q) !== -1;
         const matchFilter = activeFilter === 'all' || type === activeFilter;
         if (matchSearch && matchFilter) {
           row.style.display = '';
           visible++;
         } else {
           row.style.display = 'none';
         }
       });
       countEl.textContent = visible;
     }

     search.addEventListener('input', filterRows);

     filterBtns.forEach(function(btn) {
       btn.addEventListener('click', function() {
         filterBtns.forEach(function(b) { b.classList.remove('active'); });
         btn.classList.add('active');
         activeFilter = btn.getAttribute('data-filter');
         filterRows();
       });
     });

     /* Column sorting */
     var sortDir = {};
     table.querySelectorAll('th.sortable').forEach(function(th) {
       th.style.cursor = 'pointer';
       th.addEventListener('click', function() {
         var col = th.getAttribute('data-sort');
         var idx = Array.from(th.parentNode.children).indexOf(th);
         var dir = sortDir[col] === 'asc' ? 'desc' : 'asc';
         sortDir[col] = dir;
         var arr = Array.from(rows);
         arr.sort(function(a, b) {
           var at = a.children[idx].textContent.trim().toLowerCase();
           var bt = b.children[idx].textContent.trim().toLowerCase();
           if (at < bt) return dir === 'asc' ? -1 : 1;
           if (at > bt) return dir === 'asc' ? 1 : -1;
           return 0;
         });
         var tbody = table.querySelector('tbody');
         arr.forEach(function(row) { tbody.appendChild(row); });
       });
     });
   });
   </script>


.. note::
   **MIDI API Status** — The MIDI API column shows whether a plugin supports
   remote control from the RP2350 microcontroller (used in the PicoSeqRack
   and future controller firmwares). The PicoSeqRack is currently in
   development as the reference implementation. Once the MIDI API is finalized,
   support will be added to all legacy plugins.

   Legend: ✅ Supported · 🔧 In Progress · — Planned
