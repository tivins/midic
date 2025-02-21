//
// Created by shop on 20/02/2025.
//

#ifndef MIDIC_UTIL_H
#define MIDIC_UTIL_H

#include <string>
#include <memory>
#include <array>

namespace v {
    class Col {
    public:
        float r{}, g{}, b{}, a{};
        Col() = default;
        Col(float r, float g, float b, float a = 1): r(r),g(g),b(b),a(a) {}
        void set(float _r, float _g, float _b, float _a = 1) {
            r = _r;
            g=_g;
            b=_b;
            a=_a;
        }
    };
    class Vec {
    public:
        float x{}, y{};

        Vec() = default;

        Vec(float _x, float _y) {
            x = _x;
            y = _y;
        }

        Vec(const Vec &p) {
            x = p.x;
            y = p.y;
        }
    };

    class Util {
    public:
        static std::string exec(const char *cmd) {
            char buffer[128];
            std::string result;
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
            if (!pipe) {
                throw std::runtime_error("popen() failed!");
            }
            while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
                result += buffer;
            }
            return result;
        }

        static float rand() {
            return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        }
    };

    class File {
    public:
        uint8_t *data = nullptr;
        size_t size = 0;

        ~File() {
            delete[] data;
        }

        void load(const char *filename) {
            FILE *fp = fopen(filename, "rb");
            if (!fp) {
                throw std::runtime_error("cannot load file");
            }
            fseek(fp, 0, SEEK_END);
            size = ftell(fp);
            rewind(fp);
            data = new unsigned char[size]();
            fread(data, sizeof(uint8_t), size, fp);
            fclose(fp);
        }
    };
} // v

#endif //MIDIC_UTIL_H
