//
// Created by shop on 23/02/2025.
//

#include <string>
#include "lib/Midic.h"
#include <fmt/format.h>
#include <libremidi/writer.hpp>
#include <fstream>

using namespace v;

int main(int argc, char ** argv) {
    std::string infile = "../data/example.raw";
    std::string outfile = "../files/example.midi";

    libremidi::writer writer;

    RawFile rawFile;
    rawFile.init(infile, RawFile::READ);

    MidiData md;
    Message rawMsg;
    libremidi::message msg;

    while (rawFile.read(&rawMsg)) {
        int track = 1;
        int tick = (int) rawMsg.timestamp / 100000;
        if (static_cast<libremidi::message_type>(rawMsg.status) == libremidi::message_type::NOTE_ON) {
            msg = libremidi::channel_events::note_on(1, rawMsg.note, rawMsg.velocity);
        }
        if (static_cast<libremidi::message_type>(rawMsg.status) == libremidi::message_type::NOTE_OFF) {
            msg = libremidi::channel_events::note_off(1, rawMsg.note, rawMsg.velocity);
        }
        // Tracks will be added as needed within safe limits
        writer.add_event(tick, track, msg);
    }
    rawFile.close();

    // Read raw from a MIDI file
    std::ofstream output{outfile, std::ios::binary};
    writer.write(output);
    return 0;
}