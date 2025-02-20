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

void render_frame(v::MidiData *md, Board &board, canvas_ity::canvas &context, int width, int height, int frame) {

    int ratio = 25;
    float t = static_cast<float>(frame) / static_cast<float>(ratio);


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
//        printf("start=%f,bottom=%f\n", startAt,noteBottom);
        if (noteTop < 0 || noteTop > bottomY) {
            continue;
        }

        if (noteBottom > bottomY) {
            length = bottomY - noteTop;
        }

        double notePos = board.getPos(touch.startMessage.note);

        context.set_color(canvas_ity::brush_type::fill_style, .5, .6, .7, 1);
        context.fill_rectangle(notePos - 1, noteTop, board.widthWhite - 2, length);
    }

    char name[255] = {};
    sprintf(name, "../files/out-%d.png", frame);

    auto *image = new unsigned char[height * width * 4];
    context.get_image_data(image, width, height, width * 4, 0, 0);
    stbi_write_png(name, width, height, 4, image, width * 4);
    delete[] image;

}

int main() {

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
    rawFile.init("out.raw", v::RawFile::READ);
    md.parse(rawFile);
    rawFile.close();


    Board board(10, 1180);
    board.buildPos();

    for (int i=0;i<2*25;i++) {
        printf("Frame=%d   \r", i);
        context.set_color(canvas_ity::brush_type::fill_style, 0, 0, 0, 1);
        context.fill_rectangle(0, 0, width, height);
        draw_board(board, context, 600 - 110, 100);
        render_frame(&md, board, context, width, height, i);
    }
// -c:v libx264 -pix_fmt yuv420p
    const char * cmd = "C:\\Users\\shop\\Downloads\\ffmpeg-7.1-full_build\\ffmpeg-7.1-full_build\\bin\\ffmpeg.exe -y -framerate 25 -i ../files/out-%d.png out.mp4";
    exec(cmd);
    printf("done\n");
    return 0;
}