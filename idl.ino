#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include "DHT.h"
#include "MQ135.h"
#include <ESPAsyncTCP.h>


// Replace with your network credentials
const char* ssid = "EKNET";
const char* password = "Hasan2SalakOsman4.";

#define DHTPIN D7 // Digital pin 2 (GPIO 4) connected to the DHT sensor
#define ANALOGPIN A0
#define BUZZERPIN D6

// Uncomment the type of sensor in use:
#define DHTTYPE DHT11 // DHT 11
// #define DHTTYPE DHT22 // DHT 22 (AM2302)
// #define DHTTYPE DHT21 // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);
MQ135 gasSensor = MQ135(ANALOGPIN);

// current temperature & humidity, this will be updated in loop function
float t = 0.0;
float tf = 0.0;
float h = 0.0;
float ppm = 0.0;
float oaq= 0.0;

float h_qual = 0.0;
float ppm_qual = 0.0;
float t_qual = 0.0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

unsigned long previousMillis = 0; //stoe last time DHT was updated
const long interval = 1000; // Updates DHT readings every 1 seconds

//web page
const char index_html[] PROGMEM = R"webpage(
<!DOCTYPE HTML><html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<script src="https://code.highcharts.com/8.0/highcharts.js"></script>
<style>
body {
min-width: 300px;
max-width: 800px;
height: 400px;
margin: 0 auto;
}
h2 {
font-family: Arial;
font-size: 2.5rem;
text-align: center;
}
</style>
</head>
<body>
<h2> Indoor Air Quality Detecting Project </h2>
<div id="temperature-chart" class="container"></div>
<div id="fahrenheit-chart" class="container"></div>
<div id="humidity-chart" class="container"></div>
<div id="airquality-chart" class="container"></div>
<div id="overallquality-chart" class="container"></div>
</body>
<script>
var chartT = new Highcharts.Chart({
chart:{ renderTo : 'temperature-chart' },
title: { text: 'Temperature in Degree Celsius' },
series: [{
showInLegend: false,
data: []
}],
plotOptions: {
line: { animation: false,
dataLabels: { enabled: true }
},
series: { color: '#059e8a' }
},
xAxis: { type: 'datetime',
dateTimeLabelFormats: { second: '%H:%M:%S' }
},
yAxis: {
title: { text: 'Temperature (Celsius)' }
},
credits: { enabled: false }
});
setInterval(function ( ) {
var xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function() {
if (this.readyState == 4 && this.status == 200) {
var x = (new Date()).getTime(),
y = parseFloat(this.responseText);
if(chartT.series[0].data.length > 200) {
chartT.series[0].addPoint([x, y], true, true, true);
} else {
chartT.series[0].addPoint([x, y], true, false, true);
}
}
};
xhttp.open("GET", "/temperature", true);
xhttp.send();
}, 2000 ) ;

var chartF = new Highcharts.Chart({
chart:{ renderTo:'fahrenheit-chart' },
title: { text: 'Temperature in Fahrenheit' },
series: [{
showInLegend: false,
data: []
}],
plotOptions: {
line: { animation: false,
dataLabels: { enabled: true }
}
},
xAxis: {
type: 'datetime',
dateTimeLabelFormats: { second: '%H:%M:%S' }
},
yAxis: {
title: { text: 'fahrenheit (F)' }
},
credits: { enabled: false }
});
setInterval(function ( ) {
var xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function() {
if (this.readyState == 4 && this.status == 200) {
var x = (new Date()).getTime(),
y = parseFloat(this.responseText);
//console.log(this.responseText);
if(chartF.series[0].data.length > 200) {
chartF.series[0].addPoint([x, y], true, true, true);
} else {
chartF.series[0].addPoint([x, y], true, false, true);
}
}
};
xhttp.open("GET", "/fahrenheit", true);
xhttp.send();
}, 2000 ) ;

var chartH = new Highcharts.Chart({
chart:{ renderTo:'humidity-chart' },
title: { text: 'Humidity (%)' },
series: [{
showInLegend: false,
data: []
}],
plotOptions: {
line: { animation: false,
dataLabels: { enabled: true }
},
series: { color: '#18009c' }
},
xAxis: {
type: 'datetime',
dateTimeLabelFormats: { second: '%H:%M:%S' }
},
yAxis: {
title: { text: 'Humidity (%)' }
},
credits: { enabled: false }
});
setInterval(function ( ) {
var xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function() {
if (this.readyState == 4 && this.status == 200) {
var x = (new Date()).getTime(),
y = parseFloat(this.responseText);
//console.log(this.responseText);
if(chartH.series[0].data.length > 200) {
chartH.series[0].addPoint([x, y], true, true, true);
} else {
chartH.series[0].addPoint([x, y], true, false, true);
}
}
};
xhttp.open("GET", "/humidity", true);
xhttp.send();
}, 2000 ) ;

