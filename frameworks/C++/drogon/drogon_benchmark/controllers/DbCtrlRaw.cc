#include "DbCtrlRaw.h"
#include "models/World.h"
#include <drogon/utils/Utilities.h>
#include <stdlib.h>
#include <thread>
#include <time.h>
using namespace drogon_model::hello_world;

void DbCtrlRaw::asyncHandleHttpRequest(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    // write your application logic here
    static std::once_flag once;
    std::call_once(once, []() { srand(time(NULL)); });
    if (!*_dbClient)
    {
        *_dbClient = drogon::app().getFastDbClient();
    }
    int id = rand() % 10000 + 1;
    auto callbackPtr =
        std::make_shared<std::function<void(const HttpResponsePtr &)>>(
            std::move(callback));

    **_dbClient << "select * from world where id=$1" << id >>
        [callbackPtr, id](const Result &rows) {
            auto resp = HttpResponse::newHttpResponse();
            char json[64];
            auto size = sprintf(json,
                                "{\"id\":%d,\"randomnumber\":%s}",
                                id,
                                rows[0][1ul].c_str());
            resp->setBody(std::string(json, size));
            resp->setContentTypeCode(CT_APPLICATION_JSON);
            (*callbackPtr)(resp);
        } >>
        [callbackPtr](const DrogonDbException &err) {
            Json::Value json{};
            json["code"] = 0;
            json["message"] = err.base().what();
            (*callbackPtr)(HttpResponse::newHttpJsonResponse(json));
        };
}
