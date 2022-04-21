/* шрифт создан с помощью сервиса ...
*  файл шрифта необходимо полдожить в папку C:\Users\Gamer\Documents\Arduino\libraries\TTGO_TWatch_Library-master\src\TFT_eSPI\Fonts\Custom
*  и прописать в файле "C:\Users\Gamer\Documents\Arduino\libraries\TTGO_TWatch_Library-master\src\TFT_eSPI\User_Setups\User_Custom_Fonts.h"
*  подобрать положение текста и единиц измерения
* 
* 
*/

#include <Wire.h>
#include <TTGO.h>
#include <Arduino.h>
#include "radSens1v2.h"
#include "Free_Fonts.h"  

#define CF_OM72 &Orbitron_Medium_72
#define CF_OL32 &Orbitron_Light_32

ClimateGuard_RadSens1v2 radSens(RS_DEFAULT_I2C_ADDRESS); /*Constructor of the class ClimateGuard_RadSens1v2,
                                                           sets the address parameter of I2C sensor.
                                                           Default address: 0x66.*/

TTGOClass *ttgo;

static float RadIntensyDyanmic[240];
static float RadIntensyStatic[240];
int i = 0;
uint16_t tft_width  = 240;                                // ST7789_TFTWIDTH;
uint16_t tft_height = 240;                                // ST7789_TFTHEIGHT;
bool warning = false;
static float maxDynamic = 20;

#define NormalLevel 20
#define GraphHeight 100

#define TFT_GREY 0x5AEB

void pressed(){
  
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  radSens.radSens_init();                                 /*Initialization function and sensor connection. Returns false if the sensor is not connected to the I2C bus.*/  
  uint8_t sensorChipId = radSens.getChipId();             /*Returns chip id, default value: 0x7D.*/
  Serial.print("Chip id: 0x");
  Serial.println(sensorChipId, HEX);
  uint8_t firmWareVer = radSens.getFirmwareVersion();     /*Returns firmware version.*/
  Serial.print("Firmware version: ");
  Serial.println(firmWareVer);
  Serial.println("-------------------------------------");
  uint16_t sensitivity = radSens.getSensitivity();        /*Rerutns the value coefficient used for calculating
                                                          the radiation intensity or 0 if sensor isn't connected.*/

  Serial.print("\t getSensitivity(): ");
  Serial.println(sensitivity);
  bool hvGeneratorState = radSens.getHVGeneratorState();  /*Returns state of high-voltage voltage Converter.
                                                           If return true -> on
                                                           If return false -> off or sensor isn't conneted*/

  Serial.print("\n\t HV generator state: ");
  Serial.println(hvGeneratorState);

  ttgo->button->setPressedHandler(pressed);                           
  ttgo->openBL();
  ttgo->lvgl_begin();
  ttgo->eTFT->setRotation(0);
  drawLogo();
  ttgo->eTFT->fillScreen(TFT_BLACK);
  delay(5000);
}

void loop() {
  ttgo->button->loop();
  if (i < (tft_width-1)){                                   //накапливаем значения в массив
    RadIntensyDyanmic[i] = radSens.getRadIntensyDyanmic();
    warning = int(RadIntensyDyanmic[i]) / NormalLevel;      //проверяем на превышение нормы
    if (RadIntensyDyanmic[i] <= 0){                         //проверяем на равенство 0
      RadIntensyDyanmic[i] = RadIntensyDyanmic[i-1];        //что если i равно 0?
    }
    if (RadIntensyDyanmic[i] > maxDynamic){                 //находим максимум
      maxDynamic = RadIntensyDyanmic[i];
    }
    RadIntensyStatic[i] = radSens.getRadIntensyStatic();
    if (RadIntensyStatic[i] <= 0){                          //проверяем на равенство 0
      RadIntensyStatic[i] = RadIntensyStatic[i-1];          //что если i равно 0?
    }
    i += 1;    
  }
  else {
    for (int k = 0; k < (tft_width-1); k++){                //сдвигаем массив влево на 1 значение
      RadIntensyDyanmic[k] = RadIntensyDyanmic[k+1];
      RadIntensyStatic[k] = RadIntensyStatic[k+1];
      }
    RadIntensyDyanmic[i] = radSens.getRadIntensyDyanmic();  //заполняем последнее значение массива
    warning = int(RadIntensyDyanmic[i]) / NormalLevel;      //проверяем на превышение нормы
    if (RadIntensyDyanmic[i] == 0){                         //проверяем на равенство 0
      RadIntensyDyanmic[i] = RadIntensyDyanmic[i-1];
      }
    if (RadIntensyDyanmic[i] > maxDynamic){                 //находим максимум
      maxDynamic = RadIntensyDyanmic[i];
    }  
    RadIntensyStatic[i] = radSens.getRadIntensyStatic();    //заполняем последнее значение массива
    if (RadIntensyStatic[i] == 0){ 
      RadIntensyStatic[i] = RadIntensyStatic[i-1];          //проверяем на равенство 0
    }          
  }
  //ttgo->eTFT->setFreeFont(FMB24);
  ttgo->eTFT->setFreeFont(CF_OM72);
  ttgo->eTFT->setTextColor(ttgo->eTFT->color565(127, 127, 255), TFT_BLACK);
  ttgo->eTFT->setCursor(75, 110);  
 
  Serial.println(i);
  if (i<tft_width){
    ttgo->eTFT->fillRect(70, 55, 170, 60, TFT_BLACK);
    ttgo->eTFT->print(RadIntensyDyanmic[i-1], 1);
    drawBar(RadIntensyDyanmic[i-1]);
    Serial.println(RadIntensyDyanmic[i-1]);    
    }
  else {
    ttgo->eTFT->fillRect(70, 55, 170, 60, TFT_BLACK);
    ttgo->eTFT->print(RadIntensyDyanmic[i], 1);
    drawBar(RadIntensyDyanmic[i]);
    Serial.println(RadIntensyDyanmic[i]);    
  }
  
  ttgo->eTFT->setFreeFont(CF_OL32);
  ttgo->eTFT->setTextColor(ttgo->eTFT->color565(127, 127, 255), TFT_BLACK);
  ttgo->eTFT->setCursor(100, 140);
  ttgo->eTFT->print("uR/h"); 
  
  draw2Graph();                                             //функция вывода двух графиков
  drawWarning(warning);                                     //функция вывода шкалы
  delay(2000);  
}

