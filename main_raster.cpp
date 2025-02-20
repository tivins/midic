// Created by Tivins on 19/02/2025.

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "libs/stb_image_write.h"

#define CANVAS_ITY_IMPLEMENTATION

#include "libs/canvas_ity/canvas_ity.hpp"
#include "src/Midic.h"
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <ctime>

std::string exec(const char* cmd) {
    std::array<char, 128> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

float vRandom() {
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

class Vec {
public:
    float x{},y{};
    Vec() = default;

    Vec(float _x, float _y) { x=_x; y=_y;}

    Vec(const Vec& p) { x=p.x; y=p.y;}
};

class Particle {
public:
    float opacity=1;
    Vec pos;
    Vec dir;
    Particle(const Vec& pos, const Vec& dir):pos(pos),dir(dir) {}
};

class Particles {
public:
    std::vector<Particle> elements;
    void emitAt(const Vec& pos, const Vec& dir) {
        elements.emplace_back(pos, dir);
    }
    void update() {
        for (auto & particle : elements) {
            const float random_value_f = vRandom() * 2 - 1;
            particle.dir.y -= vRandom() * .1;
            particle.opacity -= vRandom() * .25;
            particle.dir.x += random_value_f;
            particle.pos.x += particle.dir.x;
            particle.pos.y += particle.dir.y;
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
        printf("index=%d\n", index);
    }
};

void draw_board(Board &board, canvas_ity::canvas &context, float y, float height) {
    context.set_color(canvas_ity::brush_type::fill_style, .8, .8, .8, 1);
    for (uint8_t note = Board::note_lowest; note <= Board::note_highest; note++) {
        if (Board::isWhite(note)) {
            float pos = board.getPos(note) - 1;
            context.fill_rectangle(pos, y, board.widthWhite - 2, height);
        }
    }
    context.set_color(canvas_ity::brush_type::fill_style, .2, .2, .2, 1);
    for (uint8_t note = Board::note_lowest; note <= Board::note_highest; note++) {
        if (!Board::isWhite(note)) {
            context.fill_rectangle(board.getPos(note) - 1, y, board.widthWhite - 2, (float) height / 1.5f);
        }
    }
}



/*
class File {
public:
    unsigned char *data = nullptr;
    size_t size = 0;
    ~File() {
        delete [] data;
    }
    void load(const char *filename) {
        FILE *fp = fopen(filename, "rb");
        if (!fp) {
            throw std::runtime_error("cannot load file");
        }
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        printf("Size=%zu", size);
        rewind(fp);
        data = new unsigned char[size]();
        fwrite(data, sizeof(unsigned char), size, fp);
        fclose(fp);
    }
};
*/

void render_frame(Particles * particles, v::MidiData *md, Board &board, canvas_ity::canvas &context, int width, int height, int frame) {

    int ratio = 25;
    float t = static_cast<float>(frame) / static_cast<float>(ratio);


    particles->update();
    context.set_shadow_blur(5);
    for (const auto particle : particles->elements) {
        if (particle.opacity < 0.1) {
            continue;
        }
        context.set_shadow_color(1,1,1,particle.opacity);
        context.set_color(canvas_ity::brush_type::fill_style, .9, .9, .9, particle.opacity);
        context.fill_rectangle(particle.pos.x - 1, particle.pos.y - 1, 2, 2);
    }
    context.set_shadow_blur(0);


    float bottomY = 600 - 110 - 10;

    context.set_color(canvas_ity::brush_type::stroke_style, .5, .6, .7, 1);
    context.move_to(0, bottomY);
    context.line_to(width, bottomY);
    context.stroke();

    // read part
    float stretch = 200;
    for (const auto &touch: md->touches) {
        float startAt = (float) (touch.startMessage.getSeconds() - t) * stretch;
        float length  = ((float) touch.length / v::Message::seconds) * stretch;
        float noteTop = bottomY - startAt;
        float noteBottom = noteTop + length;
        bool contact = noteBottom > bottomY;
        // printf("start=%f,bottom=%f\n", startAt,noteBottom);
        if (noteBottom < 0 || noteTop > bottomY) {
            continue;
        }

        if (contact) {
            length = bottomY - noteTop;
        }

        double notePos = board.getPos(touch.startMessage.note);

        context.set_color(canvas_ity::brush_type::fill_style, .5, .6, .7, .5);
        context.fill_rectangle(notePos - 1, noteTop, board.widthWhite - 2, length);

        if (contact) {
            context.set_shadow_blur(5);
            context.set_shadow_color(1,1,1,1);
            context.set_color(canvas_ity::brush_type::fill_style, .9, .9, .9, .5);
            context.fill_rectangle(notePos - 1, bottomY - 20, board.widthWhite - 2, 20);
            context.set_shadow_blur(0);

            for (int i=0;i<10;i++) {
                particles->emitAt(Vec(notePos,bottomY-20),Vec(vRandom()*4-2, -vRandom()*10));
            }
        }
    }

    char name[255] = {};
    sprintf(name, "../files/out-%d.png", frame);

    auto *image = new unsigned char[height * width * 4];
    context.get_image_data(image, width, height, width * 4, 0, 0);
    stbi_write_png(name, width, height, 4, image, width * 4);
    delete[] image;

}

int main() {
    std::srand(std::time({})); 

    static int const width = 1200, height = 600;
    canvas_ity::canvas context(width, height);

    //    File font;
    //    font.load("../data/Candara.ttf");
    //    bool result = context.set_font(font.data, font.size, 15);
    //    printf("Result=%d\n",result);
    //    context.fill_text("Test", 30, 30);


    // read file and parse
    v::MidiData md;
    v::RawFile rawFile;
    rawFile.init("../data/example.raw", v::RawFile::READ);
    md.parse(rawFile);
    rawFile.close();


    Particles particles;
    Board board(10, 1180);
    board.buildPos();

    for (int i=0;i<2*25;i++) {
        printf("Frame=%d   \r", i);
        context.set_color(canvas_ity::brush_type::fill_style, 0, 0, 0, 1);
        context.fill_rectangle(0, 0, width, height);
        draw_board(board, context, 600 - 110, 100);
        render_frame(&particles, &md, board, context, width, height, i);
    }
    //
    const char * cmd = "ffmpeg -y -framerate 25 -i ../files/out-%d.png -c:v libx264 -pix_fmt yuv420p ../files/out.mp4";
    exec(cmd);
    printf("done\n");
    return 0;
}