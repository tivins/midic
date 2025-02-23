//
// Created by shop on 19/02/2025.
//

#ifndef MIDIC_MIDIC_H
#define MIDIC_MIDIC_H

#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include "libremidi/message.hpp"
#include "../../libs/fmt-11.1.3/include/fmt/format.h"
#include "Util.h"

namespace v {

    class RasterConfig {
    public:
        Col white_up;
        Col white_down;
        Col black_up;
        Col black_down;
        int frameRate = 25;
        Size videoSize;
        unsigned int seed = 0;
        std::string title;
        std::string author;
        std::string sub_text;
        int numFrames = 1;
        bool buildVideo = true;
        bool debug = false;
        std::string mp4Filename;
        std::string imagesPath;

        void print() const {
            fmt::print("Raster configuration:\n");
            fmt::print("colors:\n");
            fmt::print("  white_up: [{},{},{},{}]\n", white_up.r, white_up.g, white_up.b, white_up.a);
            fmt::print("  white_down: [{},{},{},{}]\n", white_down.r, white_down.g, white_down.b, white_down.a);
            fmt::print("  black_up: [{},{},{},{}]\n", black_up.r, black_up.g, black_up.b, black_up.a);
            fmt::print("  black_down: [{},{},{},{}]\n", black_down.r, black_down.g, black_down.b, black_down.a);
            fmt::print("video:\n");
            fmt::print("  buildVideo: {}\n", buildVideo);
            fmt::print("  frameRate: {}\n", frameRate);
            fmt::print("  numFrames: {}\n", numFrames);
        }
    };

    class Message {
    public:
        constexpr static const float seconds = 1000000000.0;

        uint64_t timestamp{};
        uint8_t status{};
        uint8_t note{};
        uint8_t velocity{};

        Message() = default;

        Message(uint64_t timestamp, uint8_t status, uint8_t note, uint8_t velocity) :
                timestamp(timestamp),
                status(status),
                note(note),
                velocity(velocity) {
        }

        [[nodiscard]] double getSeconds() const {
            return static_cast<double>(timestamp) / seconds;
        }

    };

    class RawFile {
        FILE *filePointer = nullptr;
    public:
        enum Mode {
            WRITE, READ
        };

        void init(const std::string &name, Mode mode) {
            filePointer = fopen(name.c_str(), mode == Mode::READ ? "rb" : "wb+");
            uint64_t magic = 123456789;
            switch (mode) {
                case Mode::READ:
                    uint64_t magic_input;
                    fread(&magic_input, sizeof(uint64_t), 1, filePointer);
                    if (magic_input != magic) {
                        throw std::runtime_error("invalid magic number");
                    }
                    break;
                case Mode::WRITE:
                    fwrite(&magic, sizeof(uint64_t), 1, filePointer);
                    break;
            }
        }

        void push(const Message &msg) {
            fwrite(&msg.timestamp, sizeof(int64_t), 1, filePointer);
            fputc(msg.status, filePointer);
            fputc(msg.note, filePointer);
            fputc(msg.velocity, filePointer);
        }

        bool read(Message * msg) const {

            auto bytes = fread(&msg->timestamp, sizeof(int64_t), 1, filePointer);
            if (!bytes) {
                return false;
            }
            msg->status = fgetc(filePointer);
            msg->note = fgetc(filePointer);
            msg->velocity = fgetc(filePointer);
            return true;
        }

        void close() {
            fclose(filePointer);
        }
    };

    class Touch {
    public:
        uint64_t length{};
        Message startMessage{};
    };

    class MidiData {
    public:
        std::vector<Touch> touches;
        void parse(const RawFile& file) {
            Message mCurr;
            Message notes[128]{};
            while (file.read(&mCurr)) {
                if (static_cast<libremidi::message_type>(mCurr.status) == libremidi::message_type::NOTE_ON) {
                    notes[mCurr.note] = mCurr;
                }
                else if (static_cast<libremidi::message_type>(mCurr.status) == libremidi::message_type::NOTE_OFF) {
                    if (notes[mCurr.note].timestamp) {
                        Touch t;
                        t.startMessage = notes[mCurr.note];
                        t.length = mCurr.timestamp - notes[mCurr.note].timestamp;
                        touches.push_back(t);
                    }
                }
            }

            // for (const auto &touch : touches) {
            //     printf("Note=%d Length=%f\n", touch.startMessage.note, (double)touch.length/Message::seconds);
            // }
        }
    };

} // v

#endif //MIDIC_MIDIC_H
