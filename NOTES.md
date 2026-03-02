# Notes for the Project

## Arduino CLI Setup

```bash
./bin/arduino-cli config init
./bin/arduino-cli core update-index
./bin/arduino-cli core install arduino:avr
```

If `arduino-cli` is in your PATH, remove `./bin/`.

## Useful Commands

List connected boards:

```bash
./bin/arduino-cli board list
```

List all supported boards:

```bash
./bin/arduino-cli board listall
```

Compile sketch for Arduino Uno:

```bash
./bin/arduino-cli compile --fqbn arduino:avr:uno arduino/robotic_arm
```

Upload to Arduino Uno:

```bash
./bin/arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno arduino/robotic_arm
```

Open serial monitor:

```bash
./bin/arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```
