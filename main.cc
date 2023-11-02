#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <curl/curl.h>


//string apiKey;

using namespace std;
size_t writeCallback(void* contents, size_t size, size_t nmemb, string* userp);
void sentText(const string& phoneNum, const string& message, const string& key);
void sendText(const string& phoneNum, const string& message);
void sendTextTest(const string& phoneNum, const string& message);

int main(void){
    string phoneNumber = "6095751848"; 
    string message = "Test message from the code rather than command line"; 
    //string apiKey = "your_api_key_here"; 
    sendTextTest(phoneNumber, message);
}

size_t writeCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void sendText(const string& phoneNum, const string& message, const string& key) {
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        string postData = "phone=" + phoneNum + "&message=" + message;
        string url = "https://textbelt.com/text";
        string keyData = "key=" + key;

        string fullCommand = url + " --data-urlencode " + postData + " -d " + keyData; //formats data for curl command

        curl_easy_setopt(curl, CURLOPT_URL, fullCommand.c_str()); //sends curl command

        string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response); //retrieves response

        res = curl_easy_perform(curl); //performs operation

        if (res != CURLE_OK) { //doesn't yet really do much with response, might do something later like warn if quota gets low or something like that
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        } else {
            cout << "Response:\n" << response << endl;
        }

        curl_easy_cleanup(curl); //cleanup
    }
}

void sendText(const string& phoneNum, const string& message) { //overload function, if key not included then use global key variable
   // sendText(phoneNum, message, apiKey); 
}

void sendTextTest(const string& phoneNum, const string& message) { //overload function uses free test key, limited use
    sendText(phoneNum, message, "textbelt"); 
}
