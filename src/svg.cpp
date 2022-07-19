#include "svg.h"



namespace svg {

using namespace std::literals;

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
  out << "r=\""sv << radius_ << "\" "sv;
  RenderAttrs(out);
  out << "/>"sv;
}

void Circle::Render(const RenderContext& context) const {
  Object::Render(context);
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point){
  points_poly.push_back(std::move(point));
  return *this;
}

void Polyline::Render(const RenderContext& context) const {
  Object::Render(context);
}


void Polyline::RenderObject(const RenderContext& context) const {
  auto& out = context.out;
  bool f = false;
  out << "<polyline points=\""sv;
  for (auto point : points_poly){
    if (f == false){
      f = true;
    }
    else {
      out<<" "sv;
    }
    out<<point.x <<","sv<<point.y;
  }
  out << "\""sv;
  RenderAttrs(out);
  out << "/>"sv;
}

// ---------- Text ------------------


Text& Text::SetPosition(Point pos){
  position_ = pos;
  return *this;
}

Text& Text::SetOffset(Point offset){
  offset_ = offset;
  return *this;
}

Text& Text::SetFontSize(uint32_t size){
  size_font_ = size;
  return *this;
}

Text& Text::SetFontFamily(std::string font_family){
  font_name_ = font_family;
  return *this;
}

Text& Text::SetFontWeight(std::string font_weight){
  font_bold_ = font_weight;
  return *this;
}

std::string Text::EncodingForXML(std::string text){
  const std::list<std::pair<std::string,std::string>> spec_symbols
  {
        {"&","&amp;"},
        {"\"","&quot;"},
        {"\'","&apos;"},
        {"<","&lt;"},
        {">","&gt;"}

  };

  for (auto [from,to]: spec_symbols){
    size_t pos = 0;
    while (std::string::npos != (pos = text.find_first_of(from, pos)))
    {
      text.replace(pos, 1, to);
      pos++;
    }
  }
  return text;
}

Text& Text::SetData(std::string data){
  label_ = EncodingForXML(data);
  return *this;
}

void Text::RenderObject(const RenderContext& context) const {
  auto& out = context.out;
  out << "<text";
  RenderAttrs(out);
  out<< " x=\""sv  << position_.x << "\" y=\""sv << position_.y << "\" "sv;
  out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
  out <<" "sv << "font-size=\""sv << size_font_  << "\""sv;
  if(!font_name_.empty()){
    out << " font-family=\""sv << font_name_ << "\""sv;
  }
  if (!font_bold_.empty()){
     out << " font-weight=\""sv << font_bold_  ;
     out <<  "\""sv;
  }
  out <<  ">"sv;
  out << label_;
  out << "</text>"sv;
}

void Text::Render(const RenderContext& context) const {
  Object::Render(context);
}

void Document::Render(std::ostream& out) const
{
  RenderContext ctx(out);
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
  out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
  std::list<std::unique_ptr<Object>>::const_iterator iterator = figures_.cbegin();
  while(iterator != figures_.cend()){
    out <<"  "sv;
    (*iterator)->Render(ctx);
     ++iterator;
  }
  out << "</svg>"sv;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {figures_.emplace_back(std::move(obj));}
}  // namespace svg
