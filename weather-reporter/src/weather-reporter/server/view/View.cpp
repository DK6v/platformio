#include "View.h"

#include "Console.h"

using namespace app;

std::string CompositeView::onRequest() {

    std::string response;

    console.log("CompositeView::onRequest()");

    for (auto &view_p: mViews) {

        response += view_p->onRequest();
    }

    return response;
}

std::string HtmlView::onRequest() {

    std::string response;

    console.log("HtmlView");
   
    response += "<!DOCTYPE html>";
    response += "<html>";

    response += CompositeView::onRequest();

    response += "</html>";

    return response;
}

std::string BodyView::onRequest() {

    std::string response;

    console.log("BodyView");

    response.append("<body>");

    response.append(
        "<h1>POST form data to /postform/</h1>\
		<br/>\
		<form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
			<input type=\"text\" name=\"hello1\" value=\"1 2 3\"/><br/>\
            <input type=\"text\" name=\"hello2\" value=\"1 2 3\"/><br/>\
			<input type=\"submit\" value=\"Submit\"/>\
		</form>");

    response.append("</body>");

    return response;
}

std::string TextInputView::onRequest() {

    std::string response;

    console.log("TextInputView");

    response.append(
		"<br/>\
		<form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
			<input type=\"text\" name=\"hello1\" value=\"1 2 3\"/><br/>\
		</form>");

    return response;
}


// return "<!DOCTYPE html>\
// <html>\
//   <head>\
//     <title>ESP8266 Web Server</title>\
//     <style>\
//       body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
//     </style>\
//   </head>\
// 	<body>\
// 		<h1>POST form data to /postform/</h1>\
// 		<br/>\
// 		<form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
// 			<input type=\"text\" name=\"hello1\" value=\"1 2 3\"/><br/>\
//             <input type=\"text\" name=\"hello2\" value=\"1 2 3\"/><br/>\
// 			<input type=\"submit\" value=\"Submit\"/>\
// 		</form>\
// 	</body>\
// </html>";
