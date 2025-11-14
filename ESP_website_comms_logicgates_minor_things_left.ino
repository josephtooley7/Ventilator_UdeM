
#include <WiFi.h>
#include <WebServer.h>


int lastHumidity = -1;
int lastPressure = -1;

const int cyclePins[] = {18, 19, 21};

// Define all 7 non-zero combinations of 3 pins
const byte pinStates[][3] = {
  {0,0,1},  // 001
  {0,1,0},  // 010
  {1,0,0},  // 100
  {0,1,1},  // 011
  {1,0,1},  // 101
  {1,1,0},  // 110
  {1,1,1}   // 111
};

const int numStates = sizeof(pinStates) / sizeof(pinStates[0]);
int currentCycleIndex = 0;
String cycleResponses[numStates];   // Now one response per state

// Custom labels for each state
const char* stateLabels[numStates] = {
  "Battery percent = ",
  "Pressure = ",
  "Humidity = ",
  "Temperature = ",
  "Flow rate = ",
  "Voltage = ",
  "Current = "
};



const char WIFI_SSID[] = "BELL667";
const char WIFI_PASSWORD[] = "your_password_here";

#define LED_PIN 2
int LED_state = LOW;

WebServer server(80);

float getTemperature() {
  float temp_x100 = random(0, 10000);
  return temp_x100 / 100.0;
}

// Home Page
const char HTML_CONTENT_HOME[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head><title>My first website</title></head>
<body style="background-color: rgb(100, 100, 100); margin: 0;">
  <div class="square" style="text-align: center;"></div>
  <style>
    .square {
      width: 300px;
      height: 550px;
      position:absolute;
      left: 50%;
      top: 2%;
      transform: translateX(-50%);
      z-index: -1;
      background-color: rgb(110, 110,110);
      border: 10px solid rgb(200, 200, 200);
      border-radius: 20px;
    }
  </style>
  <div style="width: 100%; text-align: center; padding-top: 50px;">
    <a href="/information.html">
      <button style="background-color: rgb(2, 113, 249); color: #f6f5f5; font-size: 1cm; border-radius: 10px; padding: 20px 40px;">Information</button>
    </a>
  </div>
  <div style="width: 100%; text-align: center; padding-top: 40px;">
    <a href="/controls.html">
      <button style="background-color: rgb(2, 113, 249); color: #f6f5f5; font-size: 1cm; border-radius: 10px; padding: 20px 65px;">Controls</button>
    </a>
  </div>

  <div style="width: 100%; text-align: center; padding-top: 40px;">
    <a href="/lost.html">
      <button style="background-color: rgb(2, 113, 249); color: #f6f5f5; font-size: 1cm; border-radius: 10px; padding: 20px 100px;">Lost</button>
    </a>
  </div>

  <div style="width: 100%; text-align: center; padding-top: 40px;">
    <a href="/save.html">
      <button style="background-color: rgb(2, 113, 249); color: #f6f5f5; font-size: 1cm; border-radius: 10px; padding: 20px 92px;">Save</button>
    </a>
  </div>
  
</body>
</html>
)=====";

// Temperature Page
const char HTML_CONTENT_TEMPERATURE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head><title>Temperature</title></head>
<body style="background-color: rgb(100, 100, 100); margin: 0;">
  <h1 style="text-align:center;">Temperature Page</h1>
  <p style="text-align:center;">Current temperature: %TEMPERATURE_VALUE% &deg;C</p>
  <div style="width: 100%; text-align: center; padding-top: 40px;">
        <a href="/">
            <button style="
                background-color: rgb(2, 113, 249);
                color: #f6f5f5;
                font-size: 1cm;
                border-radius: 10px;
                border-top-color: rgb(16, 124, 255);
                border-left-color: rgb(16, 124,255);
                padding: 20px 100px;
            ">Back</button>
        </a>
    </div>
</body>
</html>
)=====";

