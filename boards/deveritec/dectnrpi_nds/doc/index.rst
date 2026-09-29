.. zephyr:board:: dectnrpi_nds

Overview
********

The Deveritec DECT NR+ Pi Shield is a compact development platform built around the
Nordic Semiconductor nRF9151 System-in-Package (SiP). It is intended for
evaluating and prototyping DECT NR+ (also known as DECT-2020 New Radio)
applications, and can operate either as a child node or as a gateway.

The Shield plugs directly to Raspberry Pi or over cables to the
USB-to-serial bridge for the nRF9151.

More information about the board can be found on the `DECT NR+ Pi Shield Product Page`_
and in the `DECT NR+ Pi Shield Datasheet`_.

.. note::
   You need to disable the TFM PROTECTED-STORAGE ``CONFIG_TFM_PARTITION_PROTECTED_STORAGE=n`` since we removed it from the standard partitioning scheme.

Hardware
********

- Nordic Semiconductor nRF9151 SiP

  - Arm Cortex-M33 running at up to 64 MHz
  - 1 MB flash and 256 KB RAM
  - Arm TrustZone and Arm CryptoCell security

- DECT NR+ radio with support for bands 1, 2, 9 and 22
- Multiple USB/UART interface options:
  - Raspberry Pi powered
  - USB-to-serial converter
- Supports both external or onboard antenna configuration
- USB-C connector for power and communication
- Three hardware configurable open-drain I/Os or user LED0..2
- 3× user buttons + 1× reset button
- Access to both external SYNC_IN & SYNC_OUT pins
- 32MB external flash
- Operating supply voltage 2.5 V to 5.5 V
- Multiple power options:
  - Raspberry Pi powered
  - USB powered
  - Battery powered
- Operating temperature range -40 to +85 degrees Celsius
Supported Features
==================

.. zephyr:board-supported-hw::

Connections and IOs
===================

.. image:: img/dectnrpi_nds_pinout.svg.svg
      :alt: Pinout of the dectnrPi Board

LED
---

* LED0 (blue) = P0.19
* LED1 (green) = P0.18
* LED2 (red) = P0.20

BUTTON
---

* SW0 = P0.17
* SW1 = P0.14
* SW2 = P0.28

UART
----

The nRF9151 ``uart0`` is connected to the onboard USB-to-serial interface, which
exposes it to the USB host as a serial port. It is used as the Zephyr
console and shell backend at 115200 baud (8N1).

* UART0 TX = P0.04 (default, onboard USB-to-serial channel A)
* UART0 RX = P0.25 (default, onboard USB-to-serial channel A)

* UART1 TX = P0.03 (onboard USB-to-serial channel B)
* UART1 RX = P0.02 (onboard USB-to-serial channel B)

* UART0 TX = P0.12 (Raspberry Pi header UART0)
* UART0 RX = P0.13 (Raspberry Pi header UART0)

* UART1 TX = P0.22 (Raspberry Pi header UART3)
* UART1 RX = P0.24 (Raspberry Pi header UART3)

Use UART1 (nrf) at Raspberry Pi 5 (UART3)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Change the following settings at ``/boot/firmware/config.txt``.
See `Pinout UART <https://pinout.xyz/pinout/uart>`_.

.. code-block:: ini

   [pi5]
   dtoverlay=nospi10
   # RPI5: GPIO8/9 --> /dev/ttyAMA3 (shell UART)
   dtoverlay=uart3-pi5

   [all]

   dtoverlay=uart0-pi5
   enable_uart=1

   #dtoverlay=disable-bt
   dtoverlay=disable-bt-pi5
   dtoverlay=uart0
   dtoverlay=uart3

   dtparam=spi=off
   dtparam=i2c_arm=off
   dtparam=uart0=on

After the reboot, the following devices should appear:

- ``/dev/ttyAMA0`` (UART0 at nrf)
- ``/dev/ttyAMA3`` (UART1 at nrf)

Check the UART configuration
----------------------------

Check whether the UART pins are already configured:

