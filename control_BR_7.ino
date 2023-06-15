#include <Servo.h> //Libería Servomotores
#include<EEPROM.h>// Librería Electrically Erasable Programmable Read-Only Memory

//Inicialización de Servos
Servo bas; //Eje z
Servo der; //Eje x derecho
Servo izq; //Eje x izquerdo 

char c; //Carácter recibido por ESP8266
String dataIn; //Unión de carácteres 

//Inputs de lectura de ecoder 
const int encoder_a = 2; // Green - pin 2 - Digital
const int encoder_b = 3; // White - pin 3 - Digital
long encoder = 0;

//Eje z
int velAH=88; //Velocidad positiva
int velSH=100; //Velocidad negativa
int posicion; //Posición actual
int posicionAnt;//Posición anterior 


String request = "";//Comando de ángulo y eje
String valstr= "";//Variable para transformación a integer 
int i=0;//Contador ciclos de giro de eje x
int anglex;//Ángulo en eje x
int velx= 30;//Delay en velocidad de giro 

double setpoint = 0;//Ángulo en eje z



void setup() {
  Serial.begin(9600);
  //Obtener último valor de de ángulo en x de EEPROM
  int readPrev;  
  EEPROM.get(10, readPrev);

 //Obtener último valor de de ángulo en z de EEPROM
  int readPrev2;  
  EEPROM.get(20, readPrev2);
  encoder=readPrev2;
  
  //Inicialización de pines de lectura de encoder 
  pinMode(encoder_a, INPUT_PULLUP);
  pinMode(encoder_b, INPUT_PULLUP);

  attachInterrupt(0, encoderPinChangeA, CHANGE);
  attachInterrupt(1, encoderPinChangeB, CHANGE);

  //Inicialización de pines de control PWM de servos
  der.write(readPrev);
  izq.write(180-readPrev);
  bas.attach(13);
  der.attach(7);
  izq.attach(8);

  
}
int newencoder;

void loop() 
{ 
  while(Serial.available()>0){//Recepción de carácteres via UART (Rx) de ESP8266 (Tx)
    c=Serial.read();
    if(c=='\n'){break;}  
    else {dataIn+=c;}

  }

    int readPrev;  
    EEPROM.get(10, readPrev);

    int prev = readPrev; 

  request=dataIn;

  //Lectura de posición del motor
  posicion = encoder*90/600;
  EEPROM.put(20,encoder); //Guardado de última posición de eje z en EEPROM

    if       (request.indexOf("x")>=0)  { //Movimiento eje x
      //Lectura  de ángulo y transformaión a integer de variable
      request.remove(0,1);
      valstr=request;
      anglex=  valstr.toInt(); 
      if (anglex>=45 && anglex<=135){//Verificación de input
        if (anglex> prev){//Detección de sentido de giro
          for (i=prev; i<=anglex; i++){//Ciclo para realizar el movimiento angular paso por paso
            der.write(i);
            izq.write(180-i);
            delay(velx);//Velocidad de giro 
          }
                    
        }
        else if (anglex< prev){//Detección de sentido de giro
          for (i=prev; i>=anglex; i--){//Ciclo para realizar el movimiento angular paso por paso
            der.write(i);
            izq.write(180-i);
            delay(velx);//Velocidad de giro 
          }
        }
      }
      prev=anglex;//Guardado de posición anterior 
      EEPROM.put(10,prev);  //Guardado de última posición en EEPROM 
      c=0;
      dataIn="";   //Borrado de comando de ángulo para recibir nuevo 
    }
    else if (request.indexOf("z")>=0) { //Movimiento eje z
      //Lectura  de ángulo y transformaión a integer de variable
      request.remove(0,1);
      valstr=request;
      setpoint=  valstr.toInt(); 
      if(abs(setpoint-posicion) <= 7){ //Si ya se llegó a la posición deseada   
        bas.write(90); //Servo detenido
        c=0;
        dataIn="";
      }
      else{ //Si la posición no es la deseada
        if(setpoint>posicion){
          // Relaciones: 200/20=10° (39 pasos) 300/20=10°(23 pasos y se pasa en el último)  400/25=20°
          bas.write(velAH); //Entre 0 (+) y 90 (-) (antihorario)
          posicionAnt=posicion; 
          if(abs(setpoint-posicion) <= 40){
            while(posicion-posicionAnt<=10){  
              posicionAnt=posicion;    
              bas.write(velAH); //Entre 0 (+) y 90 (-) (antihorario)
              break;
            }
            delay(200);
            bas.write(90); //Servo detenido
            c=0;
            dataIn="";
          }
        }
        else if(setpoint<posicion){  
          bas.write(velSH); //Entre 90 (-) y 180 (+)(horario)
          posicionAnt=posicion;  
          if(abs(setpoint-posicion) <= 40){ 
            while(posicionAnt-posicion <= 10){
              posicionAnt=posicion;
              bas.write(velSH); //Entre 90 (-) y 180 (+)(horario)  
              break;
            }
            delay(200);
            bas.write(90);
            c=0;
            dataIn="";
          }
        }    
      }     
    }
    else if (request.indexOf("p")>=0)  { //cambio de velocidad en eje x
      request.remove(0,1);
      valstr=request;
      velx=  valstr.toInt(); 
      velx=(3+velx*0.7)*10;
      c=0;
      dataIn="";
    }  

  delay(500);
}

//Interrupciones lectura de encoder
void encoderPinChangeA() {
  encoder += digitalRead(encoder_a) == digitalRead(encoder_b) ? -1 : 1;
}

void encoderPinChangeB() {
  encoder += digitalRead(encoder_a) != digitalRead(encoder_b) ? -1 : 1;
}