// Install and include it as shown below
#include <Arduino.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>
#include <IRsend.h> 
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
AsyncWebServer server(80); //  initiate the Asyncwebserver 
#include <EEPROM.h>

String ssid_names[20]; // Define ssid_names as an string type
String essid;// Define essid as an string type
String  epass;// Define epass as an string type
int Tnetwork=0;
int index_value; 
boolean wifiConnect = false; // set wificonnect variable as false initially
boolean eFlag = false; // set eFlag variable as false initially
boolean state = true;// set state variable as false initially
const char* ssid = "realme7";// provide default SSID to the noddemcu
const char* password = "krishna@";// provide the password for the ssid declared above
const char* mqtt_server = "broker.mqtt-dashboard.com"; // broker address is provided to which we have to connect

const uint16_t irLed = D2; // IR led is connected to D2 pin of nodemuc, it is declared here
IRsend irsend(irLed); // initiate D2 pin for transmission of hex code and it will set the frequency for D2 to 38KHz. On this frequency IR data is transmitted
WiFiClient espClient; // create client named as espClient
PubSubClient client(espClient); //enable hardware as MQTT client

const char* INPUT_1 = "names";// INPUT_1 variable is declared as globel variable and in that will store the details of names variable
const char* INPUT_2 = "pass";// INPUT_2 variable is declared as globel variable and in that will store the details of pass variable
// create an html page as per your requirement
// note:- in this file we have to enter the values of margin, padding, height, width, etc in 'px', otherwise it will not work
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
    <title>ESP Input Form</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
    <style>
        body {
            background-image: url("https://media.istockphoto.com/photos/futuristic-glowing-blue-wifi-symbol-on-black-dark-background-with-picture-id1297855259?b=1&k=20&m=1297855259&s=170667a&w=0&h=VPCwHBRy2-9NJX6kqNcJ19yQ_6aYJ9EnnCCBR9f2HOM=");
            height: 100%;
            background-repeat: no-repeat;
            background-size: cover;
            
            }
        .box {
            align-items: center;
            margin-left: 700px;
            margin-top: 100px;
            background:rgba(11, 34, 138, 0.527);
            color: antiquewhite;
            width: 300px;
            height: 250px;
            float: none;
            border: 2px;
            border-radius: 10px;
            border-style: solid;
            border-color: rgba(17, 16, 16, 0.678);
            padding: 10px;
            justify-content: center;
            }
        .sel
        {
            margin-left: 51px;
            /* margin-top: 30px; */
        }
        .div1
        {
           
            margin-top: 45px;
        }

        .div2
        {
            margin-left: 90px;
            margin-top: 40px;
        }

            .fa{
                cursor: pointer;
            }
    </style>
</head>
<body>
    <div class="box">
        <h2>Change your ssid and password</h2>
        <form action="/get"> 
           <div class="div1">
            <label for="ssid">SSID</label>
            
                <select name="names" class="sel">%xyz%</select>
           
            </div>
            <div>
                <label for="pass">PASSWORD</label>
                <input type="password" name="pass" id="pass" placeholder="enter password">
                <i class="fa fa-eye-slash" id="eye" aria-hidden="true"></i>
            </div>
            <div class="div2">
                <button type="submit" value="submit">submit</button>
                <button type="reset" value="reset">reset</button>
            </div>
                            
        </form>
    </div>
    <script>
        document.getElementById("eye").addEventListener("click",function()
        {
            if(document.getElementById("pass").type=="password")
            {
                document.getElementById("pass").type="text";
                this.classList.add("fa-eye");
                this.classList.remove("fa-eye-slash");
            }
            else
            {
                document.getElementById("pass").type="password";
                this.classList.remove("fa-eye");
                this.classList.add("fa-eye-slash");
            }
        });
    </script>
</body>
</html>
)rawliteral";

