#pragma once

#include <string>
#include <memory>
#include <map>

#include "ESP8266WebServer.h"

#include "routes/Route.h"

namespace app {

class Controller {
public:
    Controller(ESP8266WebServer *server_p);
    ~Controller() = default;

    void handleRequest();

    Controller& addRouteHandler(std::string path,
                                std::shared_ptr<Route> route_p);

    void sendResponse(unsigned int code, const std::string &response); 

private:
    ESP8266WebServer *mServer_p;

    std::map<std::string, std::shared_ptr<Route>> mRoutes;
};

} // namespace
