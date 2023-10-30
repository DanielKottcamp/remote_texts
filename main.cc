#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <curl/curl.h>

using namespace std;
int main(void){
    string phoneNumber = "1234567890"; // Replace this with the phone number
    string message = "Test message"; // Replace this with the message content
    string apiKey = "your_api_key_here"; // Replace this with your Textbelt API key
    performCurlPostCommand(phoneNumber, message, apiKey);
}

size_t writeCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void performCurlPostCommand(const string& var1, const string& var2, const string& var3) {
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        string postData = "phone=" + var1 + "&message=" + var2;
        string url = "https://textbelt.com/text";
        string keyData = "key=" + var3;

        string fullCommand = url + " --data-urlencode " + postData + " -d " + keyData;

        curl_easy_setopt(curl, CURLOPT_URL, fullCommand.c_str());

        string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        } else {
            cout << "Response:\n" << response << endl;
        }

        curl_easy_cleanup(curl);
    }
}
