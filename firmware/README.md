# Firmware directory

Firmware may be compiled using [platformio](https://piolabs.com/), which provides a vendor-neutral command line environment for embedded development supporting modern features such as continous integration, unit testing and operating system independence.

## Installing platformio

See [What is PlatformIO?](https://docs.platformio.org/en/latest/what-is-platformio.html?utm_source=github&utm_medium=core) and then either [PlatformIO IDE](https://docs.platformio.org/en/latest/integration/ide/pioide.html) or [PlatformIO CLI](https://docs.platformio.org/en/latest/core/index.html).

## Using platformio

To compile simply type `pio run`.

For other commands, see [CLI command list](https://docs.platformio.org/en/latest/core/userguide/index.html#piocore-userguide).

For example:

 * `pio project config --lint` will verify project configuration.
 * `pio package install` will install missing dependencies.
 * `pio check` will run a static code analysis.
 * `pio run` will build the project.
 * `pio pkg update` will update dependencies.
 * `pio test -h` will show testing related commands.
