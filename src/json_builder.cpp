#include "json_builder.h"

namespace json {

  BaseContext Builder::Value(json::Node node){
    if (array_nodes_.empty()){
      jroot_ = node;
      array_nodes_.emplace_back(&jroot_);
      wBuild = true;
    }else{
      if (wBuild == false){
        auto n  = array_nodes_.back();
        if (n->IsNull()){
          *n = node;
          array_nodes_.pop_back();
        }
        else {
          if (std::holds_alternative<Array>((array_nodes_.back()->GetValue()))){
            Array& z = std::get<Array>(array_nodes_.back()->GetValue());
            z.emplace_back(node);
          } else throw std::logic_error("Bad position to Value");
        }
      } else throw std::logic_error("Wrong position to Value");
    }
    return BaseContext(*this);
  }

  Node Builder::Build() {
    if (!array_nodes_.empty()) {
      if (wBuild ==true)  return *array_nodes_.back();
      else  throw std::logic_error("Wrong return");
    }else  throw std::logic_error("Nothing to return");
  }

  KeyContext Builder::Key(const std::string& key){
    if (std::holds_alternative<Dict>((array_nodes_.back()->GetValue())) && wBuild !=true){
      Dict& z = std::get<Dict>(array_nodes_.back()->GetValue());
      array_nodes_.emplace_back(&z[move(key)]);
    } else throw std::logic_error("Wrong pos to key");
    return KeyContext(*this);
  }

  DictItemContext Builder::StartDict() {
    if (array_nodes_.empty()) array_nodes_.emplace_back(new Node(Dict()));
    else{
      auto n  = array_nodes_.back();
      if (n->IsNull() || n->IsArray())  array_nodes_.emplace_back(new Node(Dict()));
      else   throw std::logic_error("U cant create Dict");
    }
    return DictItemContext(*this);
  }

  ArrayItemContext Builder::StartArray(){
    if (array_nodes_.empty()) array_nodes_.emplace_back(new Node(Array()));
    else{
      auto n  = array_nodes_.back();
      if (n->IsNull() || n->IsArray())  array_nodes_.emplace_back(new Node(Array()));
      else   throw std::logic_error("U cant create Array");
    }
    return ArrayItemContext(*this);
  }

  BaseContext Builder::EndArray(){
    if (std::holds_alternative<Array>((array_nodes_.back()->GetValue()))){
      Array& array = std::get<Array>(array_nodes_.back()->GetValue());//Zakrivaem
      array_nodes_.pop_back();
      if (array_nodes_.empty()) {
        jroot_ = array;
        array_nodes_.emplace_back(&jroot_);
        wBuild = true;
      }
      else {
        auto n  = array_nodes_.back();//Kyda cladem
        if (n->IsNull()){
          *n = array;
          array_nodes_.pop_back();
        } else {
          if (std::holds_alternative<Array>((array_nodes_.back()->GetValue()))) {
            Array& narr = std::get<Array>(array_nodes_.back()->GetValue());
            narr.push_back(array);
          }else if (std::holds_alternative<Dict>((array_nodes_.back()->GetValue()))) {
            Dict& smalld = std::get<Dict>(array_nodes_.back()->GetValue());
            smalld.rbegin()->second= array;
          }else throw std::logic_error("Some another");
       }
      }
    } else  throw std::logic_error("Not array in EndArray");
    return BaseContext(*this);
}

  BaseContext Builder::EndDict(){
    if (std::holds_alternative<Dict>((array_nodes_.back()->GetValue()))){
      Dict& dic = std::get<Dict>(array_nodes_.back()->GetValue());//Zakrivaem
      array_nodes_.pop_back();
      if (array_nodes_.empty()) {
        jroot_ = dic;
        array_nodes_.emplace_back(&jroot_);
        wBuild = true;
      }
      else {
        auto n  = array_nodes_.back();//Poslednij element
        if (n->IsNull()){
          *n = dic;
          array_nodes_.pop_back();
        } else {
          if (std::holds_alternative<Array>((array_nodes_.back()->GetValue()))) {
            Array& arr = std::get<Array>(array_nodes_.back()->GetValue());
            arr.emplace_back(dic);
          }else  if (std::holds_alternative<Dict>((array_nodes_.back()->GetValue()))) {
            Dict& smalld = std::get<Dict>(array_nodes_.back()->GetValue());
            smalld.rbegin()->second= dic;
          } else  throw std::logic_error("Some another");
        }
      }
    }
    else throw std::logic_error("Not dict in EndDict");
    return BaseContext(*this);
}

  BaseContext BaseContext::Value(json::Node node){return  builder.Value(node);}

  Node BaseContext::Build(){return builder.Build();}

  KeyContext BaseContext::Key(const std::string &key){return KeyContext(builder.Key(key));}

  DictItemContext  BaseContext::StartDict(){return DictItemContext(builder.StartDict());}

  ArrayItemContext BaseContext::StartArray(){return ArrayItemContext(builder.StartArray());}

  BaseContext BaseContext::EndArray(){return builder.EndArray();}

  BaseContext BaseContext::EndDict(){return builder.EndDict();}

  ValueKeyItemContext KeyContext::Value(Node node){return  static_cast<ValueKeyItemContext>(builder.Value(node));}

  ValueArrItemContext ArrayItemContext::Value(Node node){return static_cast<ValueArrItemContext>(builder.Value(node));}

  ArrayItemContext ValueArrItemContext::Value(Node node){return static_cast<ArrayItemContext>(builder.Value(node));}

}//namespace json
