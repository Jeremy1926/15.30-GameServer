#pragma once

#define CURL_STATICLIB
#include "curl/curl.h"
#include <algorithm>

#include "Options.h"
#pragma comment(lib, "curl/libcurl.lib")


using namespace std;

static string MMS_URL = "https://backend-services-prod.privateuser.xyz";

static std::string SendRequestBody(const std::string& url, const std::string& method, const std::string& jsonBody = "") {
    CURL* curl;
    CURLcode res;
    long response_code;
    std::string response_data;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](void* ptr, size_t size, size_t nmemb, std::string* data) -> size_t {
            data->append(static_cast<char*>(ptr), size * nmemb);
            return size * nmemb;
            });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            std::cout << method << " Response code: " << response_code << std::endl;
        }

        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "Failed to initialize curl handle." << std::endl;
    }

    return response_data;
}

static std::string SendRequestBodyV2(const std::string& url, const std::string& method, const std::string& jsonBody, long* outResponseCode = nullptr) {
    CURL* curl;
    CURLcode res;
    long response_code = 0;
    std::string response_data;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](void* ptr, size_t size, size_t nmemb, std::string* data) -> size_t {
            data->append(static_cast<char*>(ptr), size * nmemb);
            return size * nmemb;
            });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            std::cout << method << " Response code: " << response_code << std::endl;
        }

        if (outResponseCode) {
            *outResponseCode = response_code;
        }

        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "Failed to initialize curl handle." << std::endl;
    }

    return response_data;
}

static std::mutex mtx;
static std::condition_variable cv;
static std::atomic<int> ongoingRequests = 0;
constexpr int maxRequests = 5;

namespace Backend
{
    static map<string, string> UserAuthTicketMap;
    static map<string, int> UserHypeMap;

    static void UploadPlrKills(int kills, const std::string& username, bool victory, AFortPlayerControllerAthena* PC) {
        auto it = UserAuthTicketMap.find(username);
        if (it == UserAuthTicketMap.end()) {
            std::printf("Username '%s' not found in UserAuthTicketMap.\n", username.c_str());
            return;
        }

        int XpEarned = PC->XPComponent->TotalXpEarned;

        auto it_hype = UserHypeMap.find(username);
        if (it_hype == UserHypeMap.end())
            return;

        int HypeEarned = it_hype->second;

        if (!bArena)
            HypeEarned = 0;

        const std::string& authTicket = it->second;
        std::string jsonString = "{\n";
        jsonString += "  \"kills\": " + std::to_string(kills) + ",\n";
        jsonString += "  \"hype\": " + std::to_string(HypeEarned) + ",\n";
        jsonString += "  \"xp\": " + std::to_string(XpEarned) + ",\n";
        jsonString += "  \"auth_ticket\": \"" + authTicket + "\",\n";
        jsonString += "  \"victory\": " + std::string(victory ? "true" : "false") + "\n";
        jsonString += "}\n";

        std::thread([jsonString]() {
            SendRequestBody(MMS_URL + "/api/v2/dedicated/profile_update", "POST", jsonString);
        }).detach();
    }

    static void VerifyToken(const std::string& Token, APlayerController* PC) {
        std::string jsonString = "{ \"auth_ticket\": \"" + Token + "\" }";

        std::thread([jsonString, PC]() {

            long response_code = 0;
            SendRequestBodyV2(MMS_URL + "/api/v2/dedicated/verify", "POST", jsonString, &response_code);

            if (response_code != 200 && PC) {
                Log(L"Returning to mainmenu!")
                PC->ClientReturnToMainMenu(L"");
            }

        }).detach();
    }
}