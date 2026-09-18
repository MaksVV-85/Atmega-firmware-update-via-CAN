#include <EEPROM.h>
#include <avr/boot.h>
#include <mcp_can.h>

#define CS_CAN_PIN 10              // выбираем на какой пин подключен к CS CAN модуля MCP2515, пока возможно только пин 10, потому что в бутлоадере другое значение не получилось сделать 
const byte CAN_Quarz = MCP_8MHZ;   // выбираем частоту кварца CAN модуля на этом узле 


#define PRINT_NAME(a) (const __FlashStringHelper*)(const char*) pgm_read_word(&(Node_Name[a]))
#define STRING_NAME(a) (const char*) pgm_read_word(&(Node_Name[a]))

#include "NodesDetails.h"

MCP_CAN CAN(CS_CAN_PIN);


#define JUMP_TO_BOOTLOADER cli();((void (*)(void))((FLASHEND - 0xFB7)/2))()  


byte myAddress;
byte nodemode = _NORMAL_MODE; 
uint32_t prev_nodeModeTimer = 0 ;  
uint32_t prev_reportSilentwindowMode = 0 ;  
byte  reportSilentwindowModeTimer  = 0 ; 
byte silent = 0 ;



uint32_t rxId; byte lenRx = 0; byte rxBuf[8]={0}; // компоненты принятого CAN сообщения 
byte txBuf[8]={0}; // компоненты отправляемого CAN сообщения 

void  setMaskFilters (bool on, uint32_t& fil)
{
  
 CAN.begin(MCP_STDEXT, CAN_Speed, CAN_Quarz);
CAN.setMode(MCP_NORMAL);
if (!on)
{
CAN.init_Mask(0,0,0);    // отключаем аппаратные CAN фильры  
CAN.init_Mask(1,0,0);    // отключаем аппаратные CAN фильры 
}
else 
{
 CAN.init_Mask(0, 0, 0x07FFFFFF);              // Init first mask...
 CAN.init_Filt(0, 0, fil);
 CAN.init_Filt(1, 0, fil);
 CAN.init_Mask(1, 0, 0x07FFFFFF);              // Init first mask...
 CAN.init_Filt(2, 0, fil);
 CAN.init_Filt(3, 0, fil);
 CAN.init_Filt(4, 0, fil);
 CAN.init_Filt(5, 0, fil); 
 }
  }


void CANboot_Setup()
{
#ifdef NODEADDR_FROM_ENUM
for (byte i=0; i<NODES_QUANTITY; i++){if (i!=NODES_QUANTITY-1) Nodeaddress[i]=i; else Nodeaddress[i] = 0xFF;}
#endif 
myAddress = EEPROM.read(MY_ADDR_EEPROM_CELL); // считываем из ЕЕПРОМ адрес этого узла
silent = EEPROM.read(MY_ADDR_EEPROM_CELL+2);  // считываем из ЕЕПРОМ состояние режима тихого окна во время прошивки
setMaskFilters(0,rxId); //отключаем аппаратные CAN фильры  
}


void CANboot()
{
// через миниту сбрасываем режим прошивки, если прошивка не прилетела  
// или если мы не целевой узел прошивки, то через минуту сбрасываем режим тишины, если он сам не отключился по окончании прошивки  
if (nodemode>0 && millis()-prev_nodeModeTimer>TIMEOUT_MODE)
{ 
  if (nodemode == _SILENT_WINDOW_MODEON) setMaskFilters(0,rxId); //отключаем аппаратные CAN фильры 
  nodemode = _NORMAL_MODE; 
} 

if (reportSilentwindowModeTimer && millis()-prev_reportSilentwindowMode>myAddress*5)   // если был запущен таймер на отправку отчета об изменении режима тихого окна и он кончился 
   {
      
      reportSilentwindowModeTimer = 0 ;  
      byte datatx[8]= {0};
      datatx[0] = myAddress;
      datatx[1] = silent;
      CAN.sendMsgBuf(CANID_SLAVE, 0, 6, datatx); // отправляем отчет прошивальщику о включении или выключении режима тихого окна при прошивке
   }  
}  


