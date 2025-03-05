//
// Created by shop on 20/02/2025.
//

#ifndef MIDIC_EFFECTS_H
#define MIDIC_EFFECTS_H

#include <vector>
#include "Util.h"

namespace v {

    class Particle {
    public:
        float opacity = 1;
        float size = 1;
        Vec pos;
        Vec dir;

        Particle(const Vec &pos, const Vec &dir) : pos(pos), dir(dir) {}
    };

    class Particles {
    public:
        std::vector<Particle> elements{};

        void emitAt(const Vec &pos, const Vec &dir) {
            elements.emplace_back(pos, dir);
        }

        void update() {
            auto idx = std::begin(elements);
            while (idx != std::end(elements)) {
                auto &particle = *idx;
                const float random_value_f = Util::pseudoRandom() * 1 - .5f;
                particle.opacity -= Util::pseudoRandom() * .06f;
                particle.dir.y -= Util::pseudoRandom() * .07f;
                particle.dir.x += random_value_f;
                particle.pos.x += particle.dir.x;
                particle.pos.y += particle.dir.y;

                if (particle.opacity < 0.1) {
                    idx = elements.erase(idx);
                    continue;
                }
                idx++;
            }
        }
    };
    class Board {
    public:

        static const uint8_t num_white = 52; // A-1
        static const uint8_t note_lowest = 0x15; // A-1
        static const uint8_t note_highest = 0x6C; // C-7
        constexpr static const int white_suite[] = {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0};

        float widthWhite{};
        float x = {};
        float pos[128] = {};

        explicit Board(float pX, float pWidth) {
            widthWhite = pWidth / (float) num_white;
            x = pX;
        }

        static bool isWhite(int note) {
            return white_suite[(note - note_lowest) % 12];
        }

        [[nodiscard]] float getPos(uint8_t i) const {
            return x + pos[i];
        }

        void buildPos() {
            int index = 0;
            int white_inc = 0;
            for (uint8_t note = Board::note_lowest; note <= Board::note_highest; note++) {
                if (Board::isWhite(note)) {
                    pos[note] = (float) white_inc * widthWhite;
                    white_inc++;
                }
                index++;
            }
            index = 0;
            white_inc = 0;
            for (uint8_t note = Board::note_lowest; note <= Board::note_highest; note++) {
                if (!Board::isWhite(note)) {
                    pos[note] = (float) white_inc * widthWhite + widthWhite / 2;
                    white_inc++;
                } else if (Board::isWhite(note - 1)) {
                    white_inc++;
                }
                index++;
            }
        }
    };
}
#endif //MIDIC_EFFECTS_H
