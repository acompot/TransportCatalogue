#pragma once

#include "json.h"

namespace json {

  class DictItemContext;
  class BaseContext;
  class Builder;
  class ArrayItemContext;
  class KeyContext;
  class ValueKeyItemContext;
  class ValueArrItemContext;

  class Builder {
    private:
      std::vector<Node *> array_nodes_;
      bool wBuild = false;
      Node jroot_;
      friend BaseContext;

    public:
      Builder() =default;

      Node Build() ;

      BaseContext Value(json::Node node);

      KeyContext Key(const std::string& key);

      DictItemContext StartDict() ;

      ArrayItemContext StartArray();

      BaseContext EndArray();

      BaseContext EndDict();
   };

  class BaseContext {
    protected:
      Builder& builder;
    public:
       BaseContext(Builder &b) :builder(b){}

       KeyContext Key(const std::string& key);

       DictItemContext StartDict() ;

       BaseContext EndDict();

       Node Build();

       ArrayItemContext StartArray();

       BaseContext EndArray();

       BaseContext Value(json::Node node);
  };

  class DictItemContext : public BaseContext {
    public:
      DictItemContext(json::BaseContext b) :BaseContext(b){}

      Node Build() =delete;

      ArrayItemContext StartArray()  =delete;

      ArrayItemContext EndArray()  =delete;

      BaseContext Value(json::Node node)  =delete;

      DictItemContext StartDict() =delete;
  };

  class ArrayItemContext : public BaseContext {
    public:
      ArrayItemContext(json::BaseContext b) :BaseContext(b){}

      ValueArrItemContext Value(Node node);

      KeyContext Key(const std::string& key)=delete;

      DictItemContext EndDict() =delete;

      Node Build()=delete;
  };

  class KeyContext : public BaseContext {
    public:
      KeyContext(json::BaseContext b) :BaseContext(b){}

      ValueKeyItemContext Value(Node node);

      DictItemContext EndDict()=delete;

      Node Build()=delete;

      ArrayItemContext EndArray()=delete;

      KeyContext Key(const std::string& key) =delete;
  };

  //@ Spasibo Starinets Anton
  class ValueKeyItemContext : public BaseContext {
    public:
      ValueKeyItemContext(json::BaseContext b) :BaseContext(b){}

      ArrayItemContext EndArray()=delete;

      Node Build()=delete;

      BaseContext Value(Node node) =delete;

      DictItemContext StartDict()=delete;

      ArrayItemContext StartArray()=delete;
  };

  class ValueArrItemContext : public BaseContext {
    public:
      ValueArrItemContext(json::BaseContext b) :BaseContext(b){}

      ArrayItemContext Value(Node node);

      KeyContext Key(const std::string& key) = delete;
  };
}//namespace json
