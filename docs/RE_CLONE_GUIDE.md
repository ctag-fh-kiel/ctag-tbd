# Re-Clone Guide — Git History Rewrite (Phase 3b)

> **Date:** 2026-07-13  
> **Affects:** All clones and forks of `dadamachines/ctag-tbd`

## What happened?

We rewrote the git history using `git-filter-repo` to remove dead binary
blobs that had accumulated over time — old firmware builds, SD card zips,
KiCad PCB files, and other artifacts that were deleted from the working
tree but still bloated `.git/`.

**No source code was changed.** Only unreachable binary blobs were stripped
from history. Every commit, branch, and tag still exists — only their
SHA hashes have changed.

### Results

| Metric | Before | After |
|--------|--------|-------|
| `.git/objects/` | ~300 MB | ~50 MB |
| Total `.git/` | ~360 MB | ~125 MB |
| Working tree | unchanged | unchanged |

## What you need to do

### If you have a direct clone (no fork)

```bash
# 1. Back up any local branches with unpushed work
git stash  # if you have uncommitted changes

# 2. Delete the old clone and re-clone
cd ..
rm -rf ctag-tbd
git clone --recursive https://github.com/dadamachines/ctag-tbd.git
cd ctag-tbd
git checkout dada-tbd-master
```

If you had local branches with unpushed work, apply your patches onto the
new history with `git cherry-pick` or `git am`.

### If you have a GitHub fork

```bash
# 1. In your fork clone, add upstream if not already set
git remote add upstream https://github.com/dadamachines/ctag-tbd.git

# 2. Fetch the rewritten history
git fetch upstream

# 3. Reset your main branch to the rewritten upstream
git checkout dada-tbd-master
git reset --hard upstream/dada-tbd-master
git push origin dada-tbd-master --force

# 4. Update submodules
git submodule update --init --recursive
```

If you have open PRs, rebase them onto the new `dada-tbd-master`:

```bash
git checkout my-feature-branch
git rebase --onto upstream/dada-tbd-master <old-base-commit> my-feature-branch
git push origin my-feature-branch --force
```

### Alternative: fresh fork

If the rebase feels complicated, the simplest approach is to delete your
fork on GitHub, re-fork from `dadamachines/ctag-tbd`, and re-clone:

```bash
git clone --recursive https://github.com/YOUR_USER/ctag-tbd.git
cd ctag-tbd
git remote add upstream https://github.com/dadamachines/ctag-tbd.git
```

## What was removed?

Only dead binary files that no longer exist in the working tree:

- `bin/ctag-tbd.bin`, `bin/storage.bin` (old firmware builds)
- `docs/_static/firmware/p4/*.bin` (old P4 firmware snapshots)
- `docs/_static/firmware/pico/*.uf2` (old Pico firmware snapshots)
- `docs/_static/sdcard_image/**/*.zip` (old SD card archives)
- `docs/downloads/**/*.bin` (old download binaries)
- `docs/get_started/_static/firmware/**` (old get-started artifacts)
- `docs/flash/_static/sdcard_image/*.zip` (old flash page artifacts)
- `hw/pcb/`, `hardware/mk1/pcb/` (old KiCad board files)

## What was NOT changed?

- All source code (C++, Python, JavaScript, HTML, CSS, JSON)
- `sample_rom/` (WAV files + sample-rom.tbd)
- `bin/tusb_msc.bin`, `bin/usb_uac.bin`
- `simulator/*.wav`
- All documentation text
- All images (PNG, JPG)
- Submodules (ableton_link, MoogLadders, eurorack, etc.)
- Build system (CMakeLists.txt, sdkconfig, Kconfig)

## FAQ

**Q: Do I need to install Git LFS?**  
A: No. We decided against Git LFS to keep the contributor workflow simple.
No special tools are needed beyond `git` itself.

**Q: Will my ESP-IDF build still work?**  
A: Yes. The build was verified after the rewrite. Run `idf.py build` as
usual.

**Q: I see "fatal: refusing to merge unrelated histories" — what do I do?**  
A: Your local clone still has the old history. Follow the re-clone steps
above instead of trying to pull/merge.

**Q: My CI pipeline broke — what changed?**  
A: If your CI caches git history, clear the cache and re-clone. The
workflow files themselves are unchanged.
