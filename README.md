# Luos GATT

This project aims to implement a minimalistic Luos network built on
Nordic PCA10040 boards.

This network shall consist in two nodes:
- The Gate node, hosting a Gate container and a LED Toggle container.
- The Actuator node, hosting a LED Toggle container.

The Gate node shall be connected via UART to a pilot computer, so that
the user can manage the network using the PyLuos interface. The nodes
shall communicate using a serial channel implemented over Bluetooth Low
Energy.

## Mandatory packages

In order for all the steps in this README to be executed, the packages
listed below must be present on the host environment:

* `sudo`: There will be root operations (mostly for packages).

* `docker` _(and friends)_: Build is going to be performed through a
container.

* `python3`: Necessary for using the Pyluos interface.

## Available programs

This repository contains the source code for the two nodes described
above:

* The Gate node, in the `gate_node` directory.
* The Actuator node, in the `actuator_node` directory.

Along with these nodes, five test programs are provided in teh `tests`
directory:

* `msg_queue`: Test for the message queue data structure used in the
project. Enqueues messages received from serial, then dequeues them when
either receiving a special message or when the queue is full.
* `ptp_client`: Test for the PTP service used in the project. When set
up and connected with a remote PTP Server instance, toggles LED1 of both
boards on Button 1 event.
* `ptp_server`: Test for the PTP service used in the project. When set
up and connected with a remote PTP Client instance, toggles LED1 of both
boards on Button 1 event.
* `systick`: Test for the Systick module of the Luos HAL. Prints the
current tick at each round of the main loop.
* `uart`: Test for the UART helper module used in the project. When set
up, logs for each UART event the data received and its size.

In the following parts of this document, the name of a program refers
to the name of the directory containing its sources.

## Build

A Docker image of the environment of this project shall first be created
using the `Dockerfile` present at the root of this repository:

```bash
docker build -t <IMAGE_NAME> .
```

Once the image is built, a program can be built using the following
command:

```bash
docker run -t                                                       \ # Display output on terminal.
    -v $(pwd)/<HOST_OUTPUT_DIR>:/luos_gatt/<CONTAINER_OUTPUT_DIR>   \ # Link host and container output directories.
    <IMAGE_NAME>                                                    \ # Name provided to the `docker build` command.
    [BUILD_DIR_NAME=<CONTAINER_BUILD_DIR>]                          \ # Default build directory is named `build`.
    [OUTPUT_DIR_NAME=<CONTAINER_OUTPUT_DIR>]                        \ # Default output directory is named `output`.
    ./build.sh <PROGRAM_NAME> [<PROGRAM_NAME> ...]                  \ # This script can build several programs.
```

A built program can be found in the corresponding
`<HOST_OUTPUT_DIR>/<PROGRAM_NAME>` folder, with the name
`<PROGRAM_NAME>_merged.hex`.

## Flash

In order to flash a built program on a board, the Docker image of the
project environment shall already have been created.

A built program can be flashed on a board using the following command:

```bash
docker run -t                                                       \ # Display output on terminal.
    --privileged=true                                               \ # Allows JLink to access the boards.
    -v $(pwd)/<HOST_OUTPUT_DIR>:/luos_gatt/<CONTAINER_OUTPUT_DIR>   \ # Link host and container output directories.
    <IMAGE_NAME>                                                    \ # Name provided to the `docker build` command.
    [OUTPUT_DIR_NAME=<CONTAINER_OUTPUT_DIR>]                        \ # Default output directory is named `output`.
    ./flash.sh <PROGRAM_NAME> <BOARD_NUMBER>                        \ # <BOARD_NUMBER> being the serial number of the board.
        [(<PROGRAM_NAME> <BOARD_NUMBER>) ...]                       \ # This script can flash several programs.
```

Each program name must be followed with the serial number of a board.

## Physical setup

If programs are run in `DEBUG` mode, they must be connected to the pilot
PC to allow reading logs through the `JLinkRTTViewerExe` command.

The Gate node _(or board hosting either the_ `msg_queue` _or the_ `uart`
_test program)_ shall be connected to the pilot PC through UART with the
following connections:

| **PC FUNCTIONALITY** | **BOARD FUNCTIONALITY** | **BOARD PIN** |
| -------------------- | ----------------------- | ------------- |
| GND | GND | GND |
| RX | TX | P0.06 |
| TX | RX | P0.08 |

The terminal emulator settings for communication with the Gate container
must be the following:

| **VARIABLE** | **VALUE** |
| ------------ | --------- |
| Baudrate | 115 200 |
| Word length | 8 |
| Stop bits | 1 |
| Parity | None |
| Hardware Flow Control | None |

## System usage

The system cannot properly start before a BLE connection is established
between the nodes, which is made apparent by both boards' LED 4 turning
off.

In order to control the system through Pyluos, the Python interpreter
shall be run **inside the** `resources/Pyluos` **folder from this
repository**:

```bash
cd resources/Pyluos
python3
# Now this is python prompt
from pyluos import Device                                               # The Device class stores information about the whole Luos network.
<DEVICE_NAME> = Device(host=<HOST_NAME>, log_conf=<CONF_FILE>)          # <HOST_NAME> is the system device representing the connection to the Gate, <CONF_FILE> is the JSON logging configuration.
<DEVICE_NAME>.led_toggler.state = <True|False>                          # Turns the Gate node's LED on or off respectively.
<DEVICE_NAME>.led_toggler1.state = <True|False>                         # Turns the Actuator node's LED on or off respectively.
print(<DEVICE_NAME>.<led_toggler|led_toggler1>.state)                   # Prints the state of the chosen node's LED.
```

The extent of the interactions you can have with the network is
described in the Pyluos documentation: <https://docs.luos.io/pages/software/pyluos.html>.

You can also use a terminal emulator on the serial device representing
the board, with the following configuration:

| **VARIABLE** | **VALUE** |
| ------------ | --------- |
| Baudrate | 115 200 |
| Word length | 8 |
| Stop bits | 1 |
| Parity | None |
| Hardware Flow Control | None |

## Debug mode

A debug mode is available for both nodes, allowing the user to view RTT
logs emitted by the program.

In order to enable it, you must uncomment the `DEBUG` compile option in
the `CMakeLists.txt` of the desired program. Once this is done, you can
build and flash the programs on the boards using the commands described
above.

To access the logs, run the following command:

```bash
JLinkRTTViewerExe --autoconnect     \ # Automatically connect.
    --device nrf52832_xxaa          \ # Board family.
    --serialnumber <BOARD_NUMBER>   \ # Serial number of the desired board.
```

## Notes

* The `cmake` folder in the `resources` directory comes from a
third-party repository: <https://github.com/polidea/cmake-nrf5x>. As a
matter of convenience and maintainability, and since we only use a part
of it, it has been elected to copy this part in our repository rather
than creating a submodule.

* The `Luos` and `Pyluos` submodules in the `resources` directory are
forks from the following respective repositories:
<https://github.com/Luos-io/Luos.git> and
<https://github.com/Luos-io/Pyluos.git>.
