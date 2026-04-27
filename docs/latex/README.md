# LaTeX Docs

This folder contains a LaTeX version of the robotic arm hardware documentation.

## Files

- `robotic_arm_electrical_schema.tex`: main LaTeX source with a TikZ block
  diagram and a Circuitikz electrical schematic

## Build

From the repository root:

```bash
cd docs/latex
pdflatex robotic_arm_electrical_schema.tex
pdflatex robotic_arm_electrical_schema.tex
```

The second pass resolves figure placement and references cleanly.

## Notes

- The schematic matches the current repository documentation in `README.md`,
  `docs/hardware.md`, and `diagram.json`.
- The servo bank is intentionally shown on an external `5V-6V` supply.
- All grounds must be shared between the Arduino, the servos, the supply, and
  the Wii Nunchuck.
