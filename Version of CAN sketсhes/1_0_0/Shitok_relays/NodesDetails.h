// после того, как , заполнили этот файл NodeDetails.h копируем его (заменяем) в папках  прошивок для всех узлов CAN сети, в т.ч. для прошивки адаптера-прошивальщика!

byte CAN_Speed = CAN_200KBPS;      // выбираем скорость CAN (должна быть везде одинаковая и в бутлоадере и в адаптере-прошивальщике и на всех узлах сети)
const uint16_t CANID_MASTER=0x000; // ID CAN сообщения мастера (прошивальщика)    ставить такое же как в бутлоадере (лучше оставить именно такое)
const uint16_t CANID_SLAVE =0x001; // ID CAN сообщения слейва (прошиваемого узла) ставить такое же как в бутлоадере (лучше оставить именно такое)
#define TIMEOUT_MODE 60000UL       // выбираем таймаут возвращения в нормальный режим (мс) из режима прошивки или режима тишины, если что-то пошло не так
#define MY_ADDR_EEPROM_CELL 0x05   // выбираем номер ячеейки в еепроме, в которой хранится адрес этого узла (значение должно быть то же, что и в бутлоадере)
                                   // после этой ячейки еще три после нее в ЕЕПРОМЕ ничем не занимаем! 


// внимательно пишем реквизиты всех узлов сети CAN (размер адреса узла: 1 байт)
// тут четыре списка, мои убираем (кроме INCOGNITO, NODES_QUANTITY и BROADCAST, их оставляем на тех же местах), делаем свои четыре списка по аналогии, как у меня, 
// соблюдая одинаковое расположение (последовательность) узлов во всех четырех списках. 
// рекомендую название узлов сети делать такое же, как и название файла прошивки для них (во втором списке особенно).
// это сильно облегчит выбор файла прошивки и соответствующего узла при процедуре удаленной прошивки
// и уменьшит вероятность несоответствия файла прошивки целевому узлу

// первый список это нумерация узлов (порядковые номера), по количеству их должно быть не больше 250 узлов , этот список лучше писать заглавными буквами, 
// элементы этого списка используются в теле скетча при выборе целевого узла в аргументах функций, по этой нумерации как раз и происходит идентификация с каким узлом имеем дело.
// например, при отправке какому-либо узлу CAN сообщения: 
// CANsend(VYKL_KUHNYA, CAN_EXTID, 8); // где CANsend ваша процедура отправки сообщения в CAN , в левом аргументе которой выбран целевой узел

enum enum_Nodeaddress_ {
  INCOGNITO,        // 00  адрес (от неизвестного узла)
  
  SHITOK_RELAYS,    // 01  
  VYKL_ZAL,         // 02  
  VYKL_PRIHOZH,     // 03  
  PODMOYKOY,        // 04  
  VYKL_DETSK ,      // 05  
  VYKL_KUHNYA,      // 06  
  VYKL_VANNA,       // 07  
  VYKL_KLADOVKA,    // 08  
  VYKL_KORIDOR,     // 09  
  VYKL_SPALNYA,     // 0A  
  VYKL_KABINET,     // 0B  
  
  BROADCAST,         // адрес (всем сразу)
  NODES_QUANTITY,    // количество узлов в сети
  };

// второй список это строки названия узлов , помещенные во флеш для экономии оперативой памяти 
// этим списком мы пользуемся при выборе нужного узла в терминале адаптера-прошивальщика 
// (копипастим имя нужного узла в строку отправки терминала)
// также этим списком можно пользоваться для распечатки где-либо названия имени узла 
// делатся так, например Serial.print (PRINT_NAME[SHITOK_RELAYS]);
// или если нужно работать со строкой имени узла -  добавляем к си-шным строковым функциям в конце _P 
// например if (strcmp_P("Vykl_zal", STRING_NAME(VYKL_ZAL))==0) {делаем то то;} // если строка имени совпала с заданной нами , делаем то то

  const char Incognito     [] PROGMEM = "Incognito"; 
  const char Shitok_relays [] PROGMEM = "Shitok_relays"; 
  const char Vykl_zal      [] PROGMEM = "Vykl_zal"; 
  const char Vykl_prihozh  [] PROGMEM = "Vykl_prihozh"; 
  const char Podmoykoy     [] PROGMEM = "Podmoykoy";
  const char Vykl_detsk    [] PROGMEM = "Vykl_detsk";
  const char Vykl_kuhnya   [] PROGMEM = "Vykl_kuhnya";
  const char Vykl_vanna    [] PROGMEM = "Vykl_vanna";
  const char Vykl_kladovka [] PROGMEM = "Vykl_kladovka";
  const char Vykl_koridor  [] PROGMEM = "Vykl_koridor";
  const char Vykl_spalnya  [] PROGMEM = "Vykl_spalnya";
  const char Vykl_kabinet  [] PROGMEM = "Vykl_kabinet";
  const char Broadcast     [] PROGMEM = "Broadcast";


// третий список для работы прогмем (схема хранения строк во флеш), просто заполняем также , 
// сами элементы в списке те же , что и во втором списке, последовательность соблюдаем c первым списком  

const char* const Node_Name[NODES_QUANTITY+1] PROGMEM = 
{
Incognito,      // INCOGNITO,     // 00
Shitok_relays,  // SHITOK_RELAYS, // 01  
Vykl_zal,       // VYKL_ZAL,      // 02  
Vykl_prihozh,   // VYKL_PRIHOZH,  // 03  
Podmoykoy,      // PODMOYKOY,     // 04  
Vykl_detsk,     // VYKL_DETSK ,   // 05  
Vykl_kuhnya,    // VYKL_KUHNYA,   // 06  
Vykl_vanna,     // VYKL_VANNA,    // 07  
Vykl_kladovka,  // VYKL_KLADOVKA, // 08  
Vykl_koridor,   // VYKL_KORIDOR,  // 09  
Vykl_spalnya,   // VYKL_SPALNYA,  // 0A  
Vykl_kabinet,   // VYKL_KABINET,  // 0B  
Broadcast       // BROADCAST  
};

// ниже в четвертом списке задаем реальные адреса узлов 
byte Nodeaddress[NODES_QUANTITY] 
{
 0x00, // INCOGNITO,     // 00
 0x10, // SHITOK_RELAYS, // 01  
 0x15, // VYKL_ZAL,      // 02  
 0xDA, // VYKL_PRIHOZH,  // 03  
 0xCC, // PODMOYKOY,     // 04  
 0xAA, // VYKL_DETSK ,   // 05  
 0x89, // VYKL_KUHNYA,   // 06  
 0x99, // VYKL_VANNA,    // 07  
 0x14, // VYKL_KLADOVKA, // 08  
 0xAE, // VYKL_KORIDOR,  // 09  
 0xE5, // VYKL_SPALNYA,  // 0A  
 0xDC, // VYKL_KABINET,  // 0B  
 
 0xFF, // BROADCAST  
};

#define NODEADDR_FROM_ENUM  // раскомментировать эту строку, если хотим, чтобы адреса узлов совпадали с нумерацией узлов (с первым списком)
                            // то есть то что выше в списке Nodeaddress станет неактуально и в этот список внесутся адреса нумерации узлов 0,1,2,3 и тд.  

// ниже дефайны не трогаем, они для работы скетча
#define _NORMAL_MODE               0 
#define _REFLASH_MODE              1
#define _SETTING_MODE              2
#define _GET_STATUS_MODE           3
#define _REFLASH_MODE_REQUEST      4
#define _REFLASH_START             5
#define _BOOTLOADER_START          6
#define _REFLASH_END               7
#define _SILENT_WINDOW_MODEON      8
#define _SILENT_WINDOW_MODEOFF     9
#define _SILENT_WINDOW_MODEREQUEST 10
#define _CANTEST                   11
