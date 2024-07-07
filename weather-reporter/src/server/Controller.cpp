#include "Controller.h"

using namespace app;

Controller::Controller(ESP8266WebServer *server_p)
    : mServer_p(server_p) {
}

Controller& Controller::addRouteHandler(std::string url,
                                        std::shared_ptr<Route> route_p) {

    route_p->setController(this);
    mRoutes.insert(std::pair<std::string, std::shared_ptr<Route>>(url, route_p));

    mServer_p->on(url.c_str(), std::bind(&Route::handleRequest, *route_p));

    return *this;
}

void Controller::handleRequest() {

    mServer_p->handleClient();
}

void Controller::sendResponse(unsigned int code, const std::string &message) {

    mServer_p->send(code, "text/html", message.c_str());
}
