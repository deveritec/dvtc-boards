# Hardware test for dectnrpi_nds board

## Tests
### GPIO loopback (Wiring)

Tests the Raspberry Pi header GPIOs with jumper wires between the pairs listed in `dectnrpi_nds_nrf9151_ns.overlay`.
Connect these header pins with jumper wires (out pin to in pin):

| Pair | out (header) | in (header) |
|------|--------------|-------------|
| 1    |  8 (P0.13)   | 10 (P0.12)  |
| 2    | 11 (P0.15)   | 13 (P0.27)  |
| 3    | 15 (P0.26)   | 19 (P0.21)  |
| 4    | 21 (P0.22)   | 23 (P0.23)  |
| 5    | 24 (P0.24)   | 36 (P0.16)  |

Do not connect pins 27, 29 and 31 (buttons).

### LED/button pairs (operator)

Each LED is confirmed by its paired button within 15 s: led0 with SW0, led1 with SW1, led2 with SW2.

### External flash

- JEDEC ID must be EF 40 16.
- The first 4 KiB of `external_flash_partition` are erased, checked, written and read back. Nothing else uses this partition in this application.

## Run
create a hardware map yaml
```bash
- connected: true
  id: "000000000000"
  platform: dectnrpi_nds/nrf9151/ns
  product: J-Link
  runner: jlink
  serial: /dev/ttyACM0
```

```bash
west twister -p dectnrpi_nds/nrf9151/ns --device-testing --device-serial /dev/ttyACM0 \
  -T deps/deveritec-boards/tests/boards/dectnrpi_nds

west twister --device-testing --hardware-map map.yml --test-only -T tests/boards/dectnrpi_nds -vv
```

1. add the jumper wires for the loopback test (see: GPIO loopback (Wiring))
2. run twister build the test-app, flash and execute (`--test-only` flash and execute only)
3. the LED button test, Operator interaction required: each LED is switched on in turn and confirmed by its paired button (led0 <-> button0 (SW0), led1 <-> button1 (SW1), led2 <-> button2 (SW2))
4. automatic test for ext.-flash and gpio-loopback is run automatic
5. if one of the tests will fail the blue and read led will be active (if ok blue and green)
