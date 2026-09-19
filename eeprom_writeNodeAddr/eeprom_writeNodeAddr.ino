#include <EEPROM.h>
#include "NodesDetails.h"

//------------------Тут выбираем имя узла (адрес), который хотим зашить в еепром этого узла--------
//                  \/
byte NodeAddr = VYKL_VANNA;  
// имя узла нужно выбрать из списка enum_Nodeaddress в файле-вкладке NodesDetails.h

// ниже ничего не трогаем, выбираем нужный тип платы (процессор, скорость UART, вариант МК) из платы Arduino CAN_BUS
// и во вклаке "Скетч" нажимаем "Экспорт бинарного файла" 
// после чего выбираем ком порт arduinoISP , выбираем плату "8MHz_atmega328 writeAddress" или "16MHz_atmega328 writeAddress"
// и во вклаке "Инструменты" нажимаем "Прошить загрузчик"

// естественно при этом к arduinoISP по SPI должен быть подключен наш пациент - целевой узел. 


void setup() {
 
#ifdef NODEADDR_FROM_ENUM
for (byte i=0; i<NODES_QUANTITY; i++){if (i!=NODES_QUANTITY-1) Nodeaddress[i]=i; else Nodeaddress[i] = 0xFF;}
#endif 
  
  
  EEPROM.write(MY_ADDR_EEPROM_CELL, Nodeaddress[NodeAddr]);
}

void loop() {}
