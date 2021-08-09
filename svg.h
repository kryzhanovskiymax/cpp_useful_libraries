#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <optional>
#include <variant>
 
namespace svg {
    
inline const std::string NoneColor{"none"};
    
struct Rgb {
    Rgb() = default;
    Rgb(uint8_t red_, uint8_t green_, uint8_t blue_);
    
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};
    
struct Rgba {
    Rgba() = default;
    Rgba(uint8_t red_, uint8_t green_,uint8_t blue_, double opacity_);
    
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

struct ColorPrinter {
    std::ostream& out;
    void operator() (std::monostate);
    void operator() (std::string color);
    void operator() (svg::Rgb color);
    void operator() (svg::Rgba color);
};
    
enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};
 
enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};
    
std::ostream& StrokeLineCapOutput(std::ostream& out, svg::StrokeLineCap cap);

std::ostream& StrokeLineJoinOutput(std::ostream& out, svg::StrokeLineJoin join);

std::ostream& operator<<(std::ostream& os, svg::StrokeLineCap cap);
    
std::ostream& operator<<(std::ostream& os, svg::StrokeLineJoin join);
 
struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};
 
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }
 
    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }
 
    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }
 
    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }
 
    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};
 
template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = std::move(width);
        return AsOwner();
    }
    
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_line_cap_ = std::move(line_cap);
        return AsOwner();
    }
    
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_line_join_ = std::move(line_join);
        return AsOwner();
    }
private:
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
    
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }
    
    void RenderFillColor(std::ostream& out) const {
        if (fill_color_) {
            out << " fill=\"";
            std::visit(ColorPrinter{out}, *fill_color_);
            out<<"\"";
        }
    }
    
    void RenderStrokeColor(std::ostream& out) const {
        if (stroke_color_) {
            out << " stroke=\"";
            std::visit(ColorPrinter{out}, *stroke_color_);
            out <<"\"";
        }
    }
    
    void RenderStrokeWidth(std::ostream& out) const {
        if (stroke_width_) {
            out << " stroke-width=\"" << *stroke_width_ << "\"";
        }
    }
    
    void RenderStrokeLineCap(std::ostream& out) const {
        if(stroke_line_cap_) {
            out << " stroke-linecap=\""<< *stroke_line_cap_<<"\"";
        }
    }
    
    void RenderStrokeLineJoin(std::ostream& out) const {
        if(stroke_line_join_) {
            out << " stroke-linejoin=\"" << *stroke_line_join_ << "\"";
        }
    }
protected:
    ~PathProps() = default;
    
    void RenderAttrs(std::ostream& out) const {
        RenderFillColor(out);
        RenderStrokeColor(out);
        RenderStrokeWidth(out);
        RenderStrokeLineCap(out);
        RenderStrokeLineJoin(out);
    }
};
 
class Object {
public:
    void Render(const RenderContext& context) const;
 
    virtual ~Object() = default;
 
private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};
 
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);
 
private:
    void RenderObject(const RenderContext& context) const override;
 
    Point center_;
    double radius_ = 1.0;
};
 
class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline& AddPoint(Point point);
 
private:
    std::vector<Point> points_;
    
    void RenderObject(const RenderContext& context) const override;
};
 
class Text final : public Object, public PathProps<Text> {
public:
    Text& SetPosition(Point pos);
 
    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);
 
    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);
 
    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);
 
    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);
 
    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);
 
private:
    Point position_;
    Point offset_;
    std::string font_family_;
    std::string data_;
    uint32_t font_size_ = 1;
    std::string font_weight_;
    
    void RenderObject(const RenderContext& context) const override;
};
    
class ObjectContainer {
public:
    template <typename T>
    void Add(T obj) {
        AddPtr(std::move(std::make_unique<T>(obj)));
    }
    
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
 
    virtual ~ObjectContainer() = default;
};
 
class Document : public ObjectContainer {
public:
    
    void AddPtr(std::unique_ptr<Object>&& obj) override;
 
    void Render(std::ostream& out) const;
 
private:
    std::vector<std::unique_ptr<Object>> objects_;
};
    
class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
 
    virtual ~Drawable() = default;
};
 
}  // namespace svg
