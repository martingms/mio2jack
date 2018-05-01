#include <stdio.h>
#include <unistd.h>

#include <sndio.h>
#include <jack/jack.h>
#include <jack/midiport.h>


char* mio_port = MIO_PORTANY;
struct mio_hdl* mio_hdl;

char* jack_client_name = "mio2jack";
jack_client_t* jack_client;
jack_port_t* jack_out;

#define MIO_READ_SZ 256
unsigned char* event_buf;

static void usage(void)
{
    extern char* __progname;

    fprintf(stderr,
            "usage: %s [-h] [-p mio_port] [-n jack_client_name]\n",
            __progname);

    exit(EXIT_FAILURE);
}

static int process(jack_nframes_t nframes, void* arg)
{
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
    int ch;
    while ((ch = getopt(argc, argv, "p:n:h")) != -1) {
        switch (ch) {
        case 'p':
            mio_port = optarg;
            break;
        case 'n':
            jack_client_name = optarg;
            break;
        case 'h': /* fallthrough */
        default:
            usage();
        }
    }

    /* TODO: check and handle returns */
    event_buf = malloc(MIO_READ_SZ);
    mio_hdl = mio_open(mio_port, MIO_IN, 1);

    jack_client = jack_client_open(jack_client_name, JackNullOption, 0);
    jack_set_process_callback(jack_client, process, NULL);

    jack_out = jack_port_register(jack_client, "midi_out",
            JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);

    jack_activate(jack_client);

    /* TODO: Install signal handlers to clean up. */
    while(1) sleep(1);
}
