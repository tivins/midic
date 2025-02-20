#include <libremidi/libremidi.hpp>
#include <iostream>
#include <csignal>
#include "src/Midic.h"


v::RawFile rawFile;
static bool running = true;

void my_handler(sig_atomic_t s) {
    running = false;
}

auto my_callback = [](const libremidi::message &message) {
    if (message.size() != 3) {
        fprintf(stderr, "WARNING: message size is not 3 but %zu\n", message.size());
        return;
    };
    // printf("%lld:%2X,%2X,%2X\n", message.timestamp, message[0], message[1], message[2]);
    rawFile.push(v::Message(message.timestamp, message[0], message[1], message[2]));
};

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    std::cout << "MIDI-Recorder\n";
    signal(SIGINT, my_handler);

    try {
        const char *filename = "out.raw";
        libremidi::midi_in midi{libremidi::input_configuration{.on_message = my_callback}};
        midi.open_port(libremidi::midi1::in_default_port().value());
        rawFile.init(filename, v::RawFile::WRITE);
        std::cout << "Listening... (Ctrl+C to finish)\n";
        while (running); // loop while ctrl+c
        rawFile.close();
        std::cout << "File saved at '" << filename << "'.\n";
    } catch (std::bad_optional_access &e) {
        std::cerr << "| ERROR:\n";
        std::cerr << "| Cannot read MIDI instrument.\n";
        std::cerr << "| (what: " << e.what() << ")\n";
    }


    return 0;
}
