// Created by Tivins on 19/02/2025.

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb_image_write.h"

#define CANVAS_ITY_IMPLEMENTATION
#include "libs/canvas_ity/canvas_ity.hpp"

#include "libs/fmt-11.1.3/include/fmt/format.h"

#include "src/Midic.h"
#include "src/Util.h"
#include "src/Effects.h"
#include <string>
#include <ctime>
#include <iostream>

using namespace v;

class RasterConfig {
public:
    Col white_up;
    Col white_down;
    Col black_up;
    Col black_down;
    int frameRate = 25;
    int videoWidth = 800;
    int videoHeight = 600;
    unsigned int seed = 0;
    std::string title = "";
    std::string author = "";
};

static RasterConfig config;

void initConf() {
    config.white_up.set(.8,.8,.8);
    config.white_down.set(1,1,1);
    config.black_up.set(.2,.2,.2);
    config.black_down.set(.4,.4,.4);
    config.frameRate = 25;
    config.videoWidth = 1280; // 1920
    config.videoHeight = 720; // 1080
    config.seed = std::time({});
    config.title = "Title";
    config.author = "Author";
}


void draw_board(Board &board, canvas_ity::canvas &context, float y, float height, std::vector<int> & down) {

    for (uint8_t note = Board::note_lowest; note <= Board::note_highest; note++) {
        if (Board::isWhite(note)) {
            bool isDown = std::find(down.begin(), down.end(), note) != down.end();
            Col c = isDown ? config.white_down : config.white_up;
            float pos = board.getPos(note) - 1;
            context.set_color(canvas_ity::brush_type::fill_style, c.r, c.g, c.b, c.a);
            context.fill_rectangle(pos, y, board.widthWhite - 2, height);
        }
    }
    for (uint8_t note = Board::note_lowest; note <= Board::note_highest; note++) {
        if (!Board::isWhite(note)) {
            bool isDown = std::find(down.begin(), down.end(), note) != down.end();
            Col c = isDown ? config.black_down : config.black_up;
            float baseWidth = (board.widthWhite*.6f);
            float offset = (board.widthWhite*.2f);

            context.set_color(canvas_ity::brush_type::fill_style, 0, 0, 0, 1);
            context.fill_rectangle(board.getPos(note) + offset - 1 - 2, y, baseWidth - 2 + 4, (float) height / 1.5f + 2);

            context.set_color(canvas_ity::brush_type::fill_style, c.r, c.g, c.b, c.a);
            context.fill_rectangle(board.getPos(note) + offset - 1, y, baseWidth - 2, (float) height / 1.5f);
        }
    }
}

