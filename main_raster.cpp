// Created by Tivins on 19/02/2025.

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb_image_write.h"

#define CANVAS_ITY_IMPLEMENTATION
#include "libs/canvas_ity/canvas_ity.hpp"

#include "src/Midic.h"
#include "src/Util.h"
#include "src/Effects.h"
#include <string>
#include <ctime>

using namespace v;



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
            context.fill_rectangle(board.getPos(note) + (board.widthWhite*.15f) - 1, y, (board.widthWhite*.7f) - 2, (float) height / 1.5f);
        }
    }
}



void
render_frame(Particles *particles, MidiData *md, Board &board, canvas_ity::canvas &context, int width, int height,
             int frame, int ratio = 25) {

    float t = static_cast<float>(frame) / static_cast<float>(ratio);


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

    char name[255] = {};
    sprintf(name, "../files/out-%d.png", frame);

    auto *image = new unsigned char[height * width * 4];
    context.get_image_data(image, width, height, width * 4, 0, 0);
    stbi_write_png(name, width, height, 4, image, width * 4);
    delete[] image;

}

int main() {
    std::srand(std::time({}));
    static int const width = 1280, height = 720; // 1920x1080
    canvas_ity::canvas context(width, height);

    File font;
    font.load("../data/Candara.ttf");


    // read file and parse
    MidiData md;
    RawFile rawFile;
    rawFile.init("../data/example.raw", RawFile::READ);
    md.parse(rawFile);
    rawFile.close();


    Particles particles;
    Board board(10, width - 20);
    board.buildPos();

    const int frameRate = 25;

    int is = 0; int ie = 7 * frameRate;
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
        draw_board(board, context, height - 110, 100);
        render_frame(&particles, &md, board, context, width, height, i, frameRate);
    }

    char cmd[512] = {};
    sprintf(cmd,
            "ffmpeg -y -framerate %d -i ../files/out-%%d.png -c:v libx264 -pix_fmt yuv420p %s",
            frameRate,
            "../files/out.mp4");

    const std::string result = Util::exec(cmd);
    printf("done\n");
    return 0;
}