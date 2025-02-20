//
// Created by shop on 19/02/2025.
//

#ifndef MIDIC_MIDIC_H
#define MIDIC_MIDIC_H

#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include "../libs/libremidi/include/libremidi/message.hpp"

namespace v {

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

        void init(const char *name, Mode mode) {
            filePointer = fopen(name, mode == Mode::READ ? "rb" : "wb+");
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
                        t.startMessage = mCurr;
                        t.length = mCurr.timestamp - notes[mCurr.note].timestamp;
                        touches.push_back(t);
                    }
                }
            }

            for (const auto &touch : touches) {
                printf("Note=%d Length=%f\n", touch.startMessage.note, (double)touch.length/Message::seconds);
            }
        }
    };

} // v

#endif //MIDIC_MIDIC_H
