# Full Arm Validation Draft

This draft is for the first real full-arm validation session.

## Session Goals

- verify the full wiring on the real arm
- confirm stable power under movement
- confirm Nunchuck input reliability
- confirm home reset and demo mode behavior
- check whether the arm can run for `10+` minutes

## Quick Fill Section

- Date:
- Power supply:
- Nunchuck voltage: `3.3V`
- Test duration:
- Result: pass / partial / fail

## Checklist

- [ ] all 6 servos respond
- [ ] Arduino does not reset
- [ ] Nunchuck I2C remains stable
- [ ] `C` and `Z` gripper actions work
- [ ] `C + Z` demo toggle works
- [ ] long `C + Z` home reset works
- [ ] no dangerous collisions observed
- [ ] motion remains stable for 10+ minutes

## Notes

- issues found:
- joints needing recalibration:
- power observations:
- next action:

## Follow-up

For a full archived report, extend this file with `docs/tests/TEST_REPORT_TEMPLATE.md`.
