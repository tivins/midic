#include <iostream>
#include <libremidi/libremidi.hpp>
#include <csignal>
#include <cstdio>
#include "src/Midic.h"


v::RawFile rawFile;
static bool running = true;

void my_handler(sig_atomic_t s) {
    printf("Caught signal %d\n", s);
    running = false;
    //    exit(1);

}

auto my_callback = [](const libremidi::message &message) {
    // how many bytes
    if (message.size() != 3) {
        fprintf(stderr, "WARNING: message size is %zu\n", message.size());
        return;
    };
    // access to the individual bytes and the timestamp
    printf("%lld:%2X,%2X,%2X\n", message.timestamp, message[0], message[1], message[2]);
    rawFile.push(v::Message(message.timestamp, message[0], message[1], message[2]));
};

int main() {
    printf("Midirec\n");
    signal(SIGINT, my_handler);

    try {
        libremidi::midi_in midi{libremidi::input_configuration{.on_message = my_callback}};
        midi.open_port(libremidi::midi1::in_default_port().value());
        rawFile.init("out.raw", v::RawFile::WRITE);
        std::cout << "Listening... (Ctrl+C to finish)" << std::endl;
        while (running);
        rawFile.close();
    } catch (std::bad_optional_access &e) {
        std::cerr << "Exception " << e.what() << std::endl;
        std::cerr << "  Cannot read midi instrument. Process aborted." << std::endl;
    }


    return 0;
}