.. code-block:: bash

   # for /dev/ttyAMA0
   pinctrl get 14,15
   14: a4    pn | hi // GPIO14 = TXD0
   15: a4    pu | hi // GPIO15 = RXD0

   # for /dev/ttyAMA3
   pinctrl get 8,9
    8: a2    pn | hi // GPIO8 = TXD3
    9: a2    pu | hi // GPIO9 = RXD3

If the pins are still not configured, a reboot is required.

Devicetree overlay
------------------

Use an overlay in ``dectnrpi_nds``, for example:

.. code-block:: dts

   /* Use RPi-Pin Header */
   &uart0 {
       status = "okay";
       current-speed = <115200>;
       pinctrl-0 = <&uart0_pi_default>;
       pinctrl-1 = <&uart0_pi_sleep>;
       pinctrl-names = "default", "sleep";
   };

   &uart1 {
       status = "okay";
       current-speed = <115200>;
       pinctrl-0 = <&uart1_pi_default>;
       pinctrl-1 = <&uart1_pi_sleep>;
       pinctrl-names = "default", "sleep";
   };

   /* disable RPI SPI Pin Header - to prevent interference */
   &spi0 {
       status = "disabled";
   };


Security
********

The nRF9151 supports Arm TrustZone. By default the ``dectnrpi_nds/nrf9151``
board target builds a single Secure image, while the ``/ns`` variant builds a
Non-Secure application that runs alongside Trusted Firmware-M (TF-M) as the
Secure firmware. Most DECT NR+ and modem applications require the ``/ns``
variant, since the modem firmware and its security services are only reachable
from Non-Secure firmware through TF-M.

Programming and Debugging
*************************

.. zephyr:board-supported-runners::

The DECT NR+ Pi Shield need a external debugger probe. Simply plug the debugger into the TAG-connector.

Building an application
=======================

Applications that use the modem and DECT NR+ features must be built for the
Non-Secure board target ``dectnrpi_nds/nrf9151/ns``. Applications
that do not require Non-Secure execution can be built for the Secure target
``dectnrpi_nds/nrf9151``.

Flashing
========

Use the :zephyr:code-sample:`blinky` sample to verify that Zephyr runs correctly
on the board. Build and flash it with:

.. zephyr-app-commands::
   :zephyr-app: samples/basic/blinky
   :board: dectnrpi_nds/nrf9151
   :goals: build flash

To connect to the console, open the serial port presented by the shield (for
example ``/dev/ttyACM0`` on Linux) at 115200 baud.

Example flash-command:
``west flash --erase -d build/blinky``

Debugging
=========

Use a compatible "ARM20 to 6 Pin Plug-of-Nails™ - No Legs".
Segger JLink provides variants of those, combination with adaptor to SeggerJLink makes it easier to handle
`DECT NR+ Pi Shield Debugger-all`_ 
`DECT NR+ Pi Shield Debugger-cable`_  
`DECT NR+ Pi Shield Debugger-adapter`_
`DECT NR+ Pi Shield Probe-all`_.

.. zephyr-app-commands::
   :zephyr-app: samples/basic/blinky
   :board: dectnrpi_nds/nrf9151
   :goals: debug

References
**********

.. target-notes::

.. _DECT NR+ Pi Shield Product Page: https://deveritec.com/solutions-dectnrp/
.. _DECT NR+ Pi Shield Datasheet: https://deveritec.com/wp-content/uploads/2026/09/RPiSiNrpDP.pdf
.. _DECT NR+ Pi Shield Debugger-all: https://www.tag-connect.com/debugger-cable-selection-installation-instructions/segger-j-link#71_72_145-149
.. _DECT NR+ Pi Shield Debugger-cable: https://www.tag-connect.com/product/tc2030-idc-nl
.. _DECT NR+ Pi Shield Debugger-adapter: https://www.tag-connect.com/product/arm20-ctx-20-pin-to-tc2030-idc-adapter-for-cortex
.. _DECT NR+ Pi Shield Probe-all: https://www.tag-connect.com/debugger-cable-selection-installation-instructions/segger-j-link?add-to-cart=2151
