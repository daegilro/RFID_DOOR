//-----------------Librerias---------------------//
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
//#include <LiquidCrystal_I2C.h>
#include <time.h>
#include <ESP32Time.h>
ESP32Time rtc;
#include <esp_now.h>
#include <WiFi.h>

//--------Definición Pantalla LCD---------------//
//LiquidCrystal_I2C lcd(0x27, 20, 4);

//-------------Definición Pines-----------------//
#define pinSCK  18
#define  pinMISO  19
#define  pinMOSI  23
#define  pinRST  17
#define  pinSDA  5
#define  signal_relay  27
#define  led  2
#define  push_out  32
#define  led_close  25 
#define  led_open  33 
#define  pinSDA2  4 

//-------------Creación objet RFID-------------//
MFRC522 rfid(pinSDA, pinRST);
MFRC522 rfid2(pinSDA2, pinRST);
int nuidPICC[4] = {0, 0, 0, 0};
int nuidPICC2[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key_A;
MFRC522::MIFARE_Key key_B;

//-------------Variables Fecha-Hora -------------//

String dateesp32;
int horas;
int minutos;
String horacompleta;
String entrada = "ENTRADA ";
String salida = "SALIDA ";
String NoUser = "Usuario no encontrado ";
bool registro = false;

//-------------Lista ingreso permitido-------------//

const char* nombres[] =  {"    David Gil R.    ",
                         "  Cristian Tejedor  ",
                         "    Diego Basto     ",
                         "    Daniel Felipe   ",
                         "  Jonathan Penagos  ",
                         "    Llave Oso       ",
                         "   Llave Baston     ",
                         "  Sergio Manjarres  ",
                         "   Alberto Garzon   ",
                         "    Edgar Molina    ",
                         "     Invitado 1     ",
                         "     Invitado 2     ",
                         "   Diego Gonzales   ",
                         "  Anyeli Rodriguez  ",
                         "    Juan Hurtado    ",
                         "    Carlos Roman    ",
                         "    Ronald Garzon   ",
                         "    Tatiana Bernal  ",
                         "     Juan Arroyo    ",
                         "    Luis Rodriguez  ",
                         "  Santiago Vasquez  ",
                         "    Laura Ortegon   ",
                         "    Julian Ayala    ",
                         "    Marion Bonilla  ",
                         "     Diego Riaño    ",
                         "     Edwin Prieto   ",
                         "   Andres Castillo  ",
                         "  Julio Colmenares  ",
                         "   Kevin Morales    ",
                         "   Sofia Herrera    "
                        };
int usuario[4] = {0, 0, 0, 0};
const int lista[][4] = {{92, 249, 134, 171},
                        {77, 230, 161, 234},
                        {214, 171, 37, 115},
                        {109, 198, 168, 234},
                        {78, 14, 147, 81},
                        {28, 60, 9, 56},
                        {108, 200, 232, 56},
                        {235, 223, 219, 175},
                        {219, 5, 210, 175},
                        {107, 2, 207, 175},
                        {67, 83, 113, 25},
                        {147, 142, 42, 21},
                        {226, 220, 85, 111},
                        {251, 122, 205, 175},
                        {254, 215, 114, 81},
                        {27, 224, 209, 175},
                        {59, 68, 60, 64},
                        {126, 193, 116, 81},
                        {118,102,38,115},
                        {82,67,234,114},
                        {27,73,55,64},
                        {214,229,04,104},
                        {30,195,117,81},
                        {109,182,50,169},
                        {110,252,115,81},
                        {44,145,196,208},
                        {14,92,122,81},
                        {155,254,210,175},
                        {146,155,168,128},
                        {141,37,75,45}
                        };

//----------Info Device Recived---------------//
// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xC0, 0x49, 0xEF, 0xCB, 0x93, 0x50};

//-------- Structure example to send data-------//
// Must match the receiver structure
typedef struct struct_message {
  char a[21];
  char b[8];
  int c;
  int d;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  SPI.begin();//Inicio Comunicación SPI
  rfid.PCD_Init(); //Inicio Comunicación RFID
  rfid2.PCD_Init(); //Inicio Comunicación RFID
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  

  pinMode(signal_relay, OUTPUT);
  pinMode(led_open, OUTPUT);
  pinMode(led_close, OUTPUT);
  pinMode(push_out, INPUT);
  digitalWrite(signal_relay, LOW);
  digitalWrite(led_close, HIGH);
  digitalWrite(led_open, LOW);
  digitalWrite(led_close, LOW);
  delay(300);
  digitalWrite(led_close, HIGH);
  delay(300);
  digitalWrite(led_close, LOW);
  delay(300);
  digitalWrite(led_close, HIGH);
  rtc.setTime(0,0,0,31,8,2022);
  dateesp32 = rtc.getDate();
  horas = rtc.getHour(true);
  minutos = rtc.getMinute();
  Serial.println(dateesp32 + " " + horas +":"+ minutos); 
  //strcpy(myData.a, "Correcto");
  //strcpy(myData.b, "Correcto");
  //myData.c = horas;
  //myData.d = minutos;
  // Send message via ESP-NOW
  //esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
}
 
void loop() {

  Serial.println("Leyendo_horario");
  dateesp32 = rtc.getDate();
  horas = rtc.getHour(true);
  minutos = rtc.getMinute();
  horacompleta = rtc.getTime();
//  lcd.clear();
//  //lcd.home();
//  lcd.setCursor (0, 0);//poner el cursor en las coordenadas (x,y)
//  lcd.print(" LAB GEOTECNIA UNAL ");//muestra en la pantalla max 20 caracteres
//  //delay(50);
//  lcd.setCursor (0, 1);//poner el cursor en las coordenadas (x,y)
//  lcd.print("    Bienvenido!     ");//muestra en la pantalla max 20 caracteres
//  //delay(50);
//  lcd.setCursor (0, 2);//poner el cursor en las coordenadas (x,y)
//  lcd.print("     Hora-Fecha");//muestra en la pantalla max 20 caracteres
//  //delay(50);
//  lcd.setCursor (0, 3);//poner el cursor en las coordenadas (x,y)
//  lcd.print(" Acerque su tarjeta ");//muestra en la pantalla max 20 caracteres
//  //delay(50);

  registro = false;
  for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = 0;
        nuidPICC2[i] = 0;
      }

  for (byte i = 0; i < 6; i++) {
    key_A.keyByte[i] = 0xFF;
    }
    
  Serial.println("Lectura Tarjeta 1");
  if (rfid.PICC_IsNewCardPresent()) { //Si detecta una tarjeta
    if (rfid.PICC_ReadCardSerial()) { //Lee la tarjeta
      
      for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = rfid.uid.uidByte[i]; //Obtiene información de la tarjeta
      }

      //lcd.setCursor (0, 3);//poner el cursor en las coordenadas (x,y)
      //lcd.print("   Tarjeta leida!   ");//muestra en la pantalla max 20 caracteres
      delay(20);
      for (byte x = 0; x < 33; x++ ) {
        usuario[0] = lista[x][0];
        usuario[1] = lista[x][1];
        usuario[2] = lista[x][2];
        usuario[3] = lista[x][3];
                
        if (usuario[0] == nuidPICC[0] &&
            usuario[1] == nuidPICC[1] &&
            usuario[2] == nuidPICC[2] &&
            usuario[3] == nuidPICC[3]) { //Busqueda del ID entre los ID permitidos
         
          
//          lcd.clear();
//          lcd.home();
//          lcd.setCursor (0, 0);//poner el cursor en las coordenadas (x,y)
//          lcd.print(" LAB GEOTECNIA UNAL ");//muestra en la pantalla max 20 caracteres
//          delay(50);
//          lcd.setCursor (0, 1);//poner el cursor en las coordenadas (x,y)
//          lcd.print("    Bienvenido!     ");//muestra en la pantalla max 20 caracteres
//          lcd.setCursor (0, 2);//poner el cursor en las coordenadas (x,y)
//          lcd.print(nombres[x]);//muestra en la pantalla max 20 caracteres
          registro = true;
          Serial.println("Leyendo");
          // Set values to send
          strcpy(myData.a, nombres[x]);
          strcpy(myData.b, "Entrada");
          myData.c = horas;
          myData.d = minutos;
          // Send message via ESP-NOW
          esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));  
          //Desenergizar iman        
          open_door();
          rfid.PICC_HaltA(); // halt PICC
          rfid.PCD_StopCrypto1();
            //}
          break;
        }
        if(x==32){
          Serial.println("No Leyendo");
          strcpy(myData.a, "Usuario no encontrado ");
          strcpy(myData.b, "Entrada");
          myData.c = horas;
          myData.d = minutos;
          esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
        
      
        
        //lcd.setCursor (0, 2);//poner el cursor en las coordenadas (x,y)
        //lcd.print(" No User ");//muestra en la pantalla max 20 caracteres
        //delay(100);
          digitalWrite(led_close, HIGH);
          delay(300);
          digitalWrite(led_close, LOW);
          delay(300);
          digitalWrite(led_close, HIGH);
          delay(300);
          digitalWrite(led_close, LOW);
          delay(300);
          digitalWrite(led_close, HIGH);
          delay(300);
          digitalWrite(led_close, LOW);
          
          rfid.PICC_HaltA(); // halt PICC
          rfid.PCD_StopCrypto1();
          break;
          }
        
      }
        
        }
     }

  for (byte i = 0; i < 6; i++) {
    key_B.keyByte[i] = 0xFF;
    }
  Serial.println("Leyendo Tarjeta 2");
  if (rfid2.PICC_IsNewCardPresent()) { //Si detecta una tarjeta
    if (rfid2.PICC_ReadCardSerial()) { //Lee la tarjeta
      //Serial.print(("PICC type: "));
      //MFRC522::PICC_Type piccType2 = rfid2.PICC_GetType(rfid2.uid.sak);
      //Serial.println(rfid.PICC_GetTypeName(piccType));
      //Serial.println("UID Tarjeta: ");
      for (byte i = 0; i < 4; i++) {
        nuidPICC2[i] = rfid2.uid.uidByte[i]; //Obtiene información de la tarjeta
      }
      //lcd.setCursor (0, 3);//poner el cursor en las coordenadas (x,y)
      //lcd.print("   Tarjeta leida!   ");//muestra en la pantalla max 20 caracteres
      delay(200);
      for (byte x = 0; x < 33; x++ ) {
        //Serial.println("Buscando...");
        usuario[0] = lista[x][0];
        usuario[1] = lista[x][1];
        usuario[2] = lista[x][2];
        usuario[3] = lista[x][3];
                
        if (usuario[0] == nuidPICC2[0] &&
            usuario[1] == nuidPICC2[1] &&
            usuario[2] == nuidPICC2[2] &&
            usuario[3] == nuidPICC2[3]) { //Busqueda del ID entre los ID permitidos          
//          lcd.clear();
//          lcd.home();
//          lcd.setCursor (0, 0);//poner el cursor en las coordenadas (x,y)
//          lcd.print(" LAB GEOTECNIA UNAL ");//muestra en la pantalla max 20 caracteres
//          delay(50);
//          lcd.setCursor (0, 1);//poner el cursor en las coordenadas (x,y)
//          lcd.print("    Bienvenido!     ");//muestra en la pantalla max 20 caracteres
//          lcd.setCursor (0, 2);//poner el cursor en las coordenadas (x,y)
//          lcd.print(nombres[x]);//muestra en la pantalla max 20 caracteres
          registro = true;
          Serial.println("Leyendo");                   
          strcpy(myData.a, nombres[x]);
          strcpy(myData.b, "Salida");
          myData.c = horas;
          myData.d = minutos;
          // Send message via ESP-NOW
          esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
          
          open_door();
          rfid2.PICC_HaltA(); // halt PICC
          rfid2.PCD_StopCrypto1();
            //}
          break;
        }
        if(x==32){
          Serial.println("No Leyendo");
          strcpy(myData.a, "Usuario no encontrado ");
          strcpy(myData.b, "Salida");
          myData.c = horas;
          myData.d = minutos;
          esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

        
        //lcd.setCursor (0, 2);//poner el cursor en las coordenadas (x,y)
        //lcd.print(" No User ");//muestra en la pantalla max 20 caracteres
        //delay(100);
          digitalWrite(led_close, HIGH);
          delay(300);
          digitalWrite(led_close, LOW);
          delay(300);
          digitalWrite(led_close, HIGH);
          delay(300);
          digitalWrite(led_close, LOW);
          delay(300);
          digitalWrite(led_close, HIGH);
          delay(300);
          digitalWrite(led_close, LOW);
          rfid2.PICC_HaltA(); // halt PICC
          rfid2.PCD_StopCrypto1();
          break;
      
    }
      }
        
  }
  }

  Serial.println("Leyendo Pulsador");
  if (digitalRead(push_out) == HIGH) {
    open_door();
  } 
  Serial.println("Finalizando protocolos");
  rfid.PICC_HaltA(); // halt PICC
  rfid.PCD_StopCrypto1();
  rfid2.PICC_HaltA(); // halt PICC
  rfid2.PCD_StopCrypto1();
  
}


void open_door() {

  //lcd.setCursor (0, 3);//poner el cursor en las coordenadas (x,y)
  //lcd.print("   Puerta Abierta!  ");//muestra en la pantalla max 20 caracteres
  //delay(100);
  digitalWrite(signal_relay, HIGH);
  delay(20);
  digitalWrite(signal_relay, LOW);
  digitalWrite(led_open, HIGH);
  digitalWrite(led_close, LOW);
  delay(1500); //Tiempo desenergizado del iman
  digitalWrite(led_open, LOW);
  digitalWrite(led_close, HIGH);
  //lcd.setCursor (0, 3);//poner el cursor en las coordenadas (x,y)
  //lcd.print("   Puerta Cerrada!  ");//muestra en la pantalla max 20 caracteres
  
  delay(20);
  
}