// LED Page
const char HTML_CONTENT_LED[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head><title>LED Control</title></head>
<body style="background-color: rgb(100, 100, 100); margin: 0;">
  <h1 style="text-align:center;">LED Control Page</h1>
  <p style="text-align:center;">LED is currently: %LED_STATE%</p>
  <div style="text-align:center;">
    <a href="/led.html?state=on"><button style="padding: 10px 20px;">Turn ON</button></a>
    <a href="/led.html?state=off"><button style="padding: 10px 20px;">Turn OFF</button></a>
    <a href="/"><button style="padding: 10px 20px;">Back</button></a>
  </div>
</body>
</html>
)=====";

// 404 Page
const char HTML_CONTENT_404[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head><title>404 Not Found</title></head>
<body style="background-color: rgb(100, 100, 100); margin: 0;">
  <h1 style="text-align:center;">404 - Page Not Found</h1>
  <div style="width: 100%; text-align: center; padding-top: 40px;">
        <a href="/">
            <button style="
                background-color: rgb(2, 113, 249);
                color: #f6f5f5;
                font-size: 1cm;
                border-radius: 10px;
                border-top-color: rgb(16, 124, 255);
                border-left-color: rgb(16, 124,255);
                padding: 20px 95px;
            ">Back</button>
        </a>
    </div>
</body>
</html>



)=====";

// Controls Page
const char HTML_CONTENT_CONTROLS[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Toggle Switch</title>
  <style>
    .switch {
      position: relative;
      display: inline-block;
      width: 60px;
      height: 34px;
    }

    /* Hide default checkbox */
    .switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }

    /* Slider */
    .slider {
      position: absolute;
      cursor: pointer;
      top: 0; left: 0;
      right: 0; bottom: 0;
      background-color: #ccc;
      transition: 0.4s;
      border-radius: 34px;
    }

    .slider::before {
      position: absolute;
      content: "";
      height: 26px; width: 26px;
      left: 4px; bottom: 4px;
      background-color: white;
      transition: 0.4s;
      border-radius: 50%;
    }

    input:checked + .slider {
      background-color: #4CAF50;
    }

    input:checked + .slider::before {
      transform: translateX(26px);
    }
  </style>
</head>
<body style="background-color: rgb(100, 100, 100); margin: 0;">







    <!-- Toggle Switch Component -->
<label class="my-toggle-switch">
  <input type="checkbox">
  <span class="my-slider"></span>
</label>

<style>
  .my-toggle-switch {
    position: relative;
    display: inline-block;
    width: 60px;
    height: 34px;
  }

  .my-toggle-switch input {
    opacity: 0;
    width: 0;
    height: 0;
  }

  .my-slider {
    position: absolute;
    cursor: pointer;
    top: 0; left: 0;
    right: 0; bottom: 0;
    background-color: #ccc;
    transition: 0.4s;
    border-radius: 34px;
  }

  .my-slider::before {
    position: absolute;
    content: "";
    height: 26px; width: 26px;
    left: 4px; bottom: 4px;
    background-color: white;
    transition: 0.4s;
    border-radius: 50%;
  }

  .my-toggle-switch input:checked + .my-slider {
    background-color: #4CAF50;
  }

  .my-toggle-switch input:checked + .my-slider::before {
    transform: translateX(26px);
  }
</style>

<!-- Toggle Switch Component -->

  
  <div style="width: 100%; text-align: center; padding-top: 40px;">
        <a href="/humidity.html">
           <button style="background-color: rgb(2, 113, 249); color: #f6f5f5; font-size: 1cm; border-radius: 10px; padding: 20px 65px;">Humidity</button>
        
        </a>
    </div>  

  <div style="width: 100%; text-align: center; padding-top: 40px;">
        <a href="/pressure.html">
           <button style="background-color: rgb(2, 113, 249); color: #f6f5f5; font-size: 1cm; border-radius: 10px; padding: 20px 65px;">Pressure</button>
        
        </a>
  </div>  

  <div style="width: 100%; text-align: center; padding-top: 40px;">
        <a href="/">
            <button style="
                background-color: rgb(2, 113, 249);
                color: #f6f5f5;
                font-size: 1cm;
                border-radius: 10px;
                border-top-color: rgb(16, 124, 255);
                border-left-color: rgb(16, 124,255);
                padding: 20px 100px;
            ">Back</button>
        </a>
    </div>




