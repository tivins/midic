// Created by Tivins on 19/02/2025.

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../libs/stb_image_write.h"
//
#define STB_IMAGE_IMPLEMENTATION

#include "../libs/stb_image.h"
//

//
#include "yaml-cpp/yaml.h"
#include "lib/Midic.h"
#include "lib/Util.h"
#include "lib/Effects.h"
#include "lib/Display.h"
#include "../libs/cxxopts/cxxopts.hpp"
#include <string>
#include <ctime>
#include <iostream>

using namespace v;

struct Image {
    uint8_t *data = nullptr;
    SizeInt size;
    int components = 4;

    virtual ~Image() {
        delete[] data;
        size.set(0, 0);
    }

    bool load(const std::string &filename) {
        stbi_set_flip_vertically_on_load(false);
        data = stbi_load(filename.c_str(), &size.w, &size.h, &components, 0);
        // fmt::print("LoadImage w={}, h={}, c={}\n", size.w, size.h, components);
        stbi_set_flip_vertically_on_load(false);
        return true;
    }
};

static RasterConfig config;
static Image imageSmoke;

static Col parseCol(const YAML::Node &node) {
    return {node[0].as<float>(), node[1].as<float>(), node[2].as<float>(),
            node[3].IsDefined() ? node[3].as<float>() : 1};
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
    config.sub_text = "1835";
    config.buildVideo = false;
    config.mp4Filename = "../files/out.mp4";
    config.imagesPath = "../files/out-{}.png";
    config.debug = false;

    if (!filename.empty()) {
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
        if (yaml["info"]["sub_text"]) config.author = yaml["info"]["sub_text"].as<std::string>();
        if (yaml["info"]["debug"]) config.debug = yaml["info"]["debug"].as<bool>();
    }
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


    //    context.draw_image(
    //            imageSmoke.data,
    //            imageSmoke.size.w,
    //            imageSmoke.size.h,
    //            imageSmoke.size.w * imageSmoke.components,
    //            100, 100, 256, 256);
    //    context.draw_image(
    //            imageSmoke.data,
    //            imageSmoke.size.w,
    //            imageSmoke.size.h,
    //            imageSmoke.size.w * imageSmoke.components,
    //            200, 100, 256, 256);

    for (const auto &particle: particles->elements) {
        context.set_shadow_blur(5 - Util::pseudoRandom());
        context.set_shadow_color(.5, .7, .9, particle.opacity);
        context.set_color(canvas_ity::brush_type::fill_style, .9, .9, .9, particle.opacity);
        context.fill_rectangle(particle.pos.x - 1, particle.pos.y - 1, 2, 2);
    }
    context.set_shadow_blur(0);


    float bottomY = static_cast<float>(height) - 110 - 10;

    context.set_color(canvas_ity::brush_type::stroke_style, .5, .6, .7, 1);
    context.move_to(0, bottomY);
    context.line_to((float) width, bottomY);
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
            // TODO: if a note is finished, it's not rendered.
            // break;
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
        context.set_shadow_color(.5, .6, .7, 1);
        context.set_shadow_blur(2);
        context.set_color(canvas_ity::brush_type::fill_style, .5, .6, .7, (float) touch.startMessage.velocity / 127.0f);
        // context.fill_rectangle(notePos - 1, noteTop, board.widthWhite - 2, length);
        v::Display::roundRect(context, notePos - 1, noteTop, board.widthWhite - 2, length, 5);
        context.fill();
        context.set_shadow_blur(0);
        context.set_color(canvas_ity::brush_type::stroke_style, .5 + .2, .6 + .2, .7 + .2,
                          (float) touch.startMessage.velocity / 127.0f);
        context.stroke();

        if (contact) {
            float highlight_height = length > 20 ? 20 : length;
            float highlight_top = bottomY - highlight_height;

            context.set_shadow_blur(5);
            context.set_shadow_color(1, 1, 1, 1);
            context.set_color(canvas_ity::brush_type::fill_style, .9, .9, .9, .5);
            context.fill_rectangle(notePos - 1, highlight_top, board.widthWhite - 2, highlight_height);
            context.set_shadow_blur(0);


            context.set_shadow_blur(20);
            context.set_shadow_color(1, 1, 1, 1);
            context.set_color(canvas_ity::brush_type::fill_style, .9, .9, .9, .5);
            context.fill_rectangle(notePos - 1, bottomY - 2, board.widthWhite - 2, 2);
            context.set_shadow_blur(0);


            for (int i = 0; i < 10; i++) {
                particles->emitAt(Vec(notePos - 1 + (board.widthWhite - 2) / 2, bottomY),
                                  Vec(Util::pseudoRandom() * 4 - 2, -Util::pseudoRandom() * 10));
            }
        }
    }
    return notesDown;
}

void saveImage(canvas_ity::canvas &context, const std::string &name) {
    auto *image = new unsigned char[(int) config.videoSize.w * (int) config.videoSize.h * 4];
    context.get_image_data(image, (int) config.videoSize.w, (int) config.videoSize.h, (int) config.videoSize.w * 4, 0,
                           0);
    stbi_write_png(name.c_str(), (int) config.videoSize.w, (int) config.videoSize.h, 4, image,
                   (int) config.videoSize.w * 4);
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
    canvas_ity::canvas context((int) config.videoSize.w, (int) config.videoSize.h);

    imageSmoke.load("../data/smoke.png");

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


    for (int i = 0; i < config.numFrames; i++) {
        std::cout << "Rendering frame " << (i + 1) << "/" << config.numFrames << "\r" << std::flush;
        particles.update();

        // clear canvas
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
        context.fill_text(config.sub_text.c_str(), 30, 40 + 25 + 20);

        // debug
        if (config.debug) {
            context.set_color(canvas_ity::brush_type::fill_style, 1, 1, 1, 1);
            context.set_font(font.data, (int) font.size, 12);
            context.fill_text(fmt::format("F:{}, P:{}", i, particles.elements.size()).c_str(), 20,
                              config.videoSize.h - 140);
        }

        // render notes and board
        auto downNotes = render_frame(&particles, &md, board, context, (int) config.videoSize.w,
                                      (int) config.videoSize.h, i);
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

    cxxopts::Options options("midic_raster", "Convert MIDI recording to images/video");
    // options.custom_help("custom help");
    options.positional_help("source");
    options.add_options()
            // ("d,debug", "Enable debugging") // a bool parameter
            // ("i,integer", "Int param", cxxopts::value<int>())
            ("s,source", "Raw Source file name", cxxopts::value<std::string>())("c,config", "Configuration file name",
                                                                                cxxopts::value<std::string>())(
            "v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))("h,help", "Print usage");

    options.parse_positional({"source"});
    try {
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            exit(0);
        }
        // for (const auto &i: result.arguments()) {
        //     fmt::print("{} => {}\n", i.key(), i.value());
        // }
        if (!result.count("source")) {
            throw std::runtime_error("'source' is not defined.");
        }
        rawFilename = result["source"].as<std::string>();
        if (result.count("config")) {
            configFilename = result["config"].as<std::string>();
        }

        fmt::print("Input: {}\n", rawFilename);
        fmt::print("Configuration: {}\n", configFilename);
        initConf(configFilename);
        config.print();
    } catch (std::exception &e) {
        fmt::print(stderr,"ERROR: {}\n", e.what());
        exit(0);
    }
    process(rawFilename);
    return 0;
}