// function is created to update the dropdown menu
String option(const String& var){

  String s=""; 
  Serial.print("     Option function >>");// tis is done to just decorete the serial print and to print values appropriatle
  Serial.print(Tnetwork); // it will print no. of wifi available near by
  Serial.println(" network found nearby");

// the loops below will create the dragdown option of availabale SSID nearby   
if(var == "xyz"){
  for (int op=0;op<Tnetwork;op++){
    s += "<td>";
    s += "<option value='";
    s += ssid_names[op];
    s += "'>";
    s += ssid_names[op];
    s += "</option>";
    s += "</td>";
  }
  return s;
}
return String();
}
//*********************************************************

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// below function is used to do write operation in EEPROM of nodemcu
void eepromWrite(int address, String ssid, String pass){ 
  byte len = ssid.length(); // len variable will store the length of the ssid entered
  EEPROM.write(address, len);// it will write the length of the ssid in 0 address initially
//below loop is to write the ssid and password to eeprom
  for(int i=0; i<len; i++){
    EEPROM.write(address + 1 + i, ssid[i]);// this will write one character in EEPROM and will increase the address and will write next charecter of SSID
  }
  EEPROM.commit();

  EEPROM.write(len+1, pass.length());
  len += 1;
  // below loop is to write password of the network to eeprom
  for(int i=0; i<pass.length(); i++){
    EEPROM.write(len + 1 + i, pass[i]);//it will write one by one character of password same as the ssid written 
  }
  EEPROM.commit();
}

//below function is to read the data stored ineeprom
void eepromRead(int address){
  int lens = EEPROM.read(address); // store the 1st charater stored in eeprom in lens variable
  char datassid[lens + 1];

  for(int i=0; i<lens; i++){
    datassid[i] = EEPROM.read(address + 1 + i);// it will increase the address of pointer of EEPROm and will fetch the character one by one and will store it in datassid variable
  }

  datassid[lens] = '\0';
  
  lens = lens + 1;
  int lenp = EEPROM.read(lens);
  char datapass[lenp +1];
   
  for(int i=0; i<lenp; i++){
    datapass[i] = EEPROM.read(lens + 1 + i);
  }
  
  datapass[lenp] = '\0';
  essid = String(datassid);
  epass = String(datapass);
}

boolean setup_wifi() {
  boolean state = true; 
  int i = 0;
  Serial.print("       setup_wifi() >> ");
  Serial.println("Connecting to network..."); 
  WiFi.mode(WIFI_STA); 
  
  if(eFlag){// id the eFlag is true then enter to the loop
    eepromRead(0);// eepromread function is called here
    WiFi.begin(essid,  epass); // connect to the SSID and PASSWORD stored in  eeprom
  }
  //else connect to default ssid and password
  else{
    WiFi.begin(ssid, password); 
  }

 Serial.print("                       ");
  while(WiFi.status() != WL_CONNECTED){ // if wifi is not connected then will enter to this function
    
    Serial.print("."); 
    delay(500); 
    if (i > 22) { 
      state = false; 
      break; 
    }
    i++;
  }
  Serial.println("");

  if(state){
    if(eFlag)
    {Serial.print("                       ");
      Serial.print(essid); // print the essid sotred in eeprom
    }
    else
    { Serial.print("                       ");
      Serial.print(ssid);// print ssid
  }
    Serial.println("'s WiFi Connected Successfully"); 
    Serial.print("                       ");
    Serial.print("IP address :"); 
    IPAddress myIP = WiFi.localIP(); // get the local ip in myIP variable
    Serial.println(myIP); 
  }
  return state; 
}

//********************Loop for Subscriber********************\\

void subscriber()
{

  client.subscribe("mqtt/ka"); // topic name of MQTT broker is provided

}

//********************Loop for Callback********************\\

void callback(char* topic, byte* message, unsigned int length) // function is created for fetching data
{ 
  Serial.println("");
  Serial.print("  ");
  Serial.print("callback function >> ");
  Serial.println("Message arrived"); // print ("Message arrived [") in serial monitor
  Serial.print("                       ");
  Serial.print("Topic :");
  Serial.println(topic); // print topic of publisher
  Serial.print("                       Hex code is :");
  for (int i = 0; i < length; i++) 
  {
    
    Serial.print((char)message[i]);// print received data to serial monitor. here we have to convert recieved data into character so "(char) message is used"
    irsend.sendNEC((char)message[i]); // send recived hex code which is in "message" variable. Hex will be send from D2 pin of nodemcu at 38 kHz frequency
    
  }
   Serial.println("");
//   irsend.sendNEC((char)'\n'); // to write data into new line "\n" is used 
//   irsend.sendNEC((char)'\r');// "\r" is used for carriage return
}

