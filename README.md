# The Phantom Piano Project
The project focuses on developing an electronic player piano that communicates via the MIDI protocol. It is designed to support both dynamic expression (velocity tracking) and the sustain pedal. Our ultimate goal is to enable users to record their live performances and save them in MIDI format, allowing them to be accurately replayed on the instrument later. 

Additionally, we aim to implement a silent system feature that allows seamless transition from acoustic mode to electronic mode. This mechanism functions by catching/stopping the hammer action before they strike the strings, muting the acoustic sound, and routing the audio digitally. By selecting any virtual instrument patch, this system not only expands the creative capabilities of the piano but also enables silent practice using headphones.

## Visuals

![Project Render](enclosure/images/render.png)

## Setting Up

Steps for setting up the repository locally

*If project includes submodules*
```bash
git clone --recurse-submodules https://github.com/KN-Hardwire/[Project-Repo]
```

## Project structure

### Software

Software/firmware files and building instructions are located in `software/` directory. 

### Hardware

Hardware files are located in `hardware/` directory. Open the `.kicad_pro` file in KiCad.

## Contributing

For contributing guide see ![CONTRIBUTING.md](CONTRIBUTING.md).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
