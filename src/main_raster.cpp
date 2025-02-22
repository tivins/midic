// Created by Tivins on 19/02/2025.

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../libs/stb_image_write.h"

#define CANVAS_ITY_IMPLEMENTATION

#include "../libs/canvas_ity/canvas_ity.hpp"

#include "yaml-cpp/yaml.h"

#include "lib/Midic.h"
#include "lib/Util.h"
#include "lib/Effects.h"
#include <string>
#include <ctime>
#include <iostream>

using namespace v;

static RasterConfig config;

static Col parseCol(const YAML::Node &node) {
    return {node[0].as<float>(), node[1].as<float>(), node[2].as<float>(), node[3].IsDefined() ? node[3].as<float>() : 1};
}
static Size parseSize(const YAML::Node &node) {
    return {node[0].as<float>(), node[1].as<float>()};
}

void initConf(const std::string &filename) {
    config.white_up.set(.8, .8, .8);
    config.white_down.set(1, 1, 1);
    config.black_up.set(.2, .2, .2);
    config.black_down.set(.4, .4, .4);
    config.frameRate = 25;
    config.numFrames = 1;//7 * config.frameRate;
    config.videoSize.set(1280, 720); // 1920,1080
    config.seed = std::time({});
    config.title = "Title";
    config.author = "Author";
    config.buildVideo = false;
    config.mp4Filename = "../files/out.mp4";
    config.imagesPath = "../files/out-{}.png";

    YAML::Node yaml = YAML::LoadFile(filename);
    if (yaml["colors"]["white_up"]) config.white_up = parseCol(yaml["colors"]["white_up"]);
    if (yaml["colors"]["white_down"]) config.white_down = parseCol(yaml["colors"]["white_down"]);
    if (yaml["colors"]["black_up"]) config.black_up = parseCol(yaml["colors"]["black_up"]);
    if (yaml["colors"]["black_down"]) config.black_down = parseCol(yaml["colors"]["black_down"]);
    if (yaml["video"]["buildVideo"]) config.buildVideo = yaml["video"]["buildVideo"].as<bool>();
    if (yaml["video"]["frameRate"]) config.frameRate = yaml["video"]["frameRate"].as<int>();
    if (yaml["video"]["numFrames"]) config.numFrames = yaml["video"]["numFrames"].as<int>();
    if (yaml["video"]["size"]) config.videoSize = parseSize(yaml["video"]["size"]);
    if (yaml["info"]["title"]) config.title = yaml["info"]["title"].as<std::string>();
    if (yaml["info"]["author"]) config.author = yaml["info"]["author"].as<std::string>();

    config.print();
}


void draw_board(Board &board, canvas_ity::canvas &context, float y, float height, std::vector<int> &down) {
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
            float baseWidth = (board.widthWhite * .6f);
            float offset = (board.widthWhite * .2f);

            context.set_color(canvas_ity::brush_type::fill_style, 0, 0, 0, 1);
            context.fill_rectangle(board.getPos(note) + offset - 1 - 2, y, baseWidth - 2 + 4,
                                   (float) height / 1.5f + 2);

            context.set_color(canvas_ity::brush_type::fill_style, c.r, c.g, c.b, c.a);
            context.fill_rectangle(board.getPos(note) + offset - 1, y, baseWidth - 2, (float) height / 1.5f);
        }
    }
}

std::vector<int>
render_frame(Particles *particles, MidiData *md, Board &board, canvas_ity::canvas &context, int width, int height,
             int frame) {

    int ratio = config.frameRate;
    float t = static_cast<float>(frame) / static_cast<float>(ratio);
    std::vector<int> notesDown;


    particles->update();
    auto idx = std::begin(particles->elements);
    while (idx != std::end(particles->elements)) {
        // for (const auto& particle: particles->elements) {
        auto particle = *idx;
        if (particle.opacity < 0.1) {
            idx = particles->elements.erase(idx);
            continue;
        }
        context.set_shadow_blur(5 - Util::rand());
        context.set_shadow_color(.5, .7, .9, particle.opacity);
        context.set_color(canvas_ity::brush_type::fill_style, .9, .9, .9, particle.opacity);
        context.fill_rectangle(particle.pos.x - 1, particle.pos.y - 1, 2, 2);

        idx++;
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

        // Note block
        context.set_color(canvas_ity::brush_type::fill_style, .5, .6, .7, (float)touch.startMessage.velocity/255.0f);
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

            float highlight_height = length > 20 ? 20 : length;
            float highlight_top = bottomY - highlight_height;

            context.fill_rectangle(notePos - 1, highlight_top, board.widthWhite - 2, highlight_height);
            context.set_shadow_blur(0);

            for (int i = 0; i < 5; i++) {
                particles->emitAt(Vec(notePos - 1 + (board.widthWhite - 2) / 2, bottomY),
                                  Vec(Util::rand() * 4 - 2, -Util::rand() * 10));
            }
        }
    }
    return notesDown;
}