void CANboot_MSGAVAIL()
{
 
if (rxId == CANID_MASTER){   // если пришло сообщение от мастера прошивальщика 
       
       if ((rxBuf[0]==myAddress || rxBuf[0]==0xFF) && lenRx == 6)
       {
           if (rxBuf[1] == _SILENT_WINDOW_MODEON || rxBuf[1] == _SILENT_WINDOW_MODEOFF || rxBuf[1] == _CANTEST)  
           {
            silent = rxBuf[1];
            if (rxBuf[1] != _CANTEST) EEPROM.write(MY_ADDR_EEPROM_CELL+2, silent); 
            prev_reportSilentwindowMode = millis(); 
            reportSilentwindowModeTimer  = 1;
           }
        }
       
       if (rxBuf[0]==myAddress){  // если это сообщние адресовано на этот узел 
              if (lenRx == 8)           // если размер поля данных == 8 байт
               {
         if (rxBuf[1] == _REFLASH_MODE_REQUEST && rxBuf[2] == 0xAA && rxBuf[3] == 0xBB && rxBuf[4]== 0xCC ) {// если это запрос на включение режима ожидания прошивки 
         nodemode = _REFLASH_MODE; // включаем режим ожидания заливки прошивки 
         prev_nodeModeTimer = millis(); // включаем таймер , по истечение которого, если прошивка не начнётся, выйдем из режима прошивки
         byte dataTx[8]= {0};
         dataTx[0] = myAddress;
         dataTx[1] = _REFLASH_MODE; // отчитываемся прошивальщику, что перешли в режим ожидания прошивки 
         dataTx[2] = 0xDD;
         dataTx[3] = (byte)(F_CPU/1000000UL);  // отправляем прошивальщику частоту кварца МК на этом узле 
         byte sig[3]; for (byte i = 0 ; i<3;i++){sig[i] = boot_signature_byte_get(i*2); dataTx[i+4]=sig[i];} // отправляем прошивальщику сигнатуру МК этого узла
         dataTx[7] = CAN_Quarz;  // отправляем прошивальщику частоту кварца CAN на этом узле 
         CAN.sendMsgBuf(CANID_SLAVE, 0, 8, dataTx); // отправляем отчет прошивальщику
                                                                                                            }
         
               }// end lenRx == 8 
         else if (lenRx == 2)   // если размер поля данных == 2 байт
                  {if (nodemode ==_REFLASH_MODE){ if (rxBuf[1]==' ') {JUMP_TO_BOOTLOADER;}}}// если прилетела команда на начало процедуры прошивки , прыгаем в бутлоадер 
                              
                                 
                               } // end to myAddress
       
      else {}// если пришло сообщение от мастера прошивальщика, адресованное другому узлу сети 
                        } // end from CANID_MASTER
 else if (rxId == CANID_SLAVE)  // если пришло сообщение от слейва процедуры прошивки
              {
                if (lenRx == 2) { // если размер поля данных == 2 байт, значит началась прошивка другого узла 
                    if (silent ==_SILENT_WINDOW_MODEON && nodemode == _NORMAL_MODE) // если включен режим тихого окна при процедуре прошивки 
                    {
                    uint32_t filt = 0 ;  
                    byte id_addr = 0;         // переводим приемник на прием только одного сообщения _REFLASH_END от прошиваемого узла 
                    byte stat = _REFLASH_END; // переводим приемник на прием только одного сообщения _REFLASH_END от прошиваемого узла 
                    filt |= (CANID_SLAVE & 0xFFFFFFFF) << 16 | (id_addr & 0xFFFFFFFF) << 8 | (stat & 0xFFFFFFFF);
                    setMaskFilters(1,filt);   // переводим приемник на прием только одного сообщения _REFLASH_END от прошиваемого узла 
                    nodemode = _SILENT_WINDOW_MODEON; //  активируем режим тишины (в этом режиме сообщения не отправляются , а принимается только одно _REFLASH_END)
                    prev_nodeModeTimer = millis();    //  включаем таймер, по истечение которого режим тишины деактивируется  
                    
                    }    // end  if (silentWindowMode)
                             }// end  if (lenRx == 2)
             else if (lenRx == 8) // если размер поля данных == 8 байт
                       {  
                        if (nodemode == _SILENT_WINDOW_MODEON && rxBuf[1] == _REFLASH_END) // если получили сообщение _REFLASH_END от прошиваемого узла
                        {
                          nodemode =_NORMAL_MODE; //  деактивируем режим тишины 
                          setMaskFilters(0,rxId); //  деактивируем режим тишины 
                        }
                        
                       }         
              }
  
}
