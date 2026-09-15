#include <EEPROM.h>
#include <avr/boot.h>
#include <mcp_can.h>

uint32_t UART_speed = 19200;       // скорость UART по умолчанию. (эта скорость должна совпадать с той, что указана для целевой платы в файле boards.txt, и с которой работаем в терминале)
byte CAN_Quarz = MCP_8MHZ;         // частота кварца CAN на адаптере-прошивальщике по умолчанию
#define CS_CAN_PIN 10              // выбираем на какой пин подключен CS CAN модуля MCP2515, пока возможноо только 10, потому что в бутлоадере другое значение не получилось сделать 



#include "NodesDetails.h"

bool CANtest[NODES_QUANTITY-1]={0};


#define PRINT_NAME(a) (const __FlashStringHelper*)(const char*) pgm_read_word(&(Node_Name[a]))
#define PRINT_COMMAND(a) (const __FlashStringHelper*)(const char*) pgm_read_word(&(PGMusbcan_commands[a]))
#define STRING_NAME(a) (const char*) pgm_read_word(&(Node_Name[a]))
#define STRING_COMMAND(a) (const char*) pgm_read_word(&(PGMusbcan_commands[a]))

enum USBCAN_COMMANDS_ 
{
  PRINTLIST00,            // распечатка в терминал списка адресов CAN узлов и списка команд 
  PRINTLIST,              // распечатка в терминал списка адресов CAN узлов и списка команд 
  ON_REFLASHMODE11,       // включение режима прошивки на USB<->CAN адатере без подтверждения от узла 
  ON_REFLASHMODE,         // включение режима прошивки на USB<->CAN адатере без подтверждения от узла 
  CAN_TEST22,             // тест контроля связи с узлами  
  CAN_TEST,               // тест контроля связи с узлами  
  ON_SILENT_WINDOW_MODE,  // включение режима тихого окна на шине на момент прошивки 
  OFF_SILENT_WINDOW_MODE, // выключение режима тихого окна на шине на момент прошивки
  CANQUARZ_MCP_8MHZ,      // выбор частоты кварца на CAN модуле 
  CANQUARZ_MCP_16MHZ,     // выбор частоты кварца на CAN модуле 
  CANSPEED_125KBPS,       // выбор скорости CAN шины
  CANSPEED_200KBPS,       // выбор скорости CAN шины
  CANSPEED_250KBPS,       // выбор скорости CAN шины
  CANSPEED_500KBPS,       // выбор скорости CAN шины
  UARTSPEED_4800BAUD,     // выбор скорости UART
  UARTSPEED_9600BAUD,     // выбор скорости UART
  UARTSPEED_19200BAUD,    // выбор скорости UART
  UARTSPEED_38400BAUD,    // выбор скорости UART
  UARTSPEED_57600BAUD,    // выбор скорости UART
  UARTSPEED_115200BAUD,   // выбор скорости UART
  COMMANDS_QUANTITY
};
  
const char printlist00            [] PROGMEM = "00";
const char printlist              [] PROGMEM = "PRINT_LIST";
const char On_reflashMode11       [] PROGMEM = "11";
const char On_reflashMode         [] PROGMEM = "ON_REFLASHMODE";

const char Can_test11             [] PROGMEM = "22";
const char Can_test               [] PROGMEM = "CAN_TEST";

const char _ON_SILENT_WINDOW_MODE [] PROGMEM = "ON_SILENT_WINDOW_MODE";
const char _OFF_SILENT_WINDOW_MODE[] PROGMEM = "OFF_SILENT_WINDOW_MODE";
const char _CANQUARZ_MCP_8MHZ     [] PROGMEM = "CANQUARZ_MCP_8MHZ";
const char _CANQUARZ_MCP_16MHZ    [] PROGMEM = "CANQUARZ_MCP_16MHZ";
const char _CANSPEED_125KBPS      [] PROGMEM = "CANSPEED_125KBPS";
const char _CANSPEED_200KBPS      [] PROGMEM = "CANSPEED_200KBPS";
const char _CANSPEED_250KBPS      [] PROGMEM = "CANSPEED_250KBPS";
const char _CANSPEED_500KBPS      [] PROGMEM = "CANSPEED_500KBPS";
const char _UARTSPEED_4800BAUD   [] PROGMEM = "UARTSPEED_4800BAUD";
const char _UARTSPEED_9600BAUD   [] PROGMEM = "UARTSPEED_9600BAUD";
const char _UARTSPEED_19200BAUD  [] PROGMEM = "UARTSPEED_19200BAUD";
const char _UARTSPEED_38400BAUD  [] PROGMEM = "UARTSPEED_38400BAUD";
const char _UARTSPEED_57600BAUD  [] PROGMEM = "UARTSPEED_56700BAUD";
const char _UARTSPEED_115200BAUD [] PROGMEM = "UARTSPEED_115200BAUD";

const char* const PGMusbcan_commands[] PROGMEM = 
{
printlist00, 
printlist, 
On_reflashMode11,
On_reflashMode,
Can_test11,
Can_test,
_ON_SILENT_WINDOW_MODE,
_OFF_SILENT_WINDOW_MODE,
_CANQUARZ_MCP_8MHZ, 
_CANQUARZ_MCP_16MHZ, 
_CANSPEED_125KBPS,  
_CANSPEED_200KBPS,  
_CANSPEED_250KBPS,  
_CANSPEED_500KBPS,
_UARTSPEED_4800BAUD,   
_UARTSPEED_9600BAUD,   
_UARTSPEED_19200BAUD,  
_UARTSPEED_38400BAUD,  
_UARTSPEED_57600BAUD,  
_UARTSPEED_115200BAUD 
};

MCP_CAN CAN(CS_CAN_PIN);

#define I_AM   0
#define TARGET 1

byte adapterMode; 
byte NodeTargetEnum; 
uint32_t prev_updateMode_timer; 
bool updateMode_timer; 
