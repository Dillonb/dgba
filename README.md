# gba
It's a gba emulator. Things are _mostly working_.

## Notable stuff still TODO:

- [ ] Optimization
- [ ] GB sound (non-DMA/PCM sound)
- [ ] Mosaic effect
- [ ] Better CPU/memory cycle accuracy
- [ ] EEPROM backups
- [ ] Customizable controls

## Building
- Install SDL and cmake, and optionally Capstone for disassembly output.
- cd build && ./build.sh
- Build tested on Linux and MacOS. Windows unsupported at this time.

## Running
- ./gba rom.gba

### Flags
- Use -s to skip the bios
- Use -b bios_file.bin to load an alternate bios
- Use -S X to set the scaling factor for the screen to a provided integer. Default 4.
- Use -v to enable verbose logging. Repeat up to 3 times.
- Use -d for debug mode. Currently does nothing.

### Controls
- D-Pad: WASD / Arrow Keys
- A: J or Z
- B: K or X
- L: Q
- R: E
- Start: Enter
- Select: Right Shift
- Quick-save state: 1,2,3,4,5,6,7,8,9,0 - 10 save slots
- Quick-load state: ctrl+(1,2,3,4,5,6,7,8,9,10) - Same 10 save slots
