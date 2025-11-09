/*
 * This Arduino Nano ESP32 code was developed by newbiely.com
 *
 * This Arduino Nano ESP32 code is made available for public use without any restriction
 *
 * For comprehensive instructions and wiring diagrams, please visit:
 * https://newbiely.com/tutorials/arduino-nano-esp32/arduino-nano-esp32-web-server-multiple-pages
 */

const char *HTML_CONTENT_LED = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
    <title>LED Page</title>
</head>
<body>
    <h1>LED Page</h1>
    <p>LED State: <span style="color: red;">%LED_STATE%</span></p>
    <a href='/led.html?state=on'>Turn ON</a>
    <br><br>
    <a href='/led.html?state=off'>Turn OFF</a>
</body>
</html>
)=====";