var chartAQ = new Highcharts.Chart({
chart:{ renderTo : 'airquality-chart' },
title: { text: 'Air Quality in ppm' },
series: [{
showInLegend: false,
data: []
}],
plotOptions: {
line: { animation: false,
dataLabels: { enabled: true }
},
series: { color: '#F16547' }
},
xAxis: { type: 'datetime',
dateTimeLabelFormats: { second: '%H:%M:%S' }
},
yAxis: {
title: { text: 'Air Quality (ppm)' }
},
credits: { enabled: false }
});
setInterval(function ( ) {
var xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function() {
if (this.readyState == 4 && this.status == 200) {
var x = (new Date()).getTime(),
y = parseFloat(this.responseText);
if(chartAQ.series[0].data.length > 200) {
chartAQ.series[0].addPoint([x, y], true, true, true);
} else {
chartAQ.series[0].addPoint([x, y], true, false, true);
}
}
};
xhttp.open("GET", "/airquality", true);
xhttp.send();
}, 2000 ) ;

var chartOAQ = new Highcharts.Chart({
chart:{ renderTo:'overallquality-chart' },
title: { text: 'Overall Quality (%)' },
series: [{
showInLegend: false,
data: []
}],
plotOptions: {
line: { animation: false,
dataLabels: { enabled: true }
},
series: { color: '#18009c' }
},
xAxis: {
type: 'datetime',
dateTimeLabelFormats: { second: '%H:%M:%S' }
},
yAxis: {
title: { text: 'Overall Quality (%)' }
},
credits: { enabled: false }
});
setInterval(function ( ) {
var xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function() {
if (this.readyState == 4 && this.status == 200) {
var x = (new Date()).getTime(),
y = parseFloat(this.responseText);
//console.log(this.responseText);
if(chartOAQ.series[0].data.length > 200) {
chartOAQ.series[0].addPoint([x, y], true, true, true);
} else {
chartOAQ.series[0].addPoint([x, y], true, false, true);
}
}
};
xhttp.open("GET", "/overallquality", true);
xhttp.send();
}, 2000 ) ;

</script>
</html>)webpage";

void setup(){
  
  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode (BUZZERPIN ,OUTPUT);
  digitalWrite(BUZZERPIN,LOW);


  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.println(".");
}

// Print ESP32 Local IP Address
Serial.println(WiFi.localIP());

// Route for root / web page
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
request->send_P(200, "text/html", index_html);
});
server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
request->send_P(200, "text/plain", String(t).c_str());
});
server.on("/fahrenheit", HTTP_GET, [](AsyncWebServerRequest *request){
request->send_P(200, "text/plain", String(tf).c_str());
});
server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
request->send_P(200, "text/plain", String(h).c_str());
});
server.on("/airquality", HTTP_GET, [](AsyncWebServerRequest *request){
request->send_P(200, "text/plain", String(ppm).c_str());
});
server.on("/overallquality", HTTP_GET, [](AsyncWebServerRequest *request){
request->send_P(200, "text/plain", String(oaq).c_str());
});

// Start server
server.begin();
dht.begin();
}

void loop(){ 
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float currentT = dht.readTemperature();
    
    // if temperature read failed, we don't want to change t value
    if (isnan(currentT)) {
      Serial.println("Failed to read from DHT sensor!");
      Serial.println(currentT);
      }
    else {
      t = currentT;
      Serial.println(t);
      }
    // Read temperature as Fahrenheit 
    float currentTf = dht.readTemperature(true);
    // if temperature read failed, we don't want to change tf value
    if (isnan(currentTf)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      tf = currentTf;
      Serial.println(tf);
      }


  // Read Humidity
  float currentH = dht.readHumidity();
  // if humidity read failed, we don't want to change h value 
  if (isnan(currentH)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  else {
    h = currentH;
    Serial.println(h);
    }
  delay(500);
  float currentPPM = gasSensor.getPPM();
  // if air quality read failed, we don't want to change ppm value 
  if (isnan(currentPPM)) {
    Serial.println("Failed to read from MQ-135 sensor!");
    }
   else {
   ppm = currentPPM;
   Serial.println(ppm);
    }

    // Sıcaklık kalite parametresinin ayarlanması
    if(t < 25) 
    {
      t_qual = t * 4;
    }else if( t == 25){
      t_qual = 100;
    }else{
      t_qual = (50 - t)*4;
    }

    // Nem kalite parametresinin ayarlanması
    if(h < 40)
    {
      h_qual = h * 2.5;
    }else if( h == 40){
      h_qual = 100;
    }else{
      h_qual = (80 - h)*2.5;
    }

    // Hava kalitesi prametresinin ayarlanması
    if(ppm < 300)
    {
      ppm_qual = 100;
    }else{
      ppm_qual = (600 - ppm)*0.5;
    }
    
    // Overall Quality
    oaq = (ppm_qual*0.4) + (t_qual* 0.3) + (h_qual * 0.3);
    if(oaq < 0)
      oaq = 0;
    if(oaq >100)
      oaq = 100;
    if( t > 50 || h > 80 || ppm > 1000 || oaq < 40)
    {
        digitalWrite(BUZZERPIN,HIGH); //Turn on active buzzer
        delay(200);         
        digitalWrite(BUZZERPIN,LOW);
    }
  }
}