//********************Loop for Reconnect********************\\

void reconnect() // function is created for reconnection to MQTT broker
{ 
   Serial.print(" reconnect function >> ");
  // Loop will run until we reconnect to the MQTT broker
  int counter = 0; // initiate one variable as counter
  while (!client.connected()) // when client is not connected then will enter in loop
  {
    if (counter==5)//if counter value is equales to 5 then will enter to loop
    {
      ESP.restart();//it will do software restart of ESP8266
    }
    counter+=1; //  increament value of counter variable
    Serial.println("Attempting MQTT connection");// Attempt to connect to MQTT publisher
       
    if (client.connect("6039dcf1-4241-45ff-bec3-219526bb0492","krishna" , "asutosh"))// Client ID, user name and password is declared of MQTT broker
    { // If Node mcu is connected to the broker then will enter to this loop
      Serial.print("                       ");
      Serial.println("connected to MQTT Publisher"); // print connected at serial monitor
      subscriber(); 
    } 
    else // it will try to reconnect to the provided details of MQTT broker
    {
      Serial.print("                       ");
      Serial.print("failed to connect, rc="); 
      Serial.println(client.state()); // print status of connection to client
      Serial.print("                       ");
      Serial.println("try again in 5 seconds");// Wait 5 seconds before retrying
      delay(1000);// delay generated of 1000 milisecond i.e. 1 seconds
      Serial.print("                       ");
      Serial.println("try again in 4 seconds");
      delay(1000);
      Serial.print("                       ");
      Serial.println("try again in 3 seconds");
      delay(1000);
      Serial.print("                       ");
      Serial.println("try again in 2 seconds");
      delay(1000);
      Serial.print("                       ");
      Serial.println("try again in 1 seconds");
      Serial.print("                       ");
      Serial.println("Retrying.....");
      Serial.print("                       ");
    }
  }
}


void setup() {
  Serial.begin(115200);  
//  Tnetwork = WiFi.scanNetworks(); 
  EEPROM.begin(512);
  eepromRead(0);
//  Serial.print("                       ");
  wifiConnect = setup_wifi(); 
  eFlag = true;

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/html", index_html, option);
  });
  
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputMessage1;
    String inputMessage2;
    
    String inputParam;
    String inputParam1;
    String inputParam2;
    
    
    if (request->hasParam(INPUT_1)) 
    {
      inputMessage1 = request->getParam(INPUT_1)->value();
      inputParam1 = INPUT_1;
    }
    
    if (request->hasParam(INPUT_2)) 
    {
      inputMessage2 = request->getParam(INPUT_2)->value();
      inputParam2 = INPUT_2;
    }
    
    else 
    {
      inputMessage = "No message sent";
      inputParam = "none";
    }

    for (int i = 0 ; i < 512 ; i++) 
    {
      EEPROM.write(i, 0);
    }
  
    eepromWrite(0, inputMessage1, inputMessage2);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
  
  client.setServer(mqtt_server, 1883);// function named "mqtt_server" called to connect to MQTT server 
  client.setCallback(callback);// call the function named "callback" which is created above
  irsend.begin();
}

void loop() {
  Tnetwork = WiFi.scanNetworks();// scan the available nearby wifi and store it in Tnetwork variabale
  // below loop will scan available wifi and will update the list of available wifi
    for (int i = 0; i < Tnetwork; i++)
  {
    ssid_names[i] = WiFi.SSID(i);// store the ssid name in ssid_names variable
    }
  eFlag = true;
    if(WiFi.status() != WL_CONNECTED)//if wifi is not connected then enter to ht e loop
    {
    eFlag = true;// set eFlag to true
    setup_wifi(); // setup_wifi() function is called
        }

    
    if(WiFi.status() != WL_CONNECTED)//if wifi is not connected then enter to ht e loop
    {
      eFlag = false; // set eFlag to false
      setup_wifi();// setup_wifi() function is called
    }
     if (!client.connected()) //if client is not connected then enter to this loop 
  {
      reconnect(); // try to reconnect the client and try untill client is connected
    }
  client.loop(); 
}
