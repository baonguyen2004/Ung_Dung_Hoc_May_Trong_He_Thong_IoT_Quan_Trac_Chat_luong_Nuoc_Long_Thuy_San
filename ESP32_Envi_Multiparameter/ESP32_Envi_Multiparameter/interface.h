
//---RS485 interface---//  
  #define RS485_RX_PIN 32
  #define RS485_TX_PIN 33
  #define RS485_TX_EN  25

  //RS485 control state
  #define RS485_TX_PIN_VALUE HIGH
  #define RS485_RX_PIN_VALUE LOW

  HardwareSerial rs485(2);  //using Serial2 of ESP32

  unsigned long rs485_timming;

  #define MAX_CMD_LENGTH 20
  #define MAX_RESP_LENGTH 40
  byte rs485_cmd[MAX_CMD_LENGTH];
  byte resp_data[MAX_RESP_LENGTH];

  int resp_id;
  byte resp_byte;
  byte rs485_address = 0x00;
  byte func_code;
  byte resp_len;  
  unsigned long rs485_timeout;
  byte resp_state = 0;

  unsigned int crc16;
  byte HB_crc16;  
  byte LB_crc16;
  bool bWaitResp = false;
  bool bRespOK = false;
  #define RESP_TIMEOUT 2000 //2s timeout
  bool bRespTimeout = false; //flag indicating timeout    