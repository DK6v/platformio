#pragma once

#include <memory>
#include <string>

#include "ESP8266WebServer.h"

#include "server/view/View.h"

namespace app {

class Controller;

class Route {
public:
    Route();
    ~Route() = default;

    void setController(Controller *controller_p);

    template <typename T>
    Route& setView(const T& view);

    void handleRequest();

private:
    Controller *mController_p;

    std::shared_ptr<View> mView_p;
};

template <typename T>
Route& Route::setView(const T &view) {

    mView_p = std::shared_ptr<T>(new T(std::move(view)));
    return *this;
}



} // namespace
