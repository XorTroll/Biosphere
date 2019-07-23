# Dynamic NRO loading

libbio supports dynamic NRO loading (code ported from libtransistor), via both classic `dlopen` / `dlsym` and some nice high-level system via `bio::ld::Module`.

## Building

Compile `demolib`, what will generate our NRO library: `demo.lib.nro`.

Place the file on the SD card root, or change the path in `program` to a custom one and place the library there.

Run `program` and check output logs.