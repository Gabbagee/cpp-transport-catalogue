#include "svg.h"

namespace svg {

using namespace std;
using namespace std::literals;

ostream& operator<<(ostream& out, StrokeLineCap line_cap) {
    switch (line_cap) {
        case StrokeLineCap::BUTT: out << "butt"sv; break;
        case StrokeLineCap::ROUND: out << "round"sv; break;
        case StrokeLineCap::SQUARE: out << "square"sv; break;
    }
    return out;
}

ostream& operator<<(ostream& out, StrokeLineJoin line_join) {
    switch (line_join) {
        case StrokeLineJoin::ARCS: out << "arcs"sv; break;
        case StrokeLineJoin::BEVEL: out << "bevel"sv; break;
        case StrokeLineJoin::MITER: out << "miter"sv; break;
        case StrokeLineJoin::MITER_CLIP: out << "miter-clip"sv; break;
        case StrokeLineJoin::ROUND: out << "round"sv; break;
    }
    return out;
}

namespace {

    void RenderColor(ostream& out, monostate) {
        out << "none"sv;
    }

    void RenderColor(ostream& out, const string& value) {
        out << value;
    }

    void RenderColor(ostream& out, Rgb rgb) {
        out << "rgb("sv << static_cast<int>(rgb.red)
            << ',' << static_cast<int>(rgb.green)
            << ',' << static_cast<int>(rgb.blue) << ')';
    }

    void RenderColor(ostream& out, Rgba rgba) {
        out << "rgba("sv << static_cast<int>(rgba.red)
            << ',' << static_cast<int>(rgba.green)
            << ',' << static_cast<int>(rgba.blue)
            << ',' << rgba.opacity << ')';
    }

}  // namespace

ostream& operator<<(ostream& out, const Color& color) {
    visit([&out](const auto& value) {
        RenderColor(out, value);
    }, color);
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << endl;
}

void Document::AddPtr(unique_ptr<Object>&& obj) {
    objects_.emplace_back(move(obj));
}

void Document::Render(ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    RenderContext context(out, 2, 2);
    for (const auto& obj : objects_) {
        obj->Render(context);
    }
    out << "</svg>"sv;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.emplace_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool first = true;
    for (const Point& point : points_) {
        if (!first) {
            out << " "sv;
        }
        first = false;
        out << point.x << ","sv << point.y;
    }
    out << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text -----------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(string font_family) {
    font_family_ = move(font_family);
    return *this;
}

Text& Text::SetFontWeight(string font_weight) {
    font_weight_ = move(font_weight);
    return *this;
}

Text& Text::SetData(string data) {
    data_ = move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << font_size_ << "\""sv;
    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    RenderAttrs(context.out);
    out << ">"sv;
    for (char c : data_) {
        switch (c) {
            case '<': out << "&lt;"sv; break;
            case '>': out << "&gt;"sv; break;
            case '"': out << "&quot;"sv; break;
            case '&': out << "&amp;"sv; break;
            case '\'': out << "&apos;"sv; break;
            default: out << c;
        }
    }
    out << "</text>"sv;
}

} // namespace svg