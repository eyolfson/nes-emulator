# eyl NES Emulator

A work-in-progress NES emulator written for Wayland, written in C.

## License

All code is licensed under GPLv3. All documentation is licensed under CC BY-SA
4.0.

## Testing

Currently the emulator passes all `nestest` CPU tests. Most PPU tests pass (NMI
timing and Sprite 0 Hit), see `test/ppu` directory.

## TODO

- CPU
  - [ ] Add two ABS X/Y illegal opcodes
  - [ ] AXS instruction ($CB)
  - [ ] Add more automated testing
- PPU
  - [ ] Use proper shift registers
- APU
  - [ ] ALSA
  - [ ] Pulse (optional)
- Mappers
- Controller support
  - [x] PS4 controller
    - [ ] Enumerate input devices
  - [ ] Xbox One controller
  - [ ] Joy-con controller
- Console
  - [ ] Limit to 60 FPS (Wayland doesn't seem to?)

## Resources

- [Nintendo Entertainment System Architecture] [1]
- [6502 Instruction Reference] [2]
- [6502 Undocumented Opcodes] [3]
- [Extra Instructions Of The 65XX Series CPU] [4]
- [Programming with unofficial opcodes] [5]
- [NMOS 6502 Opcodes] [6]

[1]: http://fms.komkon.org/EMUL8/NES.html
     "Nintendo Entertainment System Architecture"
[2]: http://obelisk.me.uk/6502/reference.html
     "6502 Instruction Reference"
[3]: http://www.ataripreservation.org/websites/freddy.offenga/illopc31.txt
     "6502 Undocumented Opcodes"
[4]: http://www.ffd2.com/fridge/docs/6502-NMOS.extra.opcodes
     "Extra Instructions Of The 65XX Series CPU"
[5]: http://wiki.nesdev.com/w/index.php/Programming_with_unofficial_opcodes
     "Programming with unofficial opcodes"
[6]: http://www.6502.org/tutorials/6502opcodes.html
     "NMOS 6502 Opcodes"
