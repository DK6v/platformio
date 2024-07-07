#pragma once

#include <list>
#include <string>
#include <memory>

#include "Console.h"

namespace app {

class View {
public:
    virtual std::string onRequest() {
        console.log("View::onRequest() => none");
        return "none";
    };
};


class BodyView;

class CompositeView : public View {
public:

    CompositeView() {};
    ~CompositeView() {};

//    template <typename T>
//    CompositeView& add(const T& view) = delete;

    template <typename T>
    CompositeView& add(T view) {

        mViews.push_back(std::shared_ptr<T>(new T(std::move(view))));
        return *this;
    };

//    template <typename T>
//    CompositeView& add(const std::shared_ptr<T> &view_p) {
//        
//        console.log("CompositeView::add(shared_ptr) 0x%x", view_p.get());
//        mViews.push_back(view_p);
//
//        for(const auto &view_p : mViews) {
//            console.log("CompositeView::mViews => 0x%x", view_p.get());
//        }
//
//        return *this;
//    }

    virtual std::string onRequest();
    
protected:
    std::list<std::shared_ptr<View>> mViews;
};

class HtmlView : public CompositeView {
public:
    virtual std::string onRequest();
};

class BodyView : public CompositeView {
public:
    virtual std::string onRequest();
};

class TextInputView : public CompositeView {
public:
    virtual std::string onRequest();
};

template <typename T>
class ViewCompose {
public:

    ViewCompose() {
        _pImpl = std::shared_ptr<T>(new T());
    }

    ViewCompose(auto args, ...) {
        _pImpl = std::shared_ptr<T>(new T("A"));
    }
    ~ViewCompose() = default;

private:
    std::shared_ptr<T> _pImpl;
};

class A {
public:
    A() : _name("none") {};
    A(std::string name) : _name(name) {};
private:
    std::string _name;
};

} // namespace