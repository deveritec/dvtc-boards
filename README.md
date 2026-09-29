# Deveritec Boards

This repository includes all board support packages for deveritec PCBs. It is structured as a Zephyr module.

## Current boards

- `dectnrpi_nds` — Deveritec DECT NR+ Raspberry Pi Shield (with nRF9151)

## Usage

Add this repository to your West manifest:

```yaml
manifest:
  projects:
    - name: deveritec-boards
      url: https://github.com/deveritec/dvtc-boards
      revision: main
      path: deps/deveritec-boards
```

Then run:

```bash
west update
```

Build the DECT NR+ Pi board with:

```bash
west build -b dectnrpi_nds/nrf9151/ns
```

Targets:

- `dectnrpi_nds/nrf9151/ns`: non-secure application build
- `dectnrpi_nds/nrf9151`: secure build (TF-M)
