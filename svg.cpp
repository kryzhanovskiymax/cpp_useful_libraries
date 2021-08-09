#include "svg_package.hpp"

namespace svg {
 
using namespace std::literals;
    
void ColorPrinter::operator() (std::monostate) {
        out << "none"sv;
    }
    
void ColorPrinter::operator() (std::string color) {
    out << color;
}
    
void ColorPrinter::operator() (svg::Rgb color) {
    out << "rgb("sv << static_cast<int>(color.red) <<","sv<< static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) <<")"sv;
}
    
void ColorPrinter::operator() (svg::Rgba color) {
    out << "rgba("sv << static_cast<int>(color.red) <<","sv<< static_cast<int>(color.green) << ","sv << static_cast<int>(color.blue) << ","sv << color.opacity <<")"sv;
}
    
Rgb::Rgb(uint8_t red_, uint8_t green_, uint8_t blue_) :
                red(red_), green(green_), blue(blue_) {}
    
Rgba::Rgba(uint8_t red_, uint8_t green_,uint8_t blue_, double opacity_) :
                        red(red_), green(green_), blue(blue_), opacity(opacity_) {}
    
std::ostream& StrokeLineCapOutput(std::ostream& out, svg::StrokeLineCap cap) {
    if(cap == svg::StrokeLineCap::BUTT) {
        out << "butt";
    }
           
    if(cap == svg::StrokeLineCap::SQUARE) {
        out << "square";
    }
            
    if(cap == svg::StrokeLineCap::ROUND) {
        out << "round";
    }
    
    return out;
}

std::ostream& StrokeLineJoinOutput(std::ostream& out, svg::StrokeLineJoin join) {
    if(join == svg::StrokeLineJoin::ARCS) {
        out << "arcs";
    }
            
    if(join == svg::StrokeLineJoin::BEVEL) {
        out << "bevel";
    }
            
    if(join == svg::StrokeLineJoin::MITER) {
        out << "miter";
    }
            
    if(join == svg::StrokeLineJoin::MITER_CLIP) {
        out << "miter-clip";
    }
            
    if(join == svg::StrokeLineJoin::ROUND) {
        out << "round";
    }
    
    return out;
}

std::ostream& operator<<(std::ostream& os, svg::StrokeLineCap cap) {
    StrokeLineCapOutput(os, cap);
    return os;
}
    
std::ostream& operator<<(std::ostream& os, svg::StrokeLineJoin join) {
    StrokeLineJoinOutput(os, join);
    return os;
}
    
void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
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
    RenderAttrs(out);
    out << "/>"sv;
}
    
    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }
    
    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out<<"<polyline points=\""sv;
        for(size_t i = 0; i < points_.size(); ++i) {
            out<<points_[i].x<<","sv<<points_[i].y;
            if(i != points_.size() - 1) {
                out<<" "sv;
            }
        }
        out<<"\"";
        RenderAttrs(out);
        out<<"/>"sv;
    }
    
    Text& Text::SetPosition(Point pos) {
        position_ = pos;
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
 
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }
 
    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }
 
    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }
    
    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out<<"<text";
        RenderAttrs(out);
        out << " x=\""<<position_.x<<"\" y=\""<<position_.y<<"\" dx=\""<<offset_.x<<"\" dy=\""<<offset_.y<<"\" font-size=\""<<font_size_<<"\"";
        if(!font_family_.empty()) {
            out<<" font-family=\""<<font_family_<<"\"";
        }
        if(!font_weight_.empty()) {
            out<<" font-weight=\""<<font_weight_<<"\"";
        }
        out << ">"<<data_<<"</text>";
    }
    
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }
 
    void Document::Render(std::ostream& out) const {
        
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
 
        for(const auto& object : objects_) {
            object->Render(out);
        }
 
        out << "</svg>"sv;
        
        
    }
 
}
