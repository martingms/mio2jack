# mio2jack

`mio2jack` is a tool to expose [sndio](sndio.org) MIDI-ports in
[jack](jackaudio.org).

For now it only handles one-way communication, that is MIDI events flowing
in the sndio → jack direction. Patches accepted!

## Usage

    usage: mio2jack [-h] [-p mio_port] [-n jack_client_name]

### Examples

    $ mio2jack -p midithru/0 -n sndio-midithru/0

Will expose the sndiod midithru-box `midithru/0` in jack with the client name
`sndio-midithru/0`. That client will have a single MIDI output, from which all
MIDI events going through `midithru/0` will be output.
