# Notes for the Project

Quick reference only. For the full workflow, wiring, and troubleshooting, see [docs/development.md](docs/development.md).

## Arduino CLI Setup

```bash
arduino-cli config init
arduino-cli core update-index
arduino-cli core install arduino:avr
```

If you keep a local copy in `./bin/arduino-cli`, replace `arduino-cli` with that path.

## Useful Commands

```bash
arduino-cli board list
arduino-cli board listall
arduino-cli compile --fqbn arduino:avr:uno arduino/robotic_arm
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno arduino/robotic_arm
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```
