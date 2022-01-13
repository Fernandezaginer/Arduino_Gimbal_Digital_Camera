// Programa arduino gimbal
// Escuela Técnica Superior de Ingeniería y Diseño Industrial
// Universidad Politécnica de Madrid

// Trabajo libre con arduino
// Estabilizador casero para cámara de fotos (Gimbal)
// Alumno:       Andrés Fernández Muñoz (55237)
// Grado:        Ing. Electrónica Automática e Industrial
// Asignatura:   Regulación Automática
// Curso:        2021/2022



// Librerías
#include <Servo.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <Wire.h>


// Conexiones:
#define PIN_EJE1 15
#define PIN_EJE2 16
#define PIN_EJE3 17
#define LED_R 13
#define LED_G 14

// MPU SDA - Pin A4
// MPU SCL - Pin A5
// BT   TX - Pin D0 RX
// BT   RX - Pin D0 TX


// Servomotores
Servo eje_1;
Servo eje_2;
Servo eje_3;

#define ANG_0_EJE1 86
#define ANG_0_EJE2 92
#define ANG_0_EJE3 85

byte angulo_1 = ANG_0_EJE1;
byte angulo_2 = ANG_0_EJE2;
byte angulo_3 = ANG_0_EJE3;
float ang_1f = ANG_0_EJE1;
float ang_2f = ANG_0_EJE2;
float ang_3f = ANG_0_EJE3;


// MPU
const int mpuAddress = 0x68;
const float accScale = 2.0 * 9.81 / 32768.0;
const float gyroScale = 250.0 / 32768.0;

MPU6050 mpu(mpuAddress);


// Datos matlab:
#define BPS 9600
#define INTERVALO_ENVIO 100
unsigned long envio_anterior = 0;



void setup()
{

  // Puerto serie para la comunicación con Matlab
  Serial.begin(BPS);


  // Leds de estado
  pinMode(LED_G, OUTPUT);
  pinMode(LED_R, OUTPUT);
  

  // Setup MPU
  Wire.begin();
  delay(1000);
  // Comprobar que responde el MPU
  // (por si se desconectasen los cables, parar la inicialización para evitar que se descontrole)
  Wire.beginTransmission(mpuAddress);
  if (Wire.endTransmission() == 0) {
    digitalWrite(LED_G, 1);
  }
  else{
    while (true){
      digitalWrite(LED_R, 1);
      delay(200);
      digitalWrite(LED_R, 0);
      delay(200);
    }
  }

  mpu.initialize();

  delay(1000);
  // Serial.println(mpu.testConnection() ? F("IMU iniciado correctamente") : F("Error al iniciar IMU"));

  // Inicializar servos
  eje_1.attach(PIN_EJE1);
  eje_2.attach(PIN_EJE2);
  eje_3.attach(PIN_EJE3);

  eje_1.write(ANG_0_EJE1);
  eje_2.write(ANG_0_EJE2);
  eje_3.write(ANG_0_EJE3);


}

void loop()
{

  // LECTURA DE LOS ÁNGULOS DEL LA CÁMARA
  int32_t aax = 0;
  int32_t aay = 0;
  int32_t aaz = 0;
  int ax, ay, az;

  // Breve algoritmo de filtrado de ruido del acelerómetro (media ponderada)
  for (int i = 0; i < 40; i++) {
    mpu.getAcceleration(&ax, &ay, &az);
    aax += ax;
    aay += ay;
    aaz += az;
    delay(0.5);
  }
  aax = aax / 40;
  aay = aay / 40;
  aaz = aaz / 40;

  float ang1 = atan((aay * accScale) / (-(aaz * accScale))) * (180.0 / 3.1416);
  float ang2 = atan((aax * accScale) / (-(aaz * accScale))) * (180.0 / 3.1416);

  //Serial.print(ang1);
  //Serial.print('\t');
  //Serial.print(ang2);
  //Serial.print('\t');



  // ENVIO DE DATOS A MATLAB:
  if ((envio_anterior + INTERVALO_ENVIO) < millis()) {
    envio_anterior = millis();
    Serial.print(ang1, 3);
    Serial.write(' ');
    Serial.print(ang2, 3);
    Serial.println(' ');
    digitalWrite(LED_G, !digitalRead(LED_G));
  }



  // COMPENSACIÓN DEL ERROR EJE PITCH:
  if (ang1 > 2.0) {
    
    if(ang1 < 10.0){
      ang_1f = ang_1f - 1;      
    }
    else{
      ang_1f = ang_1f - ang1*0.1;    
    }
    angulo_1 = (int)ang_1f;
    eje_1.write(angulo_1);
  }

  if (ang1 < -2.0) {
    
    if(ang1 > -10.0){
      ang_1f = ang_1f + 1;      
    }
    else{
      ang_1f = ang_1f + abs(ang1)*0.1;    
    }
    angulo_1 = (int)ang_1f;
    eje_1.write(angulo_1);
  }

  //Serial.println(angulo_1);




  // COMPENSACIÓN DEL ERROR EJE ROLL:
  if (ang2 > 3.0) {

    // Interpolación de varios ejes de giro:
    if (angulo_1 >= (ANG_0_EJE1 + 15) && angulo_1 <= (180 - 20)) {
      ang_2f = ang_2f + (0.001 * map(angulo_1, ANG_0_EJE1, 180, 1000, 0));
      ang_3f = ang_3f - (0.001 * map(angulo_1, ANG_0_EJE1, 180, 0, 1000));
    }
    else if (angulo_1 <= (ANG_0_EJE1 - 15) && angulo_1 > 20) {
      ang_2f = ang_2f + (0.001 * map(angulo_1, 0, ANG_0_EJE1, 1000, 0));
      ang_3f = ang_3f + (0.001 * map(angulo_1, 0, ANG_0_EJE1, 0, 1000));
    }

    // Eje único
    else if (angulo_1 > (180 - 20)) {
      ang_3f = ang_3f + (0.001 * map(angulo_1, ANG_0_EJE1, 180, 0, 1000));
    }
    else if (angulo_1 < 20) {
      ang_3f = ang_3f - (0.001 * map(angulo_1, 0, ANG_0_EJE1, 0, 1000));
    }
    else {
      ang_2f = ang_2f + (0.001 * map(angulo_1, ANG_0_EJE1, 180, 1000, 0));
    }

    angulo_2 = (int)ang_2f;
    eje_2.write(angulo_2);
    angulo_3 = (int)ang_3f;
    eje_3.write(angulo_3);
  }


  if (ang2 < -3.0) {

    // Interpolación de varios ejes de giro:
    if (angulo_1 >= (ANG_0_EJE1 + 15) && angulo_1 <= (180 - 20)) {
      ang_2f = ang_2f - (0.001 * map(angulo_1, ANG_0_EJE1, 180, 1000, 0));
      ang_3f = ang_3f + (0.001 * map(angulo_1, ANG_0_EJE1, 180, 0, 1000));
    }

    else if (angulo_1 <= (ANG_0_EJE1 - 15) && angulo_1 > 20) {
      ang_2f = ang_2f - (0.001 * map(angulo_1, 0, ANG_0_EJE1, 1000, 0));
      ang_3f = ang_3f - (0.001 * map(angulo_1, 0, ANG_0_EJE1, 0, 1000));
    }

    // Eje único
    else if (angulo_1 > (180 - 20)) {
      ang_3f = ang_3f - (0.001 * map(angulo_1, ANG_0_EJE1, 180, 0, 1000));
    }
    else if (angulo_1 < 20) {
      ang_3f = ang_3f + (0.001 * map(angulo_1, 0, ANG_0_EJE1, 0, 1000));
    }
    else {
      ang_2f = ang_2f - (0.001 * map(angulo_1, ANG_0_EJE1, 180, 1000, 0));
    }

    angulo_2 = (int)ang_2f;
    eje_2.write(angulo_2);
    angulo_3 = (int)ang_3f;
    eje_3.write(angulo_3);

  }

}
