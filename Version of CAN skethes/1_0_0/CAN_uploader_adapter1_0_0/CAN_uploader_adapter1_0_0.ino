//v1.0.0


#include "CAN_BUS.h"


void maskFilter(uint16_t id_m, byte id_addr, bool ONfilter) {

  CAN.begin(MCP_STDEXT, CAN_Speed, CAN_Quarz);
  CAN.setMode(MCP_NORMAL);

  if (!ONfilter) {
    CAN.init_Mask(0, 0, 0);              // Init first mask...
    CAN.init_Mask(1, 0, 0);              // Init first mask...
  }
  else {
    uint32_t filt = 0 ;
    filt |= (id_m & 0xFFFFFFFF) << 16 | (id_addr & 0xFFFFFFFF) << 8;

    CAN.init_Mask(0, 0, 0x07FFFF00);              // Init first mask...
    CAN.init_Filt(0, 0, filt);
    CAN.init_Filt(1, 0, filt);
    CAN.init_Mask(1, 0, 0x07FFFF00);              // Init first mask...
    CAN.init_Filt(2, 0, filt);
    CAN.init_Filt(3, 0, filt);
    CAN.init_Filt(4, 0, filt);
    CAN.init_Filt(5, 0, filt);

  }
}

void  printNodeTarget (bool pref) {
       if (pref == I_AM)   Serial.print (F("      I am "));
  else if (pref == TARGET) Serial.print (F("Target "));
  Serial.print (F("Node \""));
  Serial.print(PRINT_NAME(NodeTargetEnum)); 
  Serial.print(F("\" address: \"0x"));
  if (Nodeaddress[NodeTargetEnum] < 0x10)Serial.print ("0");
  Serial.print (Nodeaddress[NodeTargetEnum], HEX);
  Serial.print(F("\""));
}


void USBCANsetMode(byte mode)
{
  if (mode != _GET_STATUS_MODE) {
    adapterMode = mode;
    if (adapterMode == _SETTING_MODE)   {
      maskFilter(0, 0, 0);
    }
    else if (adapterMode == _REFLASH_MODE) {
      maskFilter(CANID_SLAVE, Nodeaddress[NodeTargetEnum], 1);
    }
  }
  static bool start = 1 ;
  if (adapterMode == _SETTING_MODE)   {
    if (start) start = 0 ;
    else {
      Serial.print(F(" Adapter UART<->CAN is \"Setting mode\"!" ));
      if (!updateMode_timer) Serial.println();
    }
  }
  else if (adapterMode == _REFLASH_MODE) {
    Serial.print(F(" Adapter UART<->CAN is \"Reflashing mode\"!  "));
    printNodeTarget(TARGET);
    Serial.println();
  }
}

void printCANspeed ()
{
   Serial.print(F("CAN Speed "));
        if (CAN_Speed == CAN_125KBPS)  Serial.print(F("125"));
   else if (CAN_Speed == CAN_200KBPS)  Serial.print(F("200"));
   else if (CAN_Speed == CAN_250KBPS)  Serial.print(F("250"));
   else if (CAN_Speed == CAN_500KBPS)  Serial.print(F("500"));
   Serial.println(F(" Kbps has been selected!"));
}

void printUARTspeed ()
{
  Serial.print(F("UART Speed ")); Serial.print (UART_speed);  
  Serial.println  (F(" Baud has been selected!")); 
}


void printQuarz()
{
  Serial.println();
  if (CAN_Quarz == MCP_8MHZ)  Serial.println(F("CAN Quarz 8 MHz has been selected!"));
  else if (CAN_Quarz == MCP_16MHZ) Serial.println(F("CAN Quarz 16 MHz has been selected!"));
}

void printNodeslist (bool teston)
{
  Serial.println (F("Nodes: "));
  for (int  i = 0 ; i < NODES_QUANTITY; i++ )
  {
    Serial.print (F("0x"));
    if (Nodeaddress[i] <= 0x0F)Serial.print (F("0"));
    Serial.print (Nodeaddress[i], HEX); Serial.print (F("  "));
    Serial.print(PRINT_NAME(i));
    if (teston && i!=INCOGNITO && i!=BROADCAST) {
      if (CANtest[i]) Serial.print (F(" OK!"));
      else Serial.print (F(" FAIL!"));
      }
    Serial.println();
  }
  Serial.println();
}


void printCommandslist ()
{
  Serial.println (F("Commands: "));
  for (int  i = 0 ; i < COMMANDS_QUANTITY; i++ ) {
 /*   if (i == PRINTLIST00)     {
      Serial.print(PRINT_COMMAND(i));
      Serial.print (F(" or "));
      Serial.println(PRINT_COMMAND(i + 1));
      i++;
    }
    else if (i == ON_REFLASHMODE11) {
      Serial.print(PRINT_COMMAND(i));
      Serial.print (F(" or "));
      Serial.println(PRINT_COMMAND(i + 1));
      i++;
    }
    else if (i == CAN_TEST22) {
      Serial.print(PRINT_COMMAND(i));
      Serial.print (F(" or "));
      Serial.println(PRINT_COMMAND(i + 1));
      i++;
    }
  */

if (i == PRINTLIST00 || i == ON_REFLASHMODE11 || i == CAN_TEST22)
    {
      Serial.print(PRINT_COMMAND(i));
      Serial.print (F(" or "));
      Serial.println(PRINT_COMMAND(i + 1));
      i++;
     }
    else {
      Serial.println(PRINT_COMMAND(i));
    }
  }
  Serial.println();
}

void printList()
{
  Serial.println(F("Please enter NodeName or Command from list")); Serial.println();
  printNodeslist(0); printCommandslist();
  USBCANsetMode(_GET_STATUS_MODE);
}

void Ekran ()
{
  printQuarz();
  printCANspeed();
  printUARTspeed ();
  printList();
}

void setup()
{
  #ifdef NODEADDR_FROM_ENUM
  for (byte i=0; i<NODES_QUANTITY; i++){if (i!=NODES_QUANTITY-1) Nodeaddress[i]=i; else Nodeaddress[i] = 0xFF;}
  #endif 
  Serial.begin(UART_speed);
  USBCANsetMode (_SETTING_MODE);
  Ekran ();
}

void loop()
{
  static char currStr[60];
  static uint32_t prevsymb = 0 ;
  static bool timer_symb = 0 ;
  static bool stringEnd = 0;
  static bool test = 0;
  static uint32_t prevtest = 0;

  
  while (1) {

    if (Serial.available())
    {
      byte dataTx[8] = {0};

      if (adapterMode == _SETTING_MODE )
      {

        char currSymb[2] = {0};
        currSymb[0] = Serial.read();

        if (!timer_symb) {
          timer_symb = 1;
        }
        prevsymb = millis();

        if (currSymb[0] == '\r' || stringEnd == 1)
        {
          bool found = 0 ;
          bool uartchange = 0 ;
          for (byte i = 0; i < NODES_QUANTITY; i++)
          {

            if (strcmp_P(currStr, STRING_NAME(i)) == 0) {
              found = 1;
              NodeTargetEnum = i;
              printNodeTarget(TARGET);
              Serial.println (F(" has been successfully selected!"));
              dataTx[0] = Nodeaddress[NodeTargetEnum];
              dataTx[1] = _REFLASH_MODE_REQUEST;
              dataTx[2] = 0xAA;
              dataTx[3] = 0xBB;
              dataTx[4] = 0xCC;
              CAN.sendMsgBuf(CANID_MASTER, 0, 8, dataTx);
            }
          }
          if  (strcmp_P(currStr, STRING_COMMAND(PRINTLIST)) == 0 || strcmp_P(currStr, STRING_COMMAND(PRINTLIST00)) == 0) {
            found = 1;
           Ekran ();
          }
          else if  (strcmp_P(currStr, STRING_COMMAND(ON_REFLASHMODE)) == 0 || strcmp_P(currStr, STRING_COMMAND(ON_REFLASHMODE11)) == 0)
          {
            found = 1;
            USBCANsetMode(_REFLASH_MODE);
          }

           else if  (strcmp_P(currStr, STRING_COMMAND(CAN_TEST)) == 0 || strcmp_P(currStr, STRING_COMMAND(CAN_TEST22)) == 0)
          {
            found = 1;
            dataTx[1] = _CANTEST;
            dataTx[0] = 0xFF;
            CAN.sendMsgBuf(CANID_MASTER, 0, 6, dataTx); 
            test = 1; prevtest = millis(); for (byte j=0; j<NODES_QUANTITY-1;j++)CANtest[j]=0; 
           }
          
          else if  (strcmp_P(currStr, STRING_COMMAND(CANQUARZ_MCP_8MHZ)) == 0)  {
            found = 1;
            CAN_Quarz = MCP_8MHZ;
            USBCANsetMode(_SETTING_MODE);
            printQuarz();
          }
          else if  (strcmp_P(currStr, STRING_COMMAND(CANQUARZ_MCP_16MHZ)) == 0) {
            found = 1;
            CAN_Quarz = MCP_16MHZ;
            USBCANsetMode(_SETTING_MODE);
            printQuarz();
          }
          else if  (strcmp_P(currStr, STRING_COMMAND(CANSPEED_125KBPS)) == 0) {
            found = 1;
            CAN_Speed = CAN_125KBPS;
            USBCANsetMode(_SETTING_MODE);
            printCANspeed ();
          }
          else if  (strcmp_P(currStr, STRING_COMMAND(CANSPEED_200KBPS)) == 0) {
            found = 1;
            CAN_Speed = CAN_200KBPS;
            USBCANsetMode(_SETTING_MODE);
            printCANspeed ();
          }
          else if  (strcmp_P(currStr, STRING_COMMAND(CANSPEED_250KBPS)) == 0) {
            found = 1;
            CAN_Speed = CAN_250KBPS;
            USBCANsetMode(_SETTING_MODE);
            printCANspeed ();
          }
          else if  (strcmp_P(currStr, STRING_COMMAND(CANSPEED_500KBPS)) == 0) {
            found = 1;
            CAN_Speed = CAN_500KBPS;
            USBCANsetMode(_SETTING_MODE);
            printCANspeed ();
          }
          else if  (strcmp_P(currStr, STRING_COMMAND(UARTSPEED_4800BAUD)) == 0) {
            found = 1; uartchange = 1;
            UART_speed = 4800;
            printUARTspeed();
          }
          else if  (strcmp_P(currStr, STRING_COMMAND(UARTSPEED_9600BAUD)) == 0) {
            found = 1; uartchange = 1;
            UART_speed = 9600;
            printUARTspeed();
          }
          else if  (strcmp_P(currStr, STRING_COMMAND(UARTSPEED_19200BAUD)) == 0) {
            found = 1; uartchange = 1;
            UART_speed = 19200;
            printUARTspeed();
          }
          else if  (strcmp_P(currStr, STRING_COMMAND(UARTSPEED_38400BAUD)) == 0) {
            found = 1; uartchange = 1;
            UART_speed = 38400;
            printUARTspeed();
          }
          else if  (strcmp_P(currStr, STRING_COMMAND(UARTSPEED_57600BAUD)) == 0) {
            found = 1; uartchange = 1;
            UART_speed = 57600;
            printUARTspeed();
          }
          else if  (strcmp_P(currStr, STRING_COMMAND(UARTSPEED_115200BAUD)) == 0) {
            found = 1; uartchange = 1;
            UART_speed = 115200;
            printUARTspeed();
          }
          else if  (strcmp_P(currStr, STRING_COMMAND(OFF_SILENT_WINDOW_MODE)) == 0) {
            found = 1;
            Serial.println (F(" Send request for silentWindowMode OFF "));
            dataTx[1] = _SILENT_WINDOW_MODEOFF;
            dataTx[0] = 0xFF;
            CAN.sendMsgBuf(CANID_MASTER, 0, 6, dataTx); 
          }

          else if  (strcmp_P(currStr, STRING_COMMAND(ON_SILENT_WINDOW_MODE)) == 0) {
            found = 1;
            Serial.println (F(" Send request for silentWindowMode ON "));
            dataTx[1] = _SILENT_WINDOW_MODEON;
            dataTx[0] = 0xFF;
            CAN.sendMsgBuf(CANID_MASTER, 0, 6, dataTx); 
          }

          if       (!found ) {
            Serial.println(F("Node name or command is incorrect! Please check and enter it again!"));
          }

         if (uartchange)
         {
          delay (500);
          Serial.println (F(" Please change UART speed at the Terminal!!!")); delay (500); Serial.end();
          Serial.begin(UART_speed);
         }


          Serial.read();
          currStr[0] = 0; stringEnd = 0;

        }
        else if ( currSymb[0] != '\n')  strcat (currStr, currSymb);
        if (strlen(currStr) > 58) stringEnd = 1;

      }
      else if (adapterMode == _REFLASH_MODE)  {
        byte inbyte = Serial.read();
        if (!updateMode_timer) {
          updateMode_timer = 1;
          prev_updateMode_timer = millis();
        }
        else prev_updateMode_timer = millis();
        dataTx[0] = Nodeaddress[NodeTargetEnum];
        dataTx[1] = inbyte;
        // while (CAN.mcp2515_getNextFreeTXBuf__());
        CAN.sendMsgBuf(CANID_MASTER, 0, 2, dataTx);
      }

    }// end of Serial.available()



    if (CAN_MSGAVAIL == CAN.checkReceive())
    {
      uint32_t rxId; byte lenRx = 0; byte rxBuf[8] = {0};
      CAN.readMsgBuf(&rxId, &lenRx, rxBuf);
      
      if (lenRx == 8)
      {
        if  (adapterMode == _SETTING_MODE && rxId == CANID_SLAVE && rxBuf[0] == Nodeaddress[NodeTargetEnum] && rxBuf[1] == _REFLASH_MODE && rxBuf[2] == 0xDD)
        {
          printNodeTarget (I_AM);
          Serial.println(F(" and I am ready for firmware update!"));
          Serial.print(F("F_CPU = "));
          Serial.print(rxBuf[3]); Serial.println(F("MHz"));
          Serial.print(F("CPU model = atmega"));
               if (rxBuf[4] == 0x1E && rxBuf[5] == 0x98 && rxBuf[6] == 0x01)  Serial.println (F("2560"));
          else if (rxBuf[4] == 0x1E && rxBuf[5] == 0x95 && rxBuf[6] == 0x14)  Serial.println (F("328"));
          else if (rxBuf[4] == 0x1E && rxBuf[5] == 0x95 && rxBuf[6] == 0x0F)  Serial.println (F("328P"));
          else if (rxBuf[4] == 0x1E && rxBuf[5] == 0x95 && rxBuf[6] == 0x16)  Serial.println (F("328PB"));
          else if (rxBuf[4] == 0x1E && rxBuf[5] == 0x94 && rxBuf[6] == 0x06)  Serial.println (F("168"));
          else if (rxBuf[4] == 0x1E && rxBuf[5] == 0x94 && rxBuf[6] == 0x0B)  Serial.println (F("168P"));
          else if (rxBuf[4] == 0x1E && rxBuf[5] == 0x93 && rxBuf[6] == 0x0A)  Serial.println (F("88"));
          else if (rxBuf[4] == 0x1E && rxBuf[5] == 0x93 && rxBuf[6] == 0x0F)  Serial.println (F("88P"));
          else  Serial.println (F("Unknown CPU"));
          Serial.print (F("CAN Quarz: "));
               if (rxBuf[7] == MCP_8MHZ)   Serial.println (F("8 MHz"));
          else if (rxBuf[7] == MCP_16MHZ)  Serial.println (F("16 MHz"));
          else if (rxBuf[7] == MCP_20MHZ)  Serial.println (F("20 MHz"));
          else Serial.println (F("Unknown frequency!"));
          Serial.println();
          USBCANsetMode(_REFLASH_MODE);
        }
      
        
      }// end if (lenRx == 8)
      else if (lenRx == 2) {if (adapterMode == _REFLASH_MODE){ Serial.write (rxBuf[1]);}}
      else if (lenRx == 6) {
        
        if (rxBuf[1] == _SILENT_WINDOW_MODEON ||  rxBuf[1] == _SILENT_WINDOW_MODEOFF) {
          NodeTargetEnum = rxBuf[0];
          printNodeTarget (I_AM);
          Serial.print(F(" and silentWindowMode is "));
          if (rxBuf[1] == _SILENT_WINDOW_MODEON) Serial.println(F("ON")); else if (rxBuf[1] == _SILENT_WINDOW_MODEOFF) Serial.println(F("OFF"));
          }
        else if (rxBuf[1] == _CANTEST)
        {if (rxBuf[0] < (NODES_QUANTITY-1)) CANtest[rxBuf[0]]=1; }
        
        }
    }

    if (updateMode_timer && millis() - prev_updateMode_timer > 3000) { // после того, как байты прошивки перестали летать в шине -  режим адаптера переводим в "настройка"
      USBCANsetMode(_SETTING_MODE);
      updateMode_timer = 0;
      Serial.println(F("  (by timer after reflashing or error) "));
    }

    if (timer_symb && millis() - prevsymb > 500) {
      timer_symb = 0 ;
      currStr[0] = 0 ;
      stringEnd = 0;
    }

    if (test && millis() - prevtest>=1000)
      {
      test = 0; 
      
      printNodeslist(1);
      
      }


  } // end while(1)

}// end loop()
