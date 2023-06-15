#include <ESP8266WiFi.h>//WiFi Library
#include<SoftwareSerial.h>//Serial communication library
SoftwareSerial s(3,1);//Setting serial pins

const char WiFiPassword[] = "multitaskr";//Password
const char AP_NameChar[] = "Base_Rotatoria" ;//WiFi red name 
WiFiServer server(80);
 
 
String request = "";//Client string 
String valstr= "";//Clean string

int reset= true;//Reset confirmation for first command error fix
int pwmz = 180;//Ángulo en eje x
int pos1= 0;//Refrencia 1 para limpiado de string
int pos2= 0;//referencia 2 para limpiado de string
int pwm;//Ángulo eje x
int velx= 0;//Delay velocidad eje x

 
void setup() 
{
  s.begin(9600);

  //Inicialización de server WiFi
  boolean conn = WiFi.softAP(AP_NameChar, WiFiPassword);
  server.begin();  

} // void setup()
 
 
void loop() 
{
    if(reset==true){//Confirmación de reset
      s.println('b'); 
      reset=false;
    }
   
    // Check if a client has connected
    WiFiClient client = server.available();//Confiermación de cliente conectado
    if (!client)  {  return;  }


    // Read the first line of the request
    request = client.readStringUntil('\r');//Lectura de string en buffer de cliente

    
    if       (request.indexOf("GET /angleX=")>=0)  { //Recepción de ángulo en x
      //Limpiado de string    
      pos1 = request.indexOf('=');
      pos2 = request.indexOf('w');
      valstr = request.substring(pos1+1, pos2);
      pwm=  valstr.toInt(); 
      if (pwm>=45 && pwm<=135){
        s.print('x'+valstr+'\n');   //Envio de ángulo y eje a Arduino  
      }   
    }
    else if (request.indexOf("GET /angleZ=")>=0) {//Recepción de ángulo en z
      //Limpiado de string   
      pos1 = request.indexOf('=');
      pos2 = request.indexOf('w');
      valstr = request.substring(pos1+1, pos2);
      pwmz=  valstr.toInt();   
      if (pwmz>=0 && pwmz<=360){
        s.print('z'+valstr+'\n'); //Envio de ángulo y eje a Arduino               
      }
    }
    else if (request.indexOf("GET /rpm=")>=0){//Recepción de velocidad en z
      pos1 = request.indexOf('=');
      pos2 = request.indexOf('w');
      valstr = request.substring(pos1+1, pos2);
      velx=  valstr.toInt();     
      s.print('p'+valstr+'\n'); //Envio de delay en velocidad de eje x
    }    

    
    client.flush();//Limpiado de información pendiente en buffer de cliente 
 
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println(pwm-90);//Envío de ángulo seleccionado a cliente
 
    delay(5);
  // The client will actually be disconnected when the function returns and 'client' object is detroyed
 
} // void loop()