<h2>Toggle Switch Example</h2>

<label class="switch">
  <input type="checkbox">
  <span class="slider"></span>
</label>

</body>
</html>

)=====";

// Lost Page
const char HTML_CONTENT_LOST[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>My first website</title>
</head>
<body style="background-color: rgb(100, 100, 100); margin: 0;">

    <div style="width: 100%; text-align: center; padding-top: 50px;">
        <a href="/led.html">
            <button style="
                background-color: rgb(2, 113, 249);
                color: #f6f5f5;
                font-size: 1cm;
                border-radius: 10px;
                border-top-color: rgb(16, 124, 255);
                border-left-color: rgb(16, 124,255);
                padding: 20px 40px;
            ">Play Sound</button>
        </a>
    </div>

    <div style="width: 100%; text-align: center; padding-top: 40px;">
        <a href="/">
            <button style="
                background-color: rgb(2, 113, 249);
                color: #f6f5f5;
                font-size: 1cm;
                border-radius: 10px;
                border-top-color: rgb(16, 124, 255);
                border-left-color: rgb(16, 124,255);
                padding: 20px 100px;
            ">Back</button>
        </a>
    </div>

</body>
</html>

)=====";

// Pressure Page
const char HTML_CONTENT_PRESSURE[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html>
<head>
  <title>Pressure Settings</title>
  <style>
    .custom-input {
      font-size: 1.2cm;
      color: rgb(9, 25, 139);
      background-color: rgb(200, 200, 200);
      border: 2px solid #888;
      border-radius: 10px;
      padding: 20px;
      width: 300px;
    }
    .custom-input::placeholder {
      font-size: 1.2cm;
      color: rgb(60, 60, 60);
      opacity: 1;
    }
  </style>
</head>
<body style="background-color: rgb(100, 100, 100); margin: 0; width: 100%; text-align: center; padding-top: 50px;">
  <input type="number" id="pressure" class="custom-input" placeholder="Pressure" min="3" max="40">

  <div style="width: 100%; text-align: center; padding-top: 50px;">
    <button onclick="sendPressure()" style="
        background-color: rgb(2, 113, 249);
        color: #f6f5f5;
        font-size: 1cm;
        border-radius: 10px;
        padding: 20px 40px;
    ">Save Settings</button>
  </div>

  <script>
  function sendPressure() {
    const pressure = parseInt(document.getElementById("pressure").value);
    if (isNaN(pressure) || pressure < 3 || pressure > 40) {
      alert("Please enter a pressure value between 3 and 40.");
      return;
    }

    fetch("/savePressure", {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body: "pressure=" + encodeURIComponent(pressure)
    })
    .then(response => response.text())
    .then(data => {
      console.log(data);
      alert("Pressure sent: " + pressure);
    })
    .catch(error => {
      console.error("Error:", error);
      alert("Failed to send Pressure.");
    });
  }
  </script>


  <div style="width: 100%; text-align: center; padding-top: 40px;">
    <a href="/controls.html">
      <button style="background-color: rgb(2, 113, 249); color: #f6f5f5; font-size: 1cm; border-radius: 10px; padding: 20px 100px;">Back</button>
    </a>
  </div>

  <div style="width: 100%; text-align: center; padding-top: 40px;">
    <a href="/">
      <button style="background-color: rgb(2, 113, 249); color: #f6f5f5; font-size: 1cm; border-radius: 10px; padding: 20px 100px;">Home</button>
    </a>
  </div>
</body>
</html>
)=====";

