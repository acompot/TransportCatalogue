#include "map_renderer.h"

namespace maprender {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

void MapRender::CreateMap(const std::unique_ptr<transport::kernel::TransportCatalogue>& ptc,const jsonreader::JSONRender& render,std::vector<const transport::Stop*>& actual_stops
                          ,std::vector<const transport::Bus*>& get_all_buses){



  if(render.isRead){
   // auto sort_names = reader->GetSortNames();
//    reader->ClearNames();


    //Transform;
    if (!get_all_buses.empty()){
      int buses = static_cast<int>(get_all_buses.size());
      std::vector<geo::Coordinates>  cords;
      cords.reserve(1000);
      for (auto i = 0; i < buses;++i){
      //Если для маршрута есть остановки
      if (!get_all_buses[i]->stops.empty()){
        std::vector<geo::Coordinates> tmp(CreateCoordCont(get_all_buses[i]->stops));
       //Собрал все координаты в 1 месте
       cords.insert(cords.end(), tmp.cbegin(), tmp.cend());
      }
    }
      //Отсюда буду тыркать координаты
      auto SP = SphereProjector(cords.begin(),cords.end(),render.width,
                              render.height,render.padding);
      cords.clear();//Очистил что бы память не жрать.
      BusPart(get_all_buses,SP,render);

      //Время отсановок

//      auto actual_stops(tc->GetActualStops(sort_stops.begin(),sort_stops.end()));//Забрал все остановки что бы их отрисовать
      StopPart(ptc,actual_stops,SP,render);
    }
    //    Добавляем всё в svg::Document
    //    1)Линии
    //    2)Имена маршуртов
    //    3)Круги остановок
    //    4)Название остановoк
    for (const auto& poly : polylines_){
        doc.Add(poly);
    }
    for (const auto& text : texts_){
        doc.Add(text);
    }
    for (const auto& cirle : cirles_){
       doc.Add(cirle);
    }
    for (const auto& stop_name_svg : name_stops_){
       doc.Add(stop_name_svg);
    }
  }
}

void MapRender::BusPart(const std::vector<const transport::Bus *> &all_buses, const maprender::SphereProjector& sp, const jsonreader::JSONRender &rendr){
  int buses = static_cast<int>(all_buses.size());
  for (auto i = 0; i < buses;++i){
    if(!all_buses[i]->stops.empty()){
    //Начинаю рисовать
    //Примеряю цвет
    int mnog = static_cast<int>(rendr.palette.size());
    int pos = i < mnog ? i :(i - mnog*(i/mnog));
    //Резервирую  место под линии  круги и прочее
    polylines_.reserve(all_buses[i]->stops.size());
    texts_.reserve(all_buses[i]->stops.size());
    //Добавление в 1.Линии
    std::vector<svg::Point> points;
    points.resize(all_buses[i]->stops.size());
    auto lambda = [&sp](auto stop){
      return  sp(stop->coordinate_);
    };
    std::transform(all_buses[i]->stops.begin(),all_buses[i]->stops.end(),
                            points.begin(),lambda);
    polylines_.emplace_back(CreatePolyline(points,rendr.line_width,rendr.palette[pos]));
     auto lstop = all_buses[i]->pos_last_stop;
    CreateTextCircle(all_buses[i],points[0],points[lstop],pos,rendr);
      //Добавление 2.Название маршрутов (автобусов)
    }
  }
}

void MapRender::StopPart(const std::unique_ptr<transport::kernel::TransportCatalogue>& ptc,const std::vector<const transport::Stop*>& stops,const maprender::SphereProjector& sp,const jsonreader::JSONRender &rendr){
  auto tc =ptc.get();
  int stopsize = static_cast<int>(stops.size());
  for (int i=0;i< stopsize ; ++i){
    int mnog = static_cast<int>(rendr.palette.size());
    //4test
    int pos = 0;
    if (mnog > 0 )  pos = i < mnog ? i :(i - mnog*(i/mnog));
    else  pos = i;
    svg::Point point;
     auto tmp = tc->GetStopNBus(stops[i]->name_);
    if (!tmp.empty()){
      point = sp(stops[i]->coordinate_);
      cirles_.emplace_back(CreateCircle(point,rendr.stop_radius));
      name_stops_.emplace_back(CreateText(point,rendr,stops[i]->name_,true));
      name_stops_.emplace_back(CreateText(point,rendr,stops[i]->name_,rendr.palette[pos],true));
    }
  }
}

void MapRender::CreateTextCircle(const transport::Bus* bus,svg::Point& first,svg::Point& last,int position, const jsonreader::JSONRender& render){
  auto lstop = bus->pos_last_stop;
  if(bus->is_circle == true) {
  //Кольцевой маршрут
    texts_.emplace_back(CreateText(first,render,bus->name_));
    texts_.emplace_back(CreateText(first,render,bus->name_,render.palette[position]));
  } else {
  //Некольцевой маршрут
    texts_.emplace_back(CreateText(first,render,bus->name_));
    texts_.emplace_back(CreateText(first,render,bus->name_,render.palette[position]));
    if (bus->stops[0]->name_ != bus->stops[lstop]->name_ ){
      texts_.emplace_back(CreateText(last,render,bus->name_));
      texts_.emplace_back(CreateText(last,render,bus->name_,render.palette[position]));
    }
  }
}

void MapRender::Print(std::ostream& out) const{return doc.Render(out);}

void MapRender::Print() const {doc.Render(std::cout);}

std::vector<geo::Coordinates> MapRender::CreateCoordCont(const std::vector<const transport::Stop *> &stops) const{
  std::vector<geo::Coordinates> cords;
  cords.reserve(stops.size());

  for (auto& stop : stops){
      cords.emplace_back(stop->coordinate_);
  }
  return cords;
}

svg::Polyline MapRender::CreatePolyline(const std::vector<svg::Point>& points, double line_width, svg::Color color) const{
  svg::Polyline p;
  p.AddPoint(points.begin(),points.end());
  p.SetFillColor("none");
  p.SetStrokeColor(color);
  p.SetStrokeWidth(line_width);
  p.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
  p.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
  return p;
}
//По умолчанию Stop = false
svg::Text MapRender::CreateText(const svg::Point point, const jsonreader::JSONRender &rendr, const std::string& name, svg::Color color,bool isStop){
  svg::Text t;
  if (isStop ==false){
    t.SetPosition(point);
    t.SetOffset(rendr.bus_offset);
    t.SetFontSize(rendr.bus_label_font_size);
    t.SetFontFamily("Verdana");
    t.SetFontWeight("bold");
    t.SetData(name);
    t.SetFillColor(color);
  } else {
       t.SetPosition(point);
       t.SetOffset(rendr.stop_offset);
       t.SetFontSize(rendr.stop_label_font_size);
       t.SetFontFamily("Verdana");
       t.SetData(name);
       t.SetFillColor("black");
  }
  return t;

}
//По умолчанию Stop = false
svg::Text MapRender::CreateText(const svg::Point point, const jsonreader::JSONRender &rendr,const  std::string& name, bool isStop){
  svg::Text t;
  if (isStop ==false){
    t.SetPosition(point);
    t.SetOffset(rendr.bus_offset);
    t.SetFontSize(rendr.bus_label_font_size);
    t.SetFontFamily("Verdana");
    t.SetFontWeight("bold");
    t.SetData(name);
    t.SetFillColor(rendr.underlayer_color);
    t.SetStrokeColor(rendr.underlayer_color);
    t.SetStrokeWidth(rendr.underlayer_width);
    t.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    t.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
  } else {
      t.SetPosition(point);
      t.SetOffset(rendr.stop_offset);
      t.SetFontSize(rendr.stop_label_font_size);
      t.SetFontFamily("Verdana");
      t.SetData(name);
      t.SetFillColor(rendr.underlayer_color);
      t.SetStrokeColor(rendr.underlayer_color);
      t.SetStrokeWidth(rendr.underlayer_width);
      t.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
      t.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
  }
  return t;
}

svg::Circle MapRender::CreateCircle(const svg::Point point, double radius) const{
  svg::Circle c;
  c.SetFillColor("white");
  c.SetRadius(radius);
  c.SetCenter(point);
  return c;
}

}//namespace maprender
