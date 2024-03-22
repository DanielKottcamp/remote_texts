#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <curl/curl.h>
#include <wiringPi.h>
#include <algorithm>
#include <cctype>

using namespace std;

string apiKey;
string phoneNums[9];
string textMess[9];
string emergencyMessage;
vector<string> emergencyNums;

// GPIO pins, currently all temp and only 1 pin per function
#define PhoneNum1 17   // assign GPIO pin 17 to first phone button
#define MessageNum1 27 // GPIO pin 27 to first message button
#define sendButton 22  // GPIO pin 22 to send message button

using namespace std;
size_t writeCallback(void *contents, size_t size, size_t nmemb, string *userp);
void sendText(string phoneNum, string message, string key);
void sendText(string phoneNum, string message);
void sendTextTest(string phoneNum, string message);
void readConfigFile();
string trimSpaces(const string &str);
void manualConfig();
void buttonLoop();

int main(void)
{
    string phoneNumber = "6095751848";
    string message = "Test message from the code rather than command line";
    if (wiringPiSetupGpio() == -1)
    { // error out if issue with wiringpi library
        std::cerr << "Failed to initialize WiringPi library" << std::endl;
        return 1;
    }
    //readConfigFile();
    manualConfig();


    sendText(phoneNums[0], textMess[0]);
    buttonLoop();
}

void manualConfig(){ //alternate to readConfigFile used in testing
    apiKey = "a2785a58de916127bd7bf54ae260dfa406e54486XMpDe4vFZFb6cbkaOejR10eeb";
    phoneNums[0] = "6095751848";
    textMess[0] = "Generic test text message";
}

void pinSetup()
{
    pinMode(PhoneNum1, INPUT);
    pinMode(MessageNum1, INPUT);
    pinMode(sendButton, INPUT);
}

void buttonLoop()
{ // break off main loop into it's own function to better facilitate debugging
    bool NumsUsed[9] = {false};
    uint8_t message;
    while (true)
    {
        int phoneNum1state = digitalRead(PhoneNum1);
        int message1state = digitalRead(MessageNum1);
        int sendstate = digitalRead(sendButton);

        if (phoneNum1state == LOW)
        {
            NumsUsed[0] = !NumsUsed[0];
            cout << "Phone number button pressed, currently" << NumsUsed[0];
        }
        if (message1state == LOW)
        {
            message = 1;
            cout << "Message button pressed";
        }
        if (sendstate == LOW && message >= 0 && message < 10)
        {
            cout << "send button pressed, currently";
            for (int i = 0; i < 9; i++)
            {
                if (NumsUsed[i])
                {
                    sendText(phoneNums[i], textMess[message]);
                }
            }
            message = 100;
            for (int i = 0; i < 9; ++i)
            {
                NumsUsed[i] = false;
            }
        }
        delay(150);
    }
}

size_t writeCallback(void *contents, size_t size, size_t nmemb, string *userp)
{
    userp->append((char *)contents, size * nmemb);
    return size * nmemb;
}

void sendText(string phoneNum, string message, string key)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl)
    {
        string encodedPhone = curl_easy_escape(curl, phoneNum.c_str(), phoneNum.length());
        string encodedMessage = curl_easy_escape(curl, message.c_str(), message.length());

        string postData = "phone=" + encodedPhone + "&message=" + encodedMessage + "&key=" + key;
        string url = "https://textbelt.com/text";
        //string keyData = "key=" + key;

        //string args = /*url + " --data-urlencode " + */ postData + " -d " + keyData; // formats data for curl command

        // curl_easy_setopt(curl, CURLOPT_URL, fullCommand.c_str()); //sends curl command

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());

        string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response); // retrieves response

        cout << "Curl Sent:\n"
                 << postData << endl;

        res = curl_easy_perform(curl); // performs operation

        if (res != CURLE_OK)
        { // doesn't yet really do much with response, might do something later like warn if quota gets low or something like that
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }
        else
        {
            cout << "Response:\n"
                 << response << endl;
        }

        curl_easy_cleanup(curl); // cleanup
    }
}

void sendText(string phoneNum, string message)
{ // overload function, if key not included then use global key variable
    sendText(phoneNum, message, apiKey);
}

void sendTextTest(string phoneNum, string message)
{ // overload function uses free test key, limited use
    sendText(phoneNum, message, "textbelt");
}

void readConfigFile()
{
    ifstream config("config.ini"); // open file
    if (!config.is_open())
    {
        return;
    }
    string line;
    while (getline(config, line))
    {
        size_t deliminator = line.find("=");
        if (deliminator == line.npos)
        {
            continue; // if no =, something went wrong. skip this line
        }
        string type = trimSpaces(line.substr(0, deliminator)); // get what value is being set in this line
        line = line.substr(deliminator + 1);                   // remove from line
        string val = trimSpaces(line);                         // get rest of line as value being set
        if (type == "API")
        {
            apiKey = val;
        }
        else if (type.substr(0, type.length() - 2) == "PHONE")
        {                                                    // phone numbers
            int num = type.c_str()[type.length() - 2] - '0'; // convert number after phone to integer
            phoneNums[num] = val;                           // add to vector
        }
        else if (type.substr(0, type.length() - 2) == "MESSAGE")
        {                                                    // messages numbers
            int num = type.c_str()[type.length() - 2] - '0'; // convert number after message to integer
            textMess[num] = val;                            // add to vector
        }
        else if (type.substr(0, type.length() - 2) == "EMERGENCY")
        {
            emergencyMessage = val;
        }
        else if (type.substr(0, type.length() - 2) == "EMERGENCYSEND")
        {
            deliminator = line.find(",");
            while (deliminator != line.npos)
            {
                string num = trimSpaces(line.substr(0, deliminator)); // issolate first num
                emergencyNums.push_back(num);                         // push num into vector
                line = line.substr(deliminator + 1);                  // remove from line
            }
        }
    }
    config.close();
}

string trimSpaces(const string &str) {
    string trimmed;
    for (char c : str) {
        if (!isspace(c)) {
            trimmed.push_back(c);
        }
    }
    return trimmed;
}