// Humidity Page
const char HTML_CONTENT_HUMIDITY[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html>
<head>
  <title>Humidity Settings</title>
  <style>
    .custom-input {
      font-size: 1.2cm;
      color: rgb(9, 25, 139);
      background-color: rgb(200, 200, 200);
      border: 2px solid #888;
      border-radius: 10px;
      padding: 20px;
      width: 300px;
    }
    .custom-input::placeholder {
      font-size: 1.2cm;
      color: rgb(60, 60, 60);
      opacity: 1;
    }
  </style>
</head>
<body style="background-color: rgb(100, 100, 100); margin: 0; width: 100%; text-align: center; padding-top: 50px;">
  <input type="number" id="humidity" class="custom-input" placeholder="Humidity %" min="0" max="100">

  <div style="width: 100%; text-align: center; padding-top: 50px;">
    <button onclick="sendHumidity()" style="
        background-color: rgb(2, 113, 249);
        color: #f6f5f5;
        font-size: 1cm;
        border-radius: 10px;
        padding: 20px 40px;
    ">Save Settings</button>
  </div>

  <script>
  function sendHumidity() {
    const humidity = parseInt(document.getElementById("humidity").value);
    if (isNaN(humidity) || humidity < 0 || humidity > 100) {
      alert("Please enter a humidity value between 0 and 100.");
      return;
    }

    fetch("/saveHumidity", {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body: "humidity=" + encodeURIComponent(humidity)
    })
    .then(response => response.text())
    .then(data => {
      console.log(data);
      alert("Humidity sent: " + humidity);
    })
    .catch(error => {
      console.error("Error:", error);
      alert("Failed to send humidity.");
    });
  }
  </script>


  <div style="width: 100%; text-align: center; padding-top: 40px;">
    <a href="/controls.html">
      <button style="background-color: rgb(2, 113, 249); color: #f6f5f5; font-size: 1cm; border-radius: 10px; padding: 20px 100px;">Back</button>
    </a>
  </div>

  <div style="width: 100%; text-align: center; padding-top: 40px;">
    <a href="/">
      <button style="background-color: rgb(2, 113, 249); color: #f6f5f5; font-size: 1cm; border-radius: 10px; padding: 20px 100px;">Home</button>
    </a>
  </div>
</body>
</html>
)=====";

const char HTML_CONTENT_INFORMATION[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html>

<head>
  <meta http-equiv="refresh" content="5">
  <title>Information</title>
</head>

<body style="background-color:rgb(100,100,100);color:white;text-align:center;">
  <h1>Card Responses</h1>
  <ul style="list-style:none;font-size:1.2em;padding:0;">
    <li>Humidity = %INFO_X%</li>
    <li>Pressure = %INFO_Y%</li>
    <li>Hours spent = %INFO_Z%</li>
  </ul>
  <div style="padding-top:40px;">
    <a href="/"><button style="font-size:1cm;padding:20px 100px;background-color:rgb(2,113,249);color:#f6f5f5;border-radius:10px;">Back</button></a>
  </div>
</body>
</html>
)=====";


// Save Page
const char HTML_CONTENT_SAVE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>My first website</title>
</head>
<body style="background-color: rgb(100, 100, 100); margin: 0;">

     <div class="square" style="text-align: center;"></div>

<style>
  .square {
    width: 300px;
    height: 550px;
    position:absolute;
    left: 50%;
    top: 2%;
    transform: translateX(-50%);
    z-index: -1;

    background-color: transparent;
    border-style: solid;
    border-width: 10px;
    border-color: rgb(200, 200, 200);
    border-radius: 20px;
    background-color: rgb(110, 110,110);
    
    
  }
</style>


    <div style="width: 100%; text-align: center; padding-top: 50px;">
        <a href="/404.html">
            <button style="
                background-color: rgb(2, 113, 249);
                color: #f6f5f5;
                font-size: 1cm;
                border-radius: 10px;
                border-top-color: rgb(16, 124, 255);
                border-left-color: rgb(16, 124,255);
                padding: 20px 10px;
            ">Download Ram</button>
        </a>
    </div>

    <div style="width: 100%; text-align: center; padding-top: 40px;">
        <a href="/">
            <button style="
                background-color: rgb(2, 113, 249);
                color: #f6f5f5;
                font-size: 1cm;
                border-radius: 10px;
                border-top-color: rgb(16, 124, 255);
                border-left-color: rgb(16, 124,255);
                padding: 20px 95px;
            ">Back</button>
        </a>
    </div>


</body>
</html>

)=====";



// Route Handlers
void handleHome() {
  server.send(200, "text/html", HTML_CONTENT_HOME);
}

void handleTemperature() {
  float temperature = getTemperature();
  String html = HTML_CONTENT_TEMPERATURE;
  html.replace("%TEMPERATURE_VALUE%", String(temperature));
  server.send(200, "text/html", html);
}

void handleLed() {
  if (server.hasArg("state")) {
    String state = server.arg("state");
    if (state == "on") {
      LED_state = HIGH;
      digitalWrite(LED_PIN, LED_state);
      Serial.println("hello");   // USB serial monitor
      Serial2.println("hello");  // TX2 output
    } else {
      LED_state = LOW;
      digitalWrite(LED_PIN, LED_state);
      Serial.println("LED turned OFF");
      Serial2.println("LED turned OFF");
    }
  }
  String html = HTML_CONTENT_LED;
  html.replace("%LED_STATE%", LED_state ? "ON" : "OFF");
  server.send(200, "text/html", html);
}


void handleControls() {
  server.send(200, "text/html", HTML_CONTENT_CONTROLS);
}

void handleLost() {
  server.send(200, "text/html", HTML_CONTENT_LOST);
}

void handleSave() {
  server.send(200, "text/html", HTML_CONTENT_SAVE);
}

void handleHumidity() {
  String html = HTML_CONTENT_HUMIDITY;

  if (lastHumidity >= 0) {
    html.replace("id=\"humidity\"", "id=\"humidity\" value=\"" + String(lastHumidity) + "\"");
    html.replace("</script>", "</script><p style='color:white;'>Last saved humidity: " + String(lastHumidity) + "%</p>");
  }

  server.send(200, "text/html", html);
}


void handleNotFound() {
  server.send(404, "text/html", HTML_CONTENT_404);
}

void handleSaveHumidity() {
  if (server.hasArg("humidity")) {
    String humidityStr = server.arg("humidity");
    int humidity = humidityStr.toInt();

    if (humidity >= 0 && humidity <= 100) {

      lastHumidity = humidity;  // Save last value it

      // Set GPIO4 HIGH
      digitalWrite(4, HIGH);

      // to ensure transmission
      delay(100);

      // Send humidity to Serial and Serial2
      char formatted[4];
      sprintf(formatted, "%02d", humidity);
      Serial.println(formatted);
      Serial2.println(formatted);


      // to ensure transmission
      delay(100);

      // Set GPIO4 LOW
      digitalWrite(4, LOW);
    } else {
      Serial.println("Invalid humidity value: " + String(humidity));
      Serial2.println("Invalid humidity value: " + String(humidity));
    }
  } else {
    Serial.println("No humidity arg found");
    Serial2.println("No humidity arg found");
  }

  server.send(200, "text/plain", "Humidity received");
}

void handlePressure() {
  String html = HTML_CONTENT_PRESSURE;

  if (lastPressure >= 3 && lastPressure <= 40) {
    html.replace("id=\"pressure\"", "id=\"pressure\" value=\"" + String(lastPressure) + "\"");
    html.replace("</script>", "</script><p style='color:white;'>Last saved pressure: " + String(lastPressure) + "</p>");
  }

  server.send(200, "text/html", html);
}

void handleInformation() {
  String html = "<!DOCTYPE html><html><head><title>Information</title></head><body style='background-color:rgb(100,100,100);color:white;text-align:center;'>";
  html += "<h1>Card Responses</h1><ul style='list-style:none;font-size:1.2em;padding:0;'>";

  for (int i = 0; i < numStates; i++) {
  html += "<li>" + String(stateLabels[i]) +
          (cycleResponses[i].length() ? cycleResponses[i] : "No data") +
          "</li>";
}

  html += "</ul><div style='padding-top:40px;'><a href=\"/\"><button style=\"font-size:1cm;padding:20px 100px;background-color:rgb(2,113,249);color:#f6f5f5;border-radius:10px;\">Back</button></a></div></body></html>";

  server.send(200, "text/html", html);
}





void handleSavePressure() {
  if (server.hasArg("pressure")) {
    String pressureStr = server.arg("pressure");
    int pressure = pressureStr.toInt();

    if (pressure >= 3 && pressure <= 40) {

      lastPressure = pressure;  // Save it
      // Set GPIO5 HIGH
      digitalWrite(5, HIGH);

      // to ensure transmission
      delay(100);

      // Send humidity to Serial and Serial2
      char formatted[4];
      sprintf(formatted, "%02d", pressure);
      Serial.println(formatted);
      Serial2.println(formatted);


      // to ensure transmission
      delay(100);

      // Set GPIO5 LOW
      digitalWrite(5, LOW);
    } else {
      Serial.println("Invalid humidity value: " + String(pressure));
      Serial2.println("Invalid humidity value: " + String(pressure));
    }
  } else {
    Serial.println("No pressure arg found");
    Serial2.println("No pressure arg found");
  }

  server.send(200, "text/plain", "Pressure received");
}


void setup() {
  Serial.begin(9600);
  delay(1000);
  pinMode(LED_PIN, OUTPUT);

  for (int i = 0; i < 3; i++) {
    pinMode(cyclePins[i], OUTPUT);
    digitalWrite(cyclePins[i], LOW);
  }
  

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(WiFi.localIP());
  } else {
    WiFi.softAP("Ventilator_NO1", "ciscut-3doxho-vybDam");
    Serial.println(WiFi.softAPIP());
  }

  server.on("/", handleHome);
  server.on("/temperature.html", handleTemperature);
  server.on("/led.html", handleLed);
  server.on("/controls.html", handleControls);
  server.on("/lost.html", handleLost);
  server.on("/save.html", handleSave);

  server.on("/humidity.html", handleHumidity);
  server.on("/saveHumidity", HTTP_POST, handleSaveHumidity);
  server.on("/pressure.html", handlePressure);
  server.on("/savePressure", HTTP_POST, handleSavePressure);
  server.on("/information.html", handleInformation);


  server.onNotFound(handleNotFound);

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);


  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // RX2=GPIO16, TX2=GPIO17


  server.begin();
}

unsigned long lastCycleTime = 0;
const unsigned long cycleInterval = 1000;  // 1 second per pin


void loop() {
  server.handleClient();

  if (millis() - lastCycleTime >= cycleInterval) {
    // Apply the current state to pins
    for (int i = 0; i < 3; i++) {
      digitalWrite(cyclePins[i], pinStates[currentCycleIndex][i]);
    }

    delay(100);  // Allow signal to propagate

    // Clear buffer
    while (Serial2.available() > 1) {
      Serial2.read();
    }

    // Read from Serial2 and store
    if (Serial2.available()) {
      int incoming = Serial2.read();
      cycleResponses[currentCycleIndex] = String(incoming);
      Serial.println(String(stateLabels[currentCycleIndex]) + String(incoming));
                     " received: " + incoming);
    }

    // Reset pins LOW after capture
    for (int i = 0; i < 3; i++) {
      digitalWrite(cyclePins[i], LOW);
    }

    // Advance to next state
    currentCycleIndex = (currentCycleIndex + 1) % numStates;
    lastCycleTime = millis();
  }
}
