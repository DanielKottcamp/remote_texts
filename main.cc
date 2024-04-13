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
string phoneNums[3];
string textMess[3];
string emergencyMessage;
vector<string> emergencyNums;

// GPIO pins for phone numbers
#define PhoneNum1 9  // GPIO pin 9 for first phone button
#define PhoneNum2 10   // GPIO pin 10 for second phone button
#define PhoneNum3 11  // GPIO pin 11 for third phone button

// GPIO pins for message buttons
#define MessageNum1 5  // GPIO pin 5 for first message button
#define MessageNum2 6    // GPIO pin 6 for second message button
#define MessageNum3 13    // GPIO pin 13 for third message button

#define sendButton 22  // GPIO pin 22 for send message button

//LED GPIO pins
#define Output1 19  // GPIO pin 19 for LED 1
#define Output2 26  // GPIO pin 26 for LED 2
#define Output3 21  // GPIO pin 21 for LED 3
#define Output4 20  // GPIO pin 20 for LED 4
#define Output5 16  // GPIO pin 16 for LED 5
#define Output6 12  // GPIO pin 12 for LED 6

using namespace std;
size_t writeCallback(void *contents, size_t size, size_t nmemb, string *userp);
void sendText(string phoneNum, string message, string key);
void sendText(string phoneNum, string message);
void sendTextTest(string phoneNum, string message);
void readConfigFile();
string trimSpaces(const string &str);
void manualConfig();
void buttonLoop();
void pinSetup();
bool toggleLEDOutput(int pin);

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
    pinSetup();
    buttonLoop();
}

void manualConfig(){ //alternate to readConfigFile used in testing
    apiKey = "a2785a58de916127bd7bf54ae260dfa406e54486XMpDe4vFZFb6cbkaOejR10eeb";
    phoneNums[0] = "6095751848";
    phoneNums[1] = "7172016926";
    textMess[0] = "Generic test text message 1";
    textMess[1] = "Second Generic test text message";
    textMess[2] = "Third test message for project";
}

void pinSetup()
{
    // Set up phone number buttons
    pinMode(PhoneNum1, INPUT);
    pinMode(PhoneNum2, INPUT);
    pinMode(PhoneNum3, INPUT);
    pullUpDnControl(PhoneNum1, PUD_UP);
    pullUpDnControl(PhoneNum2, PUD_UP);
    pullUpDnControl(PhoneNum3, PUD_UP);

    // Set up message buttons
    pinMode(MessageNum1, INPUT);
    pinMode(MessageNum2, INPUT);
    pinMode(MessageNum3, INPUT);
    pullUpDnControl(MessageNum1, PUD_UP);
    pullUpDnControl(MessageNum2, PUD_UP);
    pullUpDnControl(MessageNum3, PUD_UP);

    // Set up send message button
    pinMode(sendButton, INPUT);
    pullUpDnControl(sendButton, PUD_UP);

    // Set up output pins
    pinMode(Output1, OUTPUT);
    pinMode(Output2, OUTPUT);
    pinMode(Output3, OUTPUT);
    pinMode(Output4, OUTPUT);
    pinMode(Output5, OUTPUT);
    pinMode(Output6, OUTPUT);
}


bool toggleLEDOutput(int pin)
{
    static bool state = false;
    state = !state;
    digitalWrite(pin, state ? HIGH : LOW);
    return state;
}


void buttonLoop()
{ // break off main loop into it's own function to better facilitate debugging
    bool NumsUsed[3] = {false};
    uint8_t message;
    int outputPins[] = {Output1, Output2, Output3, Output4, Output5, Output6};
    bool outputsActive[6] = {false};


    while (true)
    {
        int phoneNum1state = digitalRead(PhoneNum1);
        int phoneNum2state = digitalRead(PhoneNum2);
        int phoneNum3state = digitalRead(PhoneNum3);
        int message1state = digitalRead(MessageNum1);
        int message2state = digitalRead(MessageNum2);
        int message3state = digitalRead(MessageNum3);
        int sendstate = digitalRead(sendButton);

         if (phoneNum1state == LOW)
        {
            NumsUsed[0] = !NumsUsed[0];
            outputsActive[0] = toggleLEDOutput(Output1);
            cout << "Phone number 1 button pressed, currently " << NumsUsed[0] << endl;
        }
        if (phoneNum2state == LOW)
        {
            NumsUsed[1] = !NumsUsed[1];
            outputsActive[1] = toggleLEDOutput(Output2);
            cout << "Phone number 2 button pressed, currently " << NumsUsed[1] << endl;
        }
        if (phoneNum3state == LOW)
        {
            NumsUsed[2] = !NumsUsed[2];
            outputsActive[2] = toggleLEDOutput(Output3);
            cout << "Phone number 3 button pressed, currently " << NumsUsed[2] << endl;
        }
        if (message1state == LOW)
        {
            message = 0;
            outputsActive[3] = toggleLEDOutput(Output4);
            if(outputsActive[4]){
                outputsActive[4] = toggleLEDOutput(Output5);
            }
            if(outputsActive[5]){
                outputsActive[5] = toggleLEDOutput(Output6);
            }
            cout << "Message button 1 pressed" << endl;
        }
        if (message2state == LOW)
        {
            message = 1;
            outputsActive[4] = toggleLEDOutput(Output5);
            if(outputsActive[3]){
                outputsActive[3] = toggleLEDOutput(Output4);
            }
            if(outputsActive[5]){
                outputsActive[5] = toggleLEDOutput(Output6);
            }
            cout << "Message button 2 pressed" << endl;
        }
        if (message3state == LOW)
        {
            message = 2;
            outputsActive[5] = toggleLEDOutput(Output6);
            if(outputsActive[4]){
                outputsActive[4] = toggleLEDOutput(Output5);
            }
            if(outputsActive[3]){
                outputsActive[3] = toggleLEDOutput(Output4);
            }
            cout << "Message button 3 pressed" << endl;
        }
        if (sendstate == LOW && message >= 0 && message < 3)
        {
            cout << "send button pressed, currently";
            //turn of LEDs as no numbers or messages will be selected once message sent
            for (int i = 0; i < 6; ++i)
            {
                if (outputsActive[i])
                {
                    outputsActive[i] = toggleLEDOutput(outputPins[i]);
                }
            }

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