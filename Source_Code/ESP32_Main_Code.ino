#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

const char* ssid="project";
const char* password="12345678";

WebServer server(80);

// MOTOR PINS
#define IN1 14
#define IN2 12
#define IN3 13
#define IN4 15

// ULTRASONIC
#define TRIG1 5
#define ECHO1 18
#define TRIG2 19
#define ECHO2 23

String mode="MANUAL";
String movement="STOP";

long frontDist=0;
long sideDist=0;


// ================= MOTOR =================

void stopMotor(){

digitalWrite(IN1,LOW);
digitalWrite(IN2,LOW);
digitalWrite(IN3,LOW);
digitalWrite(IN4,LOW);

movement="STOP";

}

void forward(){

digitalWrite(IN1,HIGH);
digitalWrite(IN2,LOW);
digitalWrite(IN3,HIGH);
digitalWrite(IN4,LOW);

movement="FWD";

}

void reverse(){

digitalWrite(IN1,LOW);
digitalWrite(IN2,HIGH);
digitalWrite(IN3,LOW);
digitalWrite(IN4,HIGH);

movement="REV";

}

void left(){

digitalWrite(IN1,LOW);
digitalWrite(IN2,HIGH);
digitalWrite(IN3,HIGH);
digitalWrite(IN4,LOW);

movement="LEFT";

}

void right(){

digitalWrite(IN1,HIGH);
digitalWrite(IN2,LOW);
digitalWrite(IN3,LOW);
digitalWrite(IN4,HIGH);

movement="RIGHT";

}


// ================= ULTRASONIC =================

long distance(int trig,int echo){

digitalWrite(trig,LOW);
delayMicroseconds(2);

digitalWrite(trig,HIGH);
delayMicroseconds(10);
digitalWrite(trig,LOW);

long duration=pulseIn(echo,HIGH);

return duration*0.034/2;

}


// ================= AUTO MODE =================

void autonomous(){

frontDist=distance(TRIG1,ECHO1);
sideDist=distance(TRIG2,ECHO2);

if(frontDist<30){

right();
delay(400);

}

else if(sideDist<25){

left();
delay(400);

}

else{

forward();

}

}


// ================= LCD =================

void updateLCD(){

lcd.clear();

lcd.setCursor(0,0);
lcd.print("Mode:");
lcd.print(mode);
lcd.print(" ");
lcd.print(movement);

lcd.setCursor(0,1);
lcd.print("F:");
lcd.print(frontDist);
lcd.print(" S:");
lcd.print(sideDist);

}


// ================= STATUS API =================

void handleStatus(){

frontDist=distance(TRIG1,ECHO1);
sideDist=distance(TRIG2,ECHO2);

String json="{";

json+="\"mode\":\""+mode+"\",";
json+="\"move\":\""+movement+"\",";
json+="\"front\":"+String(frontDist)+",";
json+="\"side\":"+String(sideDist);

json+="}";

server.send(200,"application/json",json);

}


// ================= WEB COMMANDS =================

void handleForward(){

if(mode=="MANUAL"){

forward();
updateLCD();

}

server.send(200,"text/plain","OK");

}

void handleReverse(){

if(mode=="MANUAL"){

reverse();
updateLCD();

}

server.send(200,"text/plain","OK");

}

void handleLeft(){

if(mode=="MANUAL"){

left();
updateLCD();

}

server.send(200,"text/plain","OK");

}

void handleRight(){

if(mode=="MANUAL"){

right();
updateLCD();

}

server.send(200,"text/plain","OK");

}

void handleStop(){

stopMotor();
updateLCD();

server.send(200,"text/plain","OK");

}

void handleManual(){

mode="MANUAL";
stopMotor();
updateLCD();

server.send(200,"text/plain","MANUAL");

}

void handleAuto(){

mode="AUTO";
updateLCD();

server.send(200,"text/plain","AUTO");

}


// ================= DASHBOARD =================

void handleRoot(){

String page=R"rawliteral(

<!DOCTYPE html>
<html>

<head>

<meta name="viewport" content="width=device-width,initial-scale=1">

<style>

body{
margin:0;
font-family:Arial;
color:white;
background:url('https://images.unsplash.com/photo-1507149833265-60c372daea22') no-repeat center center fixed;
background-size:cover;
}

.container{
display:flex;
height:100vh;
}

.left{
width:40%;
text-align:center;
padding-top:40px;
background:rgba(0,0,0,0.7);
}

.right{
width:60%;
display:flex;
align-items:center;
justify-content:center;
}

button{
width:100px;
height:60px;
margin:10px;
font-size:18px;
border-radius:10px;
background:#444;
color:white;
}

iframe{
width:90%;
height:80%;
border:3px solid white;
border-radius:10px;
}

.status{
font-size:22px;
margin:20px;
}

</style>

</head>

<body>

<div class="container">

<div class="left">

<h1>AI SURVEILLANCE ROBOT</h1>

<div class="status">

<div id="line1">Mode:-- M:--</div>
<div id="line2">F:-- S:--</div>

</div>

<h3>MODE</h3>

<button onclick="send('manual')">MANUAL</button>
<button onclick="send('auto')">AUTO</button>

<h3>CONTROL</h3>

<button onclick="send('forward')">UP</button><br>

<button onclick="send('left')">LEFT</button>
<button onclick="send('stop')">STOP</button>
<button onclick="send('right')">RIGHT</button><br>

<button onclick="send('reverse')">DOWN</button>

</div>


<div class="right">

<iframe src="http://10.210.172.194/"></iframe>

</div>

</div>

<script>

function send(cmd){

fetch('/'+cmd);

}

function updateStatus(){

fetch('/status')

.then(r=>r.json())

.then(d=>{

document.getElementById("line1").innerHTML="Mode:"+d.mode+"  M:"+d.move;
document.getElementById("line2").innerHTML="F:"+d.front+"  S:"+d.side;

});

}

setInterval(updateStatus,1000);

</script>

</body>
</html>

)rawliteral";

server.send(200,"text/html",page);

}


// ================= SETUP =================

void setup(){

Serial.begin(115200);

pinMode(IN1,OUTPUT);
pinMode(IN2,OUTPUT);
pinMode(IN3,OUTPUT);
pinMode(IN4,OUTPUT);

pinMode(TRIG1,OUTPUT);
pinMode(ECHO1,INPUT);

pinMode(TRIG2,OUTPUT);
pinMode(ECHO2,INPUT);

stopMotor();

Wire.begin(21,22);

lcd.init();
lcd.backlight();

lcd.clear();

lcd.setCursor(0,0);
lcd.print("Autonomous surve");
lcd.setCursor(0,1);
lcd.print("Rover");
delay(2000);
lcd.clear();
lcd.print("Connecting WiFi");

WiFi.begin(ssid,password);

while(WiFi.status()!=WL_CONNECTED){
delay(2000);
}

lcd.clear();
lcd.print(WiFi.localIP());

delay(5000);

updateLCD();

server.on("/",handleRoot);

server.on("/forward",handleForward);
server.on("/reverse",handleReverse);
server.on("/left",handleLeft);
server.on("/right",handleRight);
server.on("/stop",handleStop);

server.on("/manual",handleManual);
server.on("/auto",handleAuto);

server.on("/status",handleStatus);

server.begin();

}


// ================= LOOP =================

void loop(){

server.handleClient();

if(mode=="AUTO"){

autonomous();
updateLCD();

}

}
