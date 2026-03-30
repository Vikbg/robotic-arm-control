# Notes for the Project

## Arduino CLI Setup

```bash
arduino-cli config init
arduino-cli core update-index
arduino-cli core install arduino:avr
```

If you keep a local copy in `./bin/arduino-cli`, replace `arduino-cli` with that path.

## Useful Commands

List connected boards:

```bash
arduino-cli board list
```

List all supported boards:

```bash
arduino-cli board listall
```

Compile sketch for Arduino Uno:

```bash
arduino-cli compile --fqbn arduino:avr:uno arduino/robotic_arm
```

Upload to Arduino Uno:

```bash
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno arduino/robotic_arm
```

Open serial monitor:

```bash
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```
