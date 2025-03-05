//
// Created by shop on 20/02/2025.
//

#ifndef MIDIC_UTIL_H
#define MIDIC_UTIL_H

#include <string>
#include <memory>
#include <array>

#ifdef _WIN32
#include <cstdio>
#define popen _popen
#define pclose _pclose
#else
#include <unistd.h>
#endif

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
        Vec(float x, float y):x(x),y(y) { }
        Vec(const Vec &p):x(p.x),y(p.y) { }
    };

    template<typename T>
    class SizeType {
    public:
        T w{}, h{};
        SizeType() = default;
        SizeType(T w, T h) : w(w), h(h) { }
        SizeType(const SizeType<T> &p) : w(p.w), h(p.h) { }
        void set(T _w, T _h) { w=_w; h=_h; }
    };
    typedef SizeType<float> Size;
    typedef SizeType<int> SizeInt;


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

        /**
         * Predictable random value
         */
        static float pseudoRandom() {
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

        bool load(const char *filename) {
            FILE *fp = fopen(filename, "rb");
            if (!fp) {
                // throw std::runtime_error("cannot load file");
                return false;
            }
            fseek(fp, 0, SEEK_END);
            size = ftell(fp);
            rewind(fp);
            data = new unsigned char[size]();
            fread(data, sizeof(uint8_t), size, fp);
            fclose(fp);
            return true;
        }
    };
} // v

#endif //MIDIC_UTIL_H
