#include <IRremote.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#define IR_RECEIVE_PIN 22
#define IR_SEND_PIN 4
#define ENABLE_LED_FEEDBACK  true
#define USE_DEFAULT_FEEDBACK_LED_PIN true
#define SENDING_REPEATS 0

#define ENABLE_LED_FEEDBACK true
#define USE_DEFAULT_FEEDBACK_LED_PIN true

#define BUTTON_PIN 26

const int user_input_size = 100;
char user_input[user_input_size];
int char_changing = 0;

//if true connects to an existing network 
//if false creates a new hotspot
const boolean connect_to_existing = false;

//the credentials of the new hotspot(if applicable)
const char *new_ssid = "ESP32 Remote";
const char *new_password = "arduinos are cool";

//the credentials of the existing network(if applicable)
const char *existing_ssid = "";
const char *existing_password = "";

WiFiServer server(80);

//Sets up a static IP address making your URL contant 
//which saving you the hassle of finding the IP address
//everytime you want to connect

// Set your Static IP address
IPAddress local_IP(192, 168, 8, 179);
// Set your Gateway IP address
IPAddress gateway(192, 168, 8, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8); // optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional

const int buffer_size = 100;
char recieved_codes[buffer_size];
char str[50];
char code_word[10] = "command:";
unsigned long code;
int char_index = 0;

//YOUR CODE USES 40KHZ BUT IRREMOTE EXAMPLES USE 38.
//USE WHATEVER WORKS FOR YOU
const uint8_t NEC_KHZ = 38; // 38kHz carrier frequency for the NEC protocol

const uint16_t button1Signal[] = {4380,4372,536,1604,540,532,540,1600,540,532,540,532,540,532,536,536,536,1604,536
,1608,536,532,540,532,536,536,536,532,540,532,540,1604,536,532,540,532,540,1604,536,1604
,540,532,536,536,540,532,536,532,540,532,540,1604,536,1604,536,1604,540,1604,540,1600,540
,1604,536,1604,540,1604,536,1604,540,1604,536,1604,536,1608,536,1604,536,1608,536,1604
,536,1608,536,532,536,1604,540,1604,536,536,536,1604,536,1604,540,532,536,536,536,5184
,4376,4372,536,536,536,1604,536,536,536,1604,536,1608,536,1604,536,1604,540,532,536,536
,536,1604,536,1608,536,1604,536,1608,536,1604,536,536,536,1604,536,1604,540,532,536,536
,536,1604,536,1608,536,1604,536,1604,540,1604,536,536,536,532,536,536,536,532,540,532
,540,532,536,536,536,532,540,532,536,532,540,532,540,532,536,536,536,532,540,532,540
,532,536,1604,540,532,536,532,540,1604,536,536,536,532,540,1604,536,1604,540
}; // Using exact NEC timing

const uint16_t button2Signal[] = {4380,4372,536,1604,540,532,540,1600,540,532,540,532,540,532,536,536,536,1604,536
,1608,536,532,540,532,536,536,536,532,540,532,540,1604,536,532,540,532,540,1604,536,1604
,540,532,536,536,540,532,536,532,540,532,540,1604,536,1604,536,1604,540,1604,540,1600,540
,1604,536,1604,540,1604,536,1604,540,1604,536,1604,536,1608,536,1604,536,1608,536,1604
,536,1608,536,532,536,1604,540,1604,536,536,536,1604,536,1604,540,532,536,536,536,5184
,4376,4372,536,536,536,1604,536,536,536,1604,536,1608,536,1604,536,1604,540,532,536,536
,536,1604,536,1608,536,1604,536,1608,536,1604,536,536,536,1604,536,1604,540,532,536,536
,536,1604,536,1608,536,1604,536,1604,540,1604,536,536,536,532,536,536,536,532,540,532
,540,532,536,536,536,532,540,532,536,532,540,532,540,532,536,536,536,532,540,532,540
,532,536,1604,540,532,536,532,540,1604,536,536,536,532,540,1604,536,1604,540
}; // Using exact NEC timing