std::vector<int> render_frame(Particles *particles, MidiData *md, Board &board, canvas_ity::canvas &context, int width, int height,
             int frame) {

    int ratio = config.frameRate;
    float t = static_cast<float>(frame) / static_cast<float>(ratio);
    std::vector<int> notesDown;


    particles->update();
    for (const auto& particle: particles->elements) {
        if (particle.opacity < 0.1) {
            continue;
        }
        context.set_shadow_blur(5 - Util::rand());
        context.set_shadow_color(.5, .7, .9, particle.opacity);
        context.set_color(canvas_ity::brush_type::fill_style, .9, .9, .9, particle.opacity);
        context.fill_rectangle(particle.pos.x - 1, particle.pos.y - 1, 2, 2);
    }
    context.set_shadow_blur(0);


    float bottomY = static_cast<float>(height) - 110 - 10;

    context.set_color(canvas_ity::brush_type::stroke_style, .5, .6, .7, 1);
    context.move_to(0, bottomY);
    context.line_to(width, bottomY);
    context.stroke();

    // read part
    float stretch = 200;
    for (const auto &touch: md->touches) {
        float startAt = (float) (touch.startMessage.getSeconds() - t) * stretch;
        float length = ((float) touch.length / Message::seconds) * stretch;
        float noteTop = bottomY - startAt;
        float noteBottom = noteTop + length;
        bool contact = noteBottom > bottomY && noteTop < bottomY;

        if (noteBottom < 0) {
            // notes are stored in timestamp order. So, if a note is too far, we can stop to render.
            break;
        }
        if (noteTop > bottomY) {
            continue;
        }

        if (contact) {
            length = bottomY - noteTop;
            notesDown.push_back(touch.startMessage.note);
        }

        auto notePos = board.getPos(touch.startMessage.note);

        context.set_color(canvas_ity::brush_type::fill_style, .5, .6, .7, .5);
        context.fill_rectangle(notePos - 1, noteTop, board.widthWhite - 2, length);

        if (contact) {
            context.set_shadow_blur(5);
            context.set_shadow_color(1, 1, 1, 1);
            context.set_color(canvas_ity::brush_type::fill_style, .9, .9, .9, .5);

            // context.move_to(notePos - 1, bottomY);
            // context.line_to(notePos - 1 + board.widthWhite - 2, bottomY);
            // context.line_to(notePos - 1 + (board.widthWhite - 2) / 2, bottomY - 20);
            // context.close_path();
            // context.fill();

            context.fill_rectangle(notePos - 1, bottomY - 20, board.widthWhite - 2, 20);
            context.set_shadow_blur(0);

            for (int i = 0; i < 5; i++) {
                particles->emitAt(Vec(notePos - 1 + (board.widthWhite - 2) / 2, bottomY),
                                  Vec(Util::rand() * 4 - 2, -Util::rand() * 10));
            }
        }
    }
    return notesDown;
}

void saveImage(canvas_ity::canvas &context, const char * name) {
    auto *image = new unsigned char[config.videoHeight * config.videoWidth * 4];
    context.get_image_data(image, config.videoWidth, config.videoHeight, config.videoWidth * 4, 0, 0);
    stbi_write_png(name, config.videoWidth, config.videoHeight, 4, image, config.videoWidth * 4);
    delete[] image;
}

int main() {


    initConf();

    std::srand(config.seed);
    static int const width = 1280, height = 720; // 1920x1080
    canvas_ity::canvas context(config.videoWidth, config.videoHeight);

    File font;
    try {
        font.load("../data/Candara.ttf");
    }
    catch(std::exception&e) {
        std::cerr << fmt::format("Error: {}\n", e.what());
    }


    // read file and parse
    MidiData md;
    RawFile rawFile;
    rawFile.init("../data/example.raw", RawFile::READ);
    md.parse(rawFile);
    rawFile.close();


    Particles particles;
    Board board(10, config.videoWidth - 20);
    board.buildPos();

    // const int frameRate = 25;
    char name[255] = {};

    int is = 0; int ie = 7 * config.frameRate;
    for (int i = is; i < ie; i++) {
        printf("Frame=%d   \r", i);
        context.set_color(canvas_ity::brush_type::fill_style, 0, 0, 0, 1);
        context.fill_rectangle(0, 0, width, height);
        context.set_color(canvas_ity::brush_type::fill_style, 1, 1, 1, 1);
        context.set_font(font.data, font.size, 25);
        context.fill_text("Title", 30, 40);
        context.set_color(canvas_ity::brush_type::fill_style, .8, .8, .8, 1);
        context.set_font(font.data, font.size, 20);
        context.fill_text("Author", 30, 40 + 20);
        auto downNotes = render_frame(&particles, &md, board, context, width, height, i);
        draw_board(board, context, height - 110, 100, downNotes);

        sprintf(name, "../files/out-%d.png", i);
        saveImage(context, name);
    }

    const char * mp4Filename = "../files/out.mp4";
    
    char cmd[512] = {};
    sprintf(cmd,
            "ffmpeg -y -framerate %d -i ../files/out-%%d.png -c:v libx264 -pix_fmt yuv420p %s",
            config.frameRate,
            mp4Filename);

    const std::string result = Util::exec(cmd);
    printf("Done (%s). Video saved %s\n", result.c_str(), mp4Filename);
    return 0;
}