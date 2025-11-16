
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
  "Battery%     = ",
  "Humidity     = ",
  "Pressure MAX = ",
  "Pressure MIN = ",
  "Temperature  = ",
  "Time / water level = ",
  "empty value = "
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
    
  </style>
</head>
<body style="background-color: rgb(100, 100, 100); margin: 0;">


  
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
        <a href="/modes.html">
           <button style="
                background-color: rgb(2, 113, 249); 
                color: #f6f5f5; 
                font-size: 1cm; 
                border-radius: 10px; 
                padding: 20px 65px;">Modes</button>
        
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


</label>

</body>
</html>

)=====";

//modes page
const char HTML_CONTENT_MODES[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html>
<head>
  <title>Modes</title>
</head>
<body style="background-color: rgb(100, 100, 100); margin: 0; text-align:center;">
  <h1 style="color:white;">Select Mode</h1>

  <select id="modeSelect" style="font-size:1.2cm; padding:10px; border-radius:10px;">
    <!--Ajust the modes value here-->
    <option value="1">Cpap</option>
    <option value="2">Bipap</option>
    <option value="3">Mode 3</option>
    <option value="4">Mode 4</option>
    <option value="5">Mode 5</option>
    <option value="6">Mode 6</option>
  </select>

  <div style="padding-top:40px;">
    <button onclick="sendMode()" style="background-color:rgb(2,113,249);color:#f6f5f5;font-size:1cm;border-radius:10px;padding:20px 40px;">Send Mode</button>
  </div>

  <script>
    function sendMode() {
      const mode = document.getElementById("modeSelect").value;
      fetch("/saveMode", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: "mode=" + encodeURIComponent(mode)
      })
      .then(r => r.text())
      .then(data => alert("Mode sent: " + mode))
      .catch(err => alert("Failed to send mode."));
    }
  </script>

  <div style="padding-top:40px;">
    <a href="/controls.html">
      <button style="background-color:rgb(2,113,249);color:#f6f5f5;font-size:1cm;border-radius:10px;padding:20px 100px;">Back</button>
    </a>
  </div>
  <div style="padding-top:40px;">
      <a href="/.html">
        <button style="background-color:rgb(2,113,249);color:#f6f5f5;font-size:1cm;border-radius:10px;padding:20px 100px;">Home</button>
      </a>
    </div>
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

//pressure page

const char HTML_CONTENT_PRESSURE[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html>
<head>
  <title>Pressure Settings</title>
  <style>
    body {
      background-color: rgb(100, 100, 100);
      text-align: center;
      padding-top: 50px;
      margin: 0;
      font-family: Arial, sans-serif;
    }
    .section {
      margin-bottom: 60px;   /* <-- adds space between sections */
    }
    .custom-input {
      font-size: 1.2cm;
      color: rgb(9, 25, 139);
      background-color: rgb(200, 200, 200);
      border: 2px solid #888;
      border-radius: 10px;
      padding: 20px;
      width: 300px;
    }
    .custom-input.invalid {
      border-color: red;
      background-color: #fdd;
    }
    .button {
      background-color: rgb(2,113,249);
      color: #f6f5f5;
      font-size: 1cm;
      border-radius: 10px;
      padding: 20px 40px;
      margin-top: 20px;
      cursor: pointer;
    }
  </style>
</head>
<body>

  <!-- Min Pressure Section -->
  <div class="section">
    <input type="number" id="minPressure" class="custom-input"
           placeholder="Min Pressure" min="3" max="40">
    <div>
      <button onclick="sendMinPressure()" class="button">Save Min</button>
    </div>
  </div>

  <!-- Max Pressure Section -->
  <div class="section">
    <input type="number" id="maxPressure" class="custom-input"
           placeholder="Max Pressure" min="3" max="40">
    <div>
      <button onclick="sendMaxPressure()" class="button">Save Max</button>
    </div>
  </div>

  <script>
    function validateInput(id) {
      const el = document.getElementById(id);
      const val = parseInt(el.value);
      if (isNaN(val) || val < 3 || val > 40) {
        el.classList.add("invalid");
        return null;
      } else {
        el.classList.remove("invalid");
        return val;
      }
    }

    function sendMinPressure() {
      const val = validateInput("minPressure");
      if (val === null) {
        alert("Please enter a Min Pressure between 3 and 40.");
        return;
      }
      fetch("/saveMinPressure", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: "minPressure=" + encodeURIComponent(val)
      })
      .then(r => r.text())
      .then(data => alert("Min Pressure sent: " + val))
      .catch(err => alert("Failed to send Min Pressure."));
    }

    function sendMaxPressure() {
      const val = validateInput("maxPressure");
      if (val === null) {
        alert("Please enter a Max Pressure between 3 and 40.");
        return;
      }
      fetch("/saveMaxPressure", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: "maxPressure=" + encodeURIComponent(val)
      })
      .then(r => r.text())
      .then(data => alert("Max Pressure sent: " + val))
      .catch(err => alert("Failed to send Max Pressure."));
    }
  </script>

  <div style="padding-top:40px;">
    <a href="/controls.html">
      <button class="button" style="padding:20px 100px;">Back</button>
    </a>
  </div>

  <div style="padding-top:40px;">
    <a href="/">
      <button class="button" style="padding:20px 100px;">Home</button>
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
  <h1>Information</h1>
  <ul style="list-style:none;font-size:2cm;padding:12px;">
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
    //html.replace("</script>", "</script><p style='color:white;'>Last saved humidity: " + String(lastHumidity) + "%</p>");
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
      digitalWrite(5, LOW);

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
      Serial.println(0);
      Serial2.println(0);
    }
  } else {
    Serial.println(0);
    Serial2.println(0);
  }

  server.send(200, "text/plain", "Humidity received");
}

void handlePressure() {
  String html = HTML_CONTENT_PRESSURE;

  if (lastPressure >= 3 && lastPressure <= 40) {
    html.replace("id=\"pressure\"", "id=\"pressure\" value=\"" + String(lastPressure) + "\"");
    //html.replace("</script>", "</script><p style='color:white;'>Last saved pressure: " + String(lastPressure) + "</p>");
  }

  server.send(200, "text/html", html);
}

void handleInformation() {
  String html = "<!DOCTYPE html><html><head><title>Information</title></head><body style='background-color:rgb(100,100,100);color:white;text-align:center;'>";
  html += "<h1>Information</h1><ul style='list-style:none;font-size:1.2cm;padding:15px;'>";

  for (int i = 0; i < numStates; i++) {
  html += "<li style='margin:15px 0;'>" + String(stateLabels[i]) +
          "<span style='font-size:1.5cm; color:yellow; font-weight:bold;'>" +
          (cycleResponses[i].length() ? cycleResponses[i] : "No data") +
          "</span></li>";
}


  html += "</ul><div style='padding-top:40px;'><a href=\"/\"><button style=\"font-size:1cm;padding:20px 100px;background-color:rgb(2,113,249);color:#f6f5f5;border-radius:10px;\">Back</button></a></div></body></html>";

  server.send(200, "text/html", html);
}





void handleSaveMinPressure() {
  if (server.hasArg("minPressure")) {
    int val = server.arg("minPressure").toInt();

    if (val >= 3 && val <= 40) {   // enforce valid range
      lastPressure = val;

      // Pin signaling
      digitalWrite(4, LOW);
      digitalWrite(5, HIGH);

      delay(100);

      // Send only the number
      Serial.println(val);
      Serial2.println(val);

      digitalWrite(4, LOW);
      digitalWrite(5, LOW);
    } else {
      // Still send the number even if out of range
      Serial.println(0);
      Serial2.println(0);
    }
  } else {
    // If no argument, send 0 (or blank if you prefer)
    Serial.println(0);
    Serial2.println(0);
  }

  // Respond to HTTP client with just the number too
  server.send(200, "text/plain", "Min Pressure received");

}


void handleSaveMaxPressure() {
  if (server.hasArg("maxPressure")) {
    int val = server.arg("maxPressure").toInt();

    if (val >= 3 && val <= 40) {   // <-- enforce valid range
      lastPressure = val;

      // NEW behavior: set both pins HIGH
      digitalWrite(4, HIGH);
      digitalWrite(5, HIGH);

      delay(100);
      Serial.println(val);
      Serial2.println(val);

      digitalWrite(4, LOW);
      digitalWrite(5, LOW);
    } else {
      Serial.println(0);
      Serial2.println(0);
    }
  } else {
    Serial.println(0);
    Serial2.println(0);
  }

  server.send(200, "text/plain", "Max Pressure received");
}

int lastMode = -1;
#define MODE_PIN 22   // free pin chosen

void handleSaveMode() {
  if (server.hasArg("mode")) {
    int modeVal = server.arg("mode").toInt();
    lastMode = modeVal;

    // Set MODE_PIN HIGH briefly
    digitalWrite(MODE_PIN, HIGH);
    delay(100);

    // Send mode number via TX2
    Serial2.println(modeVal);
    Serial.println("Mode sent: " + String(modeVal));

    digitalWrite(MODE_PIN, LOW);
  } else {
    Serial2.println(0);
    Serial.println("No mode received");
  }

  server.send(200, "text/plain", "Mode received");
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

  pinMode(MODE_PIN, OUTPUT);
  digitalWrite(MODE_PIN, LOW);

  server.on("/modes.html", [](){ server.send(200, "text/html", HTML_CONTENT_MODES); });
  server.on("/saveMode", HTTP_POST, handleSaveMode);


  server.on("/", handleHome);
  server.on("/temperature.html", handleTemperature);
  server.on("/led.html", handleLed);
  server.on("/controls.html", handleControls);
  server.on("/lost.html", handleLost);
  server.on("/save.html", handleSave);

  server.on("/humidity.html", handleHumidity);
  server.on("/saveHumidity", HTTP_POST, handleSaveHumidity);
  server.on("/pressure.html", handlePressure);
  //server.on("/savePressure", HTTP_POST, handleSavePressure);
  server.on("/information.html", handleInformation);

  server.on("/saveMinPressure", HTTP_POST, handleSaveMinPressure);
  server.on("/saveMaxPressure", HTTP_POST, handleSaveMaxPressure);


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
      Serial.println("State " + String(currentCycleIndex) + " (" +
               stateLabels[currentCycleIndex] + ") received: " +
               String(incoming));

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
