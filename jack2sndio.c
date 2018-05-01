#include <stdio.h>
#include <unistd.h>

#include <sndio.h>
#include <jack/jack.h>
#include <jack/midiport.h>


struct mio_hdl* mio_hdl;

jack_client_t* jack_client;
jack_port_t* jack_out;
jack_port_t* jack_in;

#define MIO_READ_SZ 256
unsigned char* event_buf;

/* TODO: use blocking io with mio, and poll to see if anything to read etc. */
static int process(jack_nframes_t nframes, void* arg)
{
    /* Transfer events from jack to sndio. */
    void* jack_in_buf = jack_port_get_buffer(jack_in, nframes);
    jack_nframes_t nevents = jack_midi_get_event_count(jack_in_buf);

    jack_midi_event_t jack_event;
    for (size_t i = 0; i < nevents; i++) {
        jack_midi_event_get(&jack_event, jack_in_buf, i);

        /* FIXME Throwing away the timing information for now. */
        mio_write(mio_hdl, jack_event.buffer, jack_event.size);
    }

    /* Transfer events from sndio to jack. */
    void* jack_out_buf = jack_port_get_buffer(jack_out, nframes);
    jack_midi_clear_buffer(jack_out_buf);

    size_t read;
    while (read = mio_read(mio_hdl, event_buf, MIO_READ_SZ)) {
        jack_midi_event_write(jack_out_buf, 0, event_buf, read);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    /* TODO: check and handle returns */

    event_buf = malloc(MIO_READ_SZ);
    /* TODO: port through getopt() */
    mio_hdl = mio_open(MIO_PORTANY, MIO_IN | MIO_OUT, 1);
    /* TODO: name through getopt() */
    jack_client = jack_client_open("jack2sndio", JackNullOption, 0);
    jack_set_process_callback(jack_client, process, NULL);

    jack_out = jack_port_register(jack_client, "midi_out",
            JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
    jack_in = jack_port_register(jack_client, "midi_in",
            JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);

    jack_activate(jack_client);

    /* TODO: Install signal handlers to clean up. */
    while(1) sleep(1);
}