const uint16_t button3Signal[] = {4380,4372,536,1604,540,532,540,1600,540,532,540,532,540,532,536,536,536,1604,536
,1608,536,532,540,532,536,536,536,532,540,532,540,1604,536,532,540,532,540,1604,536,1604
,540,532,536,536,540,532,536,532,540,532,540,1604,536,1604,536,1604,540,1604,540,1600,540
,1604,536,1604,540,1604,536,1604,540,1604,536,1604,536,1608,536,1604,536,1608,536,1604
,536,1608,536,532,536,1604,540,1604,536,536,536,1604,536,1604,540,532,536,536,536,5184
,4376,4372,536,536,536,1604,536,536,536,1604,536,1608,536,1604,536,1604,540,532,536,536
,536,1604,536,1608,536,1604,536,1608,536,1604,536,536,536,1604,536,1604,540,532,536,536
,536,1604,536,1608,536,1604,536,1604,540,1604,536,536,536,532,536,536,536,532,540,532
,540,532,536,536,536,532,540,532,536,532,540,532,540,532,536,536,536,532,540,532,540
,532,536,1604,540,532,536,532,540,1604,536,536,536,532,540,1604,536,1604,540
}; // Using exact NEC timing

//ADD MORE _____SIGNAL VARIABLES HERE FOR EACH BUTTON


/*Sets up program. Connects to wifi or creates a hotspot*/
void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  IPAddress myIP;
  
  if(connect_to_existing){
    // Configures static IP address
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("STA Failed to configure");
    }
    
    WiFi.begin(existing_ssid, existing_password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    myIP = WiFi.localIP();
  }else{
    WiFi.softAP(new_ssid, new_password);
    myIP = WiFi.softAPIP();
  }
  
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();
  
  Serial.println("Server started");
  
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);
  IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK); // Specify send pin and enable feedback LED at default feedback LED pin

  Serial.print("Ready to send IR signals at pin ");
  Serial.println(IR_SEND_PIN);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

/*sends an IR code*/
void send_message(String s){
  IrReceiver.stop();
  
  if(s == "button1"){
    IrSender.sendRaw(button1Signal, sizeof(button1Signal) / sizeof(button1Signal[0]), NEC_KHZ); // Note the approach used to automatically calculate the size of the array.
  }else if(s == "button2"){
    IrSender.sendRaw(button2Signal, sizeof(button2Signal) / sizeof(button2Signal[0]), NEC_KHZ); // Note the approach used to automatically calculate the size of the array.
  }else if(s == "button3"){
    IrSender.sendRaw(button3Signal, sizeof(button3Signal) / sizeof(button3Signal[0]), NEC_KHZ); // Note the approach used to automatically calculate the size of the array.
  }//.....DUPLICATE ABOVE STATEMENTS FOR EACH BUTTON
  //...
  
  delay(1000);
  //restarts the reciever
  IrReceiver.start();
}

/*checks for messages over wifi*/
void check_wifi(){
  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            client.println(recieved_codes);
            
            for(int i = 0; recieved_codes[i] != '\0'; i++){
              recieved_codes[i] = '\0';
            }
            char_index = 0;
            // The HTTP response ends with another blank line:
            client.println();
            
            // break out of the while loop:
            break;
          } else {//reached the end of the line
            //if the line is a GET request(excluding a request for the icon)
            if(currentLine.startsWith("GET /") &&  !currentLine.startsWith("GET /favicon.ico")){
              String s = currentLine.substring(5);
              send_message(s);
            }
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

/*checks for IR codes picked up by the reciever*/
/*
void check_recieved(){
  if(IrReceiver.decode()){
    if(IrReceiver.decodedIRData.decodedRawData != 0){
      IrReceiver.printIRResultShort(&Serial);
      if(IrReceiver.decodedIRData.decodedRawData == 0xFFFFFFFF){
        //Repeat command(used by some IR remotes
        //to indicate that a button has been held
        Serial.println("...");
      }else{
        //adds the code to a char array containing all of the recieved codes
        //as the client can only initiate connections with the 
        //server(the esp32), codes are only sent to the client after it requests them
        
        Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
        
        sprintf(str,"%X",IrReceiver.decodedIRData.decodedRawData);
      
        for(int i = 0; str[i] != '\0'; i++){
          if(char_index < buffer_size){
            recieved_codes[char_index] = code_word[i];
            char_index++;
          }else{
            break;
          }
        }
      
        for(int j = 0; str[j] != '\0'; j++){
          if(char_index < buffer_size){
            recieved_codes[char_index] = str[j];
            char_index++;
          }else{
            break;
          }
        }
        
        if(char_index < buffer_size){
          recieved_codes[char_index] = '\n';
          char_index++;
        }
      }
    }
    IrReceiver.resume(); //Enable receiving of the next value
  }
}*/

/*the main loop*/
void loop() {
  check_wifi();
  /*
  //checks if the recieve button has been held
  if(digitalRead(BUTTON_PIN) == LOW){
    //and if so it checks if any IR codes have been picked up
    check_recieved();
  }*/
}