void saveImage(canvas_ity::canvas &context, const std::string &name) {
    auto *image = new unsigned char[(int) config.videoSize.w * (int) config.videoSize.h * 4];
    context.get_image_data(image, (int)config.videoSize.w, (int)config.videoSize.h, (int)config.videoSize.w * 4, 0, 0);
    stbi_write_png(name.c_str(), (int)config.videoSize.w, (int)config.videoSize.h, 4, image, (int)config.videoSize.w * 4);
    delete[] image;
}

void saveVideo() {
    auto cmd = fmt::format("ffmpeg -y -framerate {} -i {} -c:v libx264 -pix_fmt yuv420p {}", config.frameRate,
                           fmt::format(fmt::runtime(config.imagesPath), "%d"), config.mp4Filename);
    /* const std::string result = */ Util::exec(cmd.c_str());
    fmt::print("Video saved {}\n", config.mp4Filename);
}

void process(const std::string &rawFilename) {

    Particles particles;
    MidiData md;
    RawFile rawFile;
    File font;


    std::srand(config.seed);
    canvas_ity::canvas context(config.videoSize.w, config.videoSize.h);

    if (!font.load("../data/font.ttf")) {
        std::cerr << "Cannot load font file\n";
    }

    // read file and parse
    rawFile.init(rawFilename, RawFile::READ);
    md.parse(rawFile);
    rawFile.close();

    // build board
    Board board(10, config.videoSize.w - 20);
    board.buildPos();


    int is = 0;
    for (int i = 0; i < config.numFrames; i++) {
        std::cout << "Rendering frame " << (i + 1) << "/" << config.numFrames << "\r" << std::flush;

        context.set_color(canvas_ity::brush_type::fill_style, 0, 0, 0, 1);
        context.fill_rectangle(0, 0, config.videoSize.w, config.videoSize.h);

        // title
        context.set_color(canvas_ity::brush_type::fill_style, 1, 1, 1, 1);
        context.set_font(font.data, (int) font.size, 25);
        context.fill_text(config.title.c_str(), 30, 40);

        // author
        context.set_color(canvas_ity::brush_type::fill_style, .8, .8, .8, 1);
        context.set_font(font.data, (int) font.size, 20);
        context.fill_text(config.author.c_str(), 30, 40 + 25);

        // sub text / license / year ...
        context.set_color(canvas_ity::brush_type::fill_style, .6, .6, .6, 1);
        context.set_font(font.data, (int) font.size, 14);
        context.fill_text("arrangement John Doe", 30, 40 + 25 + 20);

        // debug
        context.set_color(canvas_ity::brush_type::fill_style, .4, .5, .6, .8);
        context.set_font(font.data, (int) font.size, 13);
        context.fill_text(fmt::format("Frame {}, Particles {}", i, particles.elements.size()).c_str(), 30,
                          300);

        // render notes and board
        auto downNotes = render_frame(&particles, &md, board, context, (int) config.videoSize.w, (int) config.videoSize.h, i);
        draw_board(board, context, config.videoSize.h - 110, 100, downNotes);

        saveImage(context, fmt::format(fmt::runtime(config.imagesPath), i));
    }
    if (config.buildVideo) {
        saveVideo();
    }
}

int main(int argc, char **argv) {
    std::string rawFilename = "../data/example.raw";
    std::string configFilename = "../config.sample.yml";
    if (argc > 1) rawFilename = argv[1];
    if (argc > 2) configFilename = argv[2];
    initConf(configFilename);
    process(rawFilename);
    return 0;
}