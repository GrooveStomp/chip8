<a rel="license" href="http://creativecommons.org/licenses/by/4.0/"><img alt="Creative Commons Licence" style="border-width:0" src="https://i.creativecommons.org/l/by/4.0/80x15.png" /></a><br /><span xmlns:dct="http://purl.org/dc/terms/" href="http://purl.org/dc/dcmitype/InteractiveResource" property="dct:title" rel="dct:type">Aaron Oman's CHIP-8 Emulator</span> by <a xmlns:cc="http://creativecommons.org/ns#" href="https://code.groovestomp.com/chip8/" property="cc:attributionName" rel="cc:attributionURL">Aaron Oman</a> is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by/4.0/">Creative Commons Attribution 4.0 International License</a>.

NOTE: This license does not extend to any of the programs in the `games/` directory.
I will update this README appropriately when I have more information about those licenses.

More Details coming soon.

# Dependencies
- sdl2
- OpenGL 3
- GLEW (OpenGL Extension Wrangler)
- libmath
- libpthread
- soundio
- gcc (Built using 8.3.0)
- Doxygen (For building documentation. Tested with 1.8.13)


# Build
```
make
make debug
make release
```

By default the release target is built.
`make release` outputs to `release/` and `make debug` outputs to `debug/`.

# Run
Where `$FILE` is one of the premade games in the `games/` directory.
```
./release/chip8 games/$FILE
```

# Test
All tests are in `test/*_test.c` and each `_test.c` file is expected to have its own `main()`.

When built, each `_test.c` file creates a correspondingly named `test/*_test` executable that can be run at any time.
For convenience, run them all with the `runtests` target.

To build a specific test:
```
make test/opcode_test
```
Then just run it by invoking that file directly.

To build all tests and run them:
```
make test
make runtests
```

# Documentation
Doxygen is used to generate sourcecode documentation.
Use the `docs` make target to generate Doxygen output in the `docs/` directory.
```
make docs
```