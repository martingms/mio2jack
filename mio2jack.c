#include <stdio.h>
#include <unistd.h>
#include <signal.h>

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


static void quit(int code)
{
    if (jack_client) jack_client_close(jack_client);
    if (mio_hdl) mio_close(mio_hdl);
    if (event_buf) free(event_buf);

    exit(code);
}

static void signal_handler(int signal)
{
    quit(EXIT_SUCCESS);
}

static void usage(void)
{
    extern char* __progname;

    fprintf(stderr,
            "usage: %s [-h] [-p mio_port] [-n jack_client_name]\n",
            __progname);

    quit(EXIT_FAILURE);
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

    event_buf = malloc(MIO_READ_SZ);
    if (!event_buf) {
        perror("malloc");
        quit(EXIT_FAILURE);
    }
    mio_hdl = mio_open(mio_port, MIO_IN, 1);
    if (!mio_hdl) {
        fprintf(stderr, "couldn't open sndio port: %s\n", mio_port);
        quit(EXIT_FAILURE);
    }

    jack_client = jack_client_open(jack_client_name, JackNullOption, 0);
    if (!jack_client) {
        fprintf(stderr, "couldn't open jack client\n");
        quit(EXIT_FAILURE);
    }

    jack_set_process_callback(jack_client, process, NULL);

    jack_out = jack_port_register(jack_client, "midi_out",
            JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
    if (!jack_out) {
        fprintf(stderr, "couldn't register jack output port\n");
        quit(EXIT_FAILURE);
    }

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    if (jack_activate(jack_client)) {
        fprintf(stderr, "couldn't activate jack client\n");
        quit(EXIT_FAILURE);
    }

    while (1) sleep(1);

    /* unreachable */
    quit(EXIT_SUCCESS);
}