void drawGraph(){
  for (int c = 0; c < tft_width-1; c++){
    ttgo->eTFT->drawFastVLine(c, tft_height-(RadIntensyDyanmic[c]*5), tft_height, TFT_YELLOW); 
    ttgo->eTFT->drawFastVLine(c, 120, 120-(RadIntensyDyanmic[c]*5), TFT_BLACK);   
    }
}

void draw2Graph(){                                           //функция вывода двух графиков
  for (int c = 0; c < tft_width-1; c++){
    int DynHeight = RadIntensyDyanmic[c] * GraphHeight / maxDynamic;
    int StatHeight = RadIntensyStatic[c] * GraphHeight / maxDynamic;
    if (DynHeight >= StatHeight){
      ttgo->eTFT->drawFastVLine(c, tft_height-DynHeight, tft_height-StatHeight, TFT_YELLOW); 
      ttgo->eTFT->drawFastVLine(c, tft_height-StatHeight, tft_height, ttgo->eTFT->color565(128, 128, 128)); 
      ttgo->eTFT->drawFastVLine(c, tft_height-GraphHeight, GraphHeight-DynHeight, TFT_BLACK);      
      }
    else {
      ttgo->eTFT->drawFastVLine(c, tft_height-StatHeight, tft_height-DynHeight, ttgo->eTFT->color565(0, 0, 128)); 
      ttgo->eTFT->drawFastVLine(c, tft_height-DynHeight, tft_height, ttgo->eTFT->color565(128, 128, 128)); 
      ttgo->eTFT->drawFastVLine(c, tft_height-GraphHeight, GraphHeight-StatHeight, TFT_BLACK);  
      } 
    }
}

void drawBar(float val){                                      //функция вывода шкалы
  int BarHeight = map(val, 0, NormalLevel, 0, 220);
  if (BarHeight > 220) {
    BarHeight = 220;
    }
  ttgo->eTFT->fillRect(10, 10, BarHeight, 10, TFT_RED);
  ttgo->eTFT->fillRect(10+BarHeight, 10, 220-BarHeight, 10, TFT_GREY);
}

void drawLogo(){                                              //анимация
  ttgo->eTFT->fillScreen(TFT_BLACK);
  ttgo->eTFT->fillTriangle(120, 20, 235, 220, 5, 220, TFT_YELLOW);
  ttgo->eTFT->fillCircle(120, 154, 50, TFT_BLACK);
  ttgo->eTFT->fillTriangle(120, 154, 149, 104, 91, 104, TFT_YELLOW);
  ttgo->eTFT->fillTriangle(120, 154, 178, 154, 149, 204, TFT_YELLOW);
  ttgo->eTFT->fillTriangle(120, 154, 62, 154, 91, 204, TFT_YELLOW);
  ttgo->eTFT->fillCircle(120, 154, 10, TFT_YELLOW);
  ttgo->eTFT->fillCircle(120, 154, 5, TFT_BLACK);
  delay(2000);
}

void drawWarning(boolean warning){                            //вывод предупреждения
  if (warning) {
    ttgo->eTFT->fillTriangle(30, 80, 55, 120, 5, 120, TFT_RED);
    ttgo->eTFT->setTextColor(TFT_WHITE, TFT_BLACK);
    ttgo->eTFT->setCursor(27, 118);
    ttgo->eTFT->print("!"); 
  }
  else {
    ttgo->eTFT->fillRect(5, 80, 50, 45, TFT_BLACK);
  }
}


