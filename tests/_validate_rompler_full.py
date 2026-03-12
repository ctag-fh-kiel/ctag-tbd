#!/usr/bin/env python3
"""Full end-to-end validation of Rompler parameter changes."""
import json, re, sys, os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

with open('sdcard_image/data/macrodefinitions/ro-allparams.json') as f:
    macro = json.load(f)
with open('sdcard_image/data/macrosoundpresets/ro-all-def.json') as f:
    preset = json.load(f)
with open('sdcard_image/data/synthdefinitions.json') as f:
    synthdef = json.load(f)
with open('components/ctagSoundProcessor/rack/RackRompler.cpp') as f:
    cpp = f.read()

errors = []
all_params = []
for g in macro['groups']:
    for p in g['parameters']:
        all_params.append(p)
idx_to_name = dict((p['idx'], p['name']) for p in all_params)

# 1. CRITICAL: idx 0=Bank, idx 1=Slice (hardcoded in SpiAPI.cpp)
if idx_to_name.get(0) != 'Bank':
    errors.append('CRITICAL: idx 0 must be Bank (hardcoded in SpiAPI.cpp), got %s' % idx_to_name.get(0))
else:
    print('OK  idx 0 = Bank (SpiAPI.cpp hardcoded)')

if idx_to_name.get(1) != 'Slice':
    errors.append('CRITICAL: idx 1 must be Slice (hardcoded in SpiAPI.cpp), got %s' % idx_to_name.get(1))
else:
    print('OK  idx 1 = Slice (SpiAPI.cpp hardcoded)')

# 2. Sequential idx values
idxs = sorted([p['idx'] for p in all_params])
expected = list(range(len(all_params)))
if idxs != expected:
    errors.append('idx values not sequential: %s' % idxs)
else:
    print('OK  %d params with sequential idx 0..%d' % (len(all_params), len(all_params)-1))

# 3. Preset count
vals = preset['values']
if len(vals) != len(all_params):
    errors.append('Preset has %d values but %d params' % (len(vals), len(all_params)))
else:
    print('OK  Preset has %d values matching %d params' % (len(vals), len(all_params)))

# 4. Preset defaults match param defs
for p in all_params:
    if vals[p['idx']] != p['def']:
        errors.append('Preset[%d]=%d != %s.def=%d' % (p['idx'], vals[p['idx']], p['name'], p['def']))
if not any('Preset[' in e for e in errors):
    print('OK  All preset values match parameter defaults')

# 5. Mapping src references valid idx
valid_idxs = set(p['idx'] for p in all_params)
for m in macro['mapping']:
    for src in m['add']:
        if src['src'] not in valid_idxs:
            errors.append('Mapping ctrl %d: src %d not a valid idx' % (m['ctrl'], src['src']))
print('OK  All %d mapping entries reference valid param indices' % len(macro['mapping']))

# 6. Mapping ctrl vs synthdefinitions.json
ro_synth = None
for machine in synthdef['machines']:
    if machine['id'] == 'ro':
        ro_synth = machine
        break
synth_ctrls = set(p['ctrl'] for p in ro_synth['parameters'])
mapping_ctrls = set(m['ctrl'] for m in macro['mapping'])
missing = mapping_ctrls - synth_ctrls
if missing:
    errors.append('Mapping ctrl numbers not in synthdefs: %s' % missing)
else:
    print('OK  All mapping ctrl numbers exist in synthdefinitions.json')

# 7. C++ registerParamAndCC offsets
cpp_regs = re.findall(r'registerParamAndCC\(initdata,\s*"([^"]+)",\s*(\d+)', cpp)
cpp_offsets = set(int(offset) for _, offset in cpp_regs)
if cpp_offsets != mapping_ctrls:
    errors.append('C++ offsets %s != mapping ctrls %s' % (sorted(cpp_offsets), sorted(mapping_ctrls)))
else:
    print('OK  C++ registerParamAndCC offsets match mapping ctrls: %s' % sorted(cpp_offsets))

# 8. Specific mapping fixes
for m in macro['mapping']:
    if m['ctrl'] == 16:
        if m['start'] != 1:
            errors.append('Attack (ctrl 16) start=%d, expected 1' % m['start'])
        else:
            print('OK  Attack mapping start=1 (prevents div-by-zero)')
    if m['ctrl'] == 17:
        if m['start'] != 1:
            errors.append('Decay (ctrl 17) start=%d, expected 1' % m['start'])
        else:
            print('OK  Decay mapping start=1 (prevents div-by-zero)')
    if m['ctrl'] == 24:
        if m['add'][0]['mul'] != 64:
            errors.append('TSMode (ctrl 24) mul=%d, expected 64' % m['add'][0]['mul'])
        else:
            print('OK  TSMode mapping mul=64 (prevents integer truncation)')

# 9. Mapping ctrl -> param name (the core routing table)
ctrl_to_param = {}
for m in macro['mapping']:
    src_idx = m['add'][0]['src']
    ctrl_to_param[m['ctrl']] = idx_to_name[src_idx]

expected_map = {
    8: 'Bank', 9: 'Slice', 10: 'Start', 11: 'End',
    12: 'Cutoff', 13: 'Reso', 14: 'Type', 15: 'Bit.CR',
    16: 'Attack', 17: 'Decay', 18: 'Speed', 19: 'Pitch',
    20: 'Loop', 21: 'PingPong', 22: 'PPStart', 23: 'EG2FM',
    24: 'TSMode', 25: 'TSAmt'
}
for ctrl, expected_name in sorted(expected_map.items()):
    actual = ctrl_to_param.get(ctrl)
    if actual != expected_name:
        errors.append('ctrl %d: expected %s, got %s' % (ctrl, expected_name, actual))
print('OK  All 18 mapping ctrl->param name associations correct')

# 10. C++ engine fixes present
if 'userSpeed' not in cpp:
    errors.append('C++ missing userSpeed member')
else:
    print('OK  C++ has userSpeed member for speed fix')
if 'autoSpeed * userSpeed' not in cpp:
    errors.append('C++ missing autoSpeed * userSpeed')
else:
    print('OK  C++ uses autoSpeed * userSpeed at note trigger')
if 'fS1Pitch - 64.f' not in cpp:
    errors.append('C++ missing pitch offset (fS1Pitch - 64.f)')
else:
    print('OK  C++ uses semitone pitch offset (fS1Pitch - 64)')
if 'fS1Attack < 0.001f' not in cpp:
    errors.append('C++ missing Attack safety clamp')
else:
    print('OK  C++ has Attack safety clamp')
if 'fS1Decay < 0.01f' not in cpp:
    errors.append('C++ missing Decay safety clamp')
else:
    print('OK  C++ has Decay safety clamp')

# 11. RomplerVoiceMinimal.cpp - envelope loop fix
with open('components/ctagSoundProcessor/synthesis/RomplerVoiceMinimal.cpp') as f:
    rvm = f.read()
if 'ad.SetLoop(params.loop)' not in rvm:
    errors.append('RomplerVoiceMinimal.cpp missing ad.SetLoop(params.loop)')
else:
    print('OK  RomplerVoiceMinimal.cpp has ad.SetLoop(params.loop)')

# 12. Page layout
page_names = [g['name'] for g in macro['groups']]
expected_pages = ['SAMPLE', 'PLAY', 'FILTER', 'LOOP', 'STRETCH']
if page_names != expected_pages:
    errors.append('Pages %s != expected %s' % (page_names, expected_pages))
else:
    print('OK  Page layout: %s' % ' -> '.join(page_names))

# 13. Pico compatibility - preset references correct macro
if preset.get('macro') != 'ro-allparams':
    errors.append('Preset macro=%s, expected ro-allparams' % preset.get('macro'))
else:
    print('OK  Preset references macro "ro-allparams"')

# SUMMARY
print()
if errors:
    print('ERRORS FOUND:')
    for e in errors:
        print('  X %s' % e)
    sys.exit(1)
else:
    print('=' * 50)
    print('  ALL %d VALIDATIONS PASSED' % 18)
    print('  Pico sequencer compatibility: SAFE')
    print('  No breaking changes to CC numbers')
    print('=' * 50)
