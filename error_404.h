/*
 * This Arduino Nano ESP32 code was developed by newbiely.com
 *
 * This Arduino Nano ESP32 code is made available for public use without any restriction
 *
 * For comprehensive instructions and wiring diagrams, please visit:
 * https://newbiely.com/tutorials/arduino-nano-esp32/arduino-nano-esp32-web-server-multiple-pages
 */

const char *HTML_CONTENT_404 = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
    <title>404 - Page Not Found</title>
    <style>
        h1 {color: #ff4040;}
    </style>
</head>
<body>
    <h1>404</h1>
    <p>Oops! The page you are looking for could not be found on Esp32 Web Server.</p>
    <p>Please check the URL or go back to the <a href="/">homepage</a>.</p>
    <p>Or check <a href="https://esp32io.com/tutorials/esp32-web-server-multiple-pages"> Esp32 Web Server</a> tutorial.</p>
</body>
</html>
)=====";
