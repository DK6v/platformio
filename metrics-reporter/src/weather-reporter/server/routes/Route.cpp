#include "Route.h"
#include "server/Controller.h"

#include "Console.h"

using namespace app;

Route::Route()
    : mController_p(nullptr),
      mView_p(nullptr) {}

template <>
Route& Route::setView(const std::shared_ptr<View> &view_p) {

    console.log("set view 0x%x", view_p);
    mView_p = view_p;

    return *this;
}

void Route::setController(Controller *controller_p) {

    console.log("set controller 0x%x", controller_p);
    mController_p = controller_p;
}

void Route::handleRequest() {

    console.log("handleRequest 0x%x", mController_p);

    if (mView_p != nullptr) {
        console.log("view is not null");
        mController_p->sendResponse(200, mView_p->onRequest());
    }
    else {

    std::string postForms = "<!DOCTYPE html>\
<html>\
	<body>\
        None!\
	</body>\
</html>";

        mController_p->sendResponse(200, postForms);
    }
}
