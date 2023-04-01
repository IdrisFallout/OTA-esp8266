#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#define TIME 60
#define RETRY_AFTER 5

//Variables
int i = 0;
int statusCode;
const char* ssid = "text";
const char* passphrase = "text";
String st;
String content;


//Function Decalration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);

//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);

void setup() {

  Serial.begin(115200);  //Initialising if(DEBUG)Serial Monitor
  Serial.println();
  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();
  EEPROM.begin(512);  //Initialasing EEPROM
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println();
  Serial.println();
  Serial.println("Startup");

  setup_password();
}

void setup_password() {
  //---------------------------------------- Read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");

  String esid;
  for (int i = 0; i < 32; ++i) {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 96; ++i) {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);


  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi()) {
    Serial.println("Succesfully Connected!!!");
    return;
  } else {
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();  // Setup HotSpot
  }

  Serial.println();
  Serial.println("Waiting");

  int timer_counter = 0;
  while ((WiFi.status() != WL_CONNECTED)) {
    Serial.print(".");
    if((timer_counter % 5000) == 0){
      Serial.println();
    }
    server.handleClient();
    if (timer_counter >= (RETRY_AFTER * 60 * 1000)){
      break;
    }
    delay(100);
    timer_counter += 100;  
  }
  setup_password();
}


void loop() {
  if ((WiFi.status() == WL_CONNECTED)) {
    main_program();
  } else {
    // Try to reconnect
    setup_password();
  }
}


//----------------------------------------------- Fuctions used for WiFi credentials saving and connecting to it which you do not need to change
bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while (c < (2 * TIME)) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb() {
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", handleRoot);  // Add this line to handle the root page
  server.on("/data", handleData);
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i) {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("NodeMCU", "tiktok14");
  Serial.println("softap");
  launchWeb();
  Serial.println("over");
}

void handleRoot() {
  String html = "<html>";
  html += "<head>";
  html += "<style>";
  html += "body {";
  html += "  font-family: Arial, sans-serif;";
  html += "  background-color: #f7f7f7;";
  html += "}";
  html += "form {";
  html += "  max-width: 400px;";
  html += "  margin: 0 auto;";
  html += "  background-color: #fff;";
  html += "  border-radius: 5px;";
  html += "  padding: 20px;";
  html += "  box-shadow: 0px 0px 10px 0px rgba(0,0,0,0.5);";
  html += "}";
  html += "input[type=text], input[type=password] {";
  html += "  width: 100%;";
  html += "  padding: 12px 20px;";
  html += "  margin: 8px 0;";
  html += "  box-sizing: border-box;";
  html += "  border: solid black 1px;";
  html += "  border-radius: 4px;";
  html += "}";
  html += "button {";
  html += "  background-color: #4CAF50;";
  html += "  color: white;";
  html += "  padding: 14px 20px;";
  html += "  margin: 8px 0;";
  html += "  border: none;";
  html += "  border-radius: 4px;";
  html += "  cursor: pointer;";
  html += "}";
  html += "button:hover {";
  html += "  background-color: #45a049;";
  html += "}";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<form>";
  html += "<label for='name'>SSID:</label>";
  html += "<input type='text' id='name' name='name'>";
  html += "<label for='password'>PASSWORD:</label>";
  html += "<input type='password' id='password' name='password'>";
  html += "<button type='button' onclick='submitData()'>Submit</button>";
  html += "</form>";
  html += "<script>";
  html += "function submitData() {";
  html += "  var name = document.getElementById('name').value;";
  html += "  var password = document.getElementById('password').value;";
  html += "  var xhttp = new XMLHttpRequest();";
  html += "  xhttp.onreadystatechange = function() {";
  html += "    if (this.readyState == 4 && this.status == 200) {";
  html += "      console.log(this.responseText);";
  html += "    }";
  html += "  };";
  html += "  xhttp.open('POST', '/data', true);";
  html += "  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');";
  html += "  xhttp.send('name=' + name + '&password=' + password);";
  html += "}";
  html += "</script>";
  html += "</body>";
  html += "</html>";
  server.send(200, "text/html", html);
}

void handleData() {
  if (server.method() == HTTP_POST) {
    String name = server.arg("name");
    String password = server.arg("password");
    Serial.print("SSID: ");
    Serial.println(name);
    Serial.print("Password: ");
    Serial.println(password);

    String qsid = name;
    String qpass = password;

    if (qsid.length() > 0 && qpass.length() > 0) {
      Serial.println("clearing eeprom");
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      Serial.println(qsid);
      Serial.println("");
      Serial.println(qpass);
      Serial.println("");

      Serial.println("writing eeprom ssid:");
      for (int i = 0; i < qsid.length(); ++i) {
        EEPROM.write(i, qsid[i]);
        Serial.print("Wrote: ");
        Serial.println(qsid[i]);
      }
      Serial.println("writing eeprom pass:");
      for (int i = 0; i < qpass.length(); ++i) {
        EEPROM.write(32 + i, qpass[i]);
        Serial.print("Wrote: ");
        Serial.println(qpass[i]);
      }
      EEPROM.commit();

      content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
      statusCode = 200;
      ESP.reset();
    } else {
      content = "{\"Error\":\"404 not found\"}";
      statusCode = 404;
      Serial.println("Sending 404");
    }
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(statusCode, "application/json", content);
  }
}

  void main_program() {
    for (int i = 0; i < 10; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
    }
  }