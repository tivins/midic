//
// Created by Tivins on 23/02/2025.
//

#ifndef MIDIC_DISPLAY_H
#define MIDIC_DISPLAY_H
#define CANVAS_ITY_IMPLEMENTATION
#include "../../libs/canvas_ity/canvas_ity.hpp"

namespace v {

    class Display {
    public:
        static void roundRect(canvas_ity::canvas &context, float x, float y, float width, float height, float radius) {
            context.begin_path();
            context.move_to(x + radius, y);
            context.line_to(x + width - radius, y);
            context.arc_to(x + width, y, x + width, y + radius, radius);
            context.line_to(x + width, y + height - radius);
            context.arc_to(x + width, y + height, x + width - radius, y + height, radius);
            context.line_to(x + radius, y + height);
            context.arc_to(x, y + height, x, y + height - radius, radius);
            context.line_to(x, y + radius);
            context.arc_to(x, y, x + radius, y, radius);
            context.close_path();

        }
    };

} // v

#endif //MIDIC_DISPLAY_H
