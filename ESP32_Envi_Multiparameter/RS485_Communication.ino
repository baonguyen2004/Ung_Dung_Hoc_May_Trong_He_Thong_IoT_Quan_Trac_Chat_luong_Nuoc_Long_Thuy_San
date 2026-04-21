//Hàm xử lý phản hồi từ thiết bị RS485 sau khi chương trình gửi đi một
//yêu cầu/lệnh cho thiết bị này 
//Hàm xử lý phản hồi từ thiết bị RS485 sau khi chương trình gửi đi một
//yêu cầu/lệnh cho thiết bị này 
void waiting_response_rs485(){  
  if(bRespTimeout == true)  return;
  
  //checking timeout response from rs485 sensor      
  if(millis() - rs485_timeout > RESP_TIMEOUT){ //timeout 2s
    bWaitResp = false;
    Serial.println("Response Timeout");
    resp_state = 0; //reset
    resp_id = 0;
    bRespTimeout = true;    
    return;      
  }
      
  /*---checking response from rs485 sensor---*/
  if(rs485.available()){      
    resp_byte = rs485.read(); //get response data
    
    switch (resp_state){
    case 0: //first byte - address byte of frame response      
      rs485_address = resp_byte;
      resp_data[resp_id] = resp_byte; //copy received data into buffer
      /**/
      if((rs485_address != 0x01) && (rs485_address != 0xAB))  
        Serial.println("Response Error[0] " + String(rs485_address,HEX));
      else{ 
        resp_state = 1; 
        resp_id = 1;
      }
      /**/        
      break;    
    case 1: //second byte - function code
      resp_data[resp_id] = resp_byte;      
      if(resp_byte != func_code)  Serial.println("Response Error[1]");
      else                        { resp_state = 2; resp_id = 2;}
      break;
    case 2: //third byte -- lenth of data in response
      resp_data[resp_id++] = resp_byte;      
      resp_len = resp_byte;        
      //else if(func_code == 0x06)  resp_len = 3;         //for write cmd        
      //else{ resp_state = 0; resp_id = 0; break;}      
      resp_state++;           
      break;
    default:
      resp_data[resp_id++] = resp_byte;
      if(resp_id == resp_len + 3){  //payload of response
        crc16 = CRC16(resp_data, resp_id);
        //Serial.println(crc16,HEX);
      }
      else if(resp_id == resp_len + 5){ //end of response
        Serial.print("\nEnd of response: [");
        Serial.print(String(millis() - rs485_timeout) + " ms] ");
        resp_state = 0;
        HB_crc16 = (crc16 >> 8) & 0xFF;  
        LB_crc16 = (byte)(crc16 & 0xFF);  
        if((HB_crc16 != resp_data[resp_len + 4]) || (LB_crc16 != resp_data[resp_len + 3]))
          Serial.println("Response Error[2]");
        else bRespOK = true;
        
        //logging response
        for(int i = 0; i < resp_id; i++){
          Serial.print(F("0x")); Serial.print(resp_data[i],HEX);Serial.print(F(" "));
        }
        Serial.println();               
      }
      else if((resp_id > resp_len + 5) || (resp_id >= sizeof(resp_data))){
        Serial.println("Response Error[3]");  //overflow and unknown
        resp_state = 0;
        resp_id = 0;
      }
      break;
    }    
  } 

  if(bRespOK) bRespTimeout = true;
}

//set request/command message from mater to slave over rs485 interface
/*định dạng khung thông tin theo chuẩn MOSBUS RS485*
- (1 byte) address
- (1 byte) function
- (2 byte) register address (địa chỉ của thanh ghi bắt đầu)
- (2 byte) register length (số lượng giá trị cần đọc, tùy thuộc vào số thanh ghi cần đọc giá trị)
- (2 byte) CRC bytes: byte tính CRC để kiểm tra lỗi cho khung thông tin tại phía thu
*/
void send_rs485_cmd(byte *cmd, int cmd_length){
  int i;

  while(rs485.available()) rs485.read();

  func_code = cmd[1];
  //copy data of request message into buffer to calculate CRC bits
  for(i = 0; i < cmd_length; i++) rs485_cmd[i] = cmd[i];
  
  //tính toán 2 byte CRC với bản tin cần gửi qua RS485
  unsigned int crc16 = CRC16(cmd, cmd_length);
  byte HB_crc16 = (crc16 >> 8) & 0xFF;  
  byte LB_crc16 = (byte)(crc16 & 0xFF);

  //add 2 bytes CRC16 to command message
  rs485_cmd[cmd_length]     = LB_crc16;
  rs485_cmd[cmd_length + 1] = HB_crc16;  
  cmd_length += 2;
 
  //logging
#if (EN_RS485_DEBUG == 1)  
  Serial.println();
  Serial.print(F("Send CMD[ "));
  for(i = 0; i < cmd_length; i++){    
    Serial.print(F("0x")); Serial.print(rs485_cmd[i],HEX);Serial.print(F(" "));
  }
  Serial.println(F(" ]"));
#endif    
        
  digitalWrite(RS485_TX_EN, HIGH);  //rs485 tx enable, change to tx mode
  for(i = 0; i < cmd_length; i++) rs485.write(rs485_cmd[i]);      
  rs485.flush();  //wait for transmit message
  digitalWrite(RS485_TX_EN, LOW); //rs485 rx enable (change to rx mode)
    
  delay(10);

  resp_id = 0;      //reset data counter
  resp_state = 0;   //state of processing response
  bWaitResp = true; //response state
  bRespOK = false;  //reset flag which indicate correct response  
  bRespTimeout = false;
  rs485_timeout = millis(); //timing waiting response from sensor    
}

void send_rs485_cmd(const byte *cmd, int cmd_length){
  int i;
  func_code = cmd[1];

  while(rs485.available()) rs485.read();
  
  //copy data of request message into buffer to calculate CRC bits
  for(i = 0; i < cmd_length; i++) rs485_cmd[i] = cmd[i];
  
  //tính toán 2 byte CRC với bản tin cần gửi qua RS485
  unsigned int crc16 = CRC16(cmd, cmd_length);
  byte HB_crc16 = (crc16 >> 8) & 0xFF;  
  byte LB_crc16 = (byte)(crc16 & 0xFF);

  //add 2 bytes CRC16 to command message
  rs485_cmd[cmd_length]     = LB_crc16;
  rs485_cmd[cmd_length + 1] = HB_crc16;  
  cmd_length += 2;
 
  //logging
#if (EN_RS485_DEBUG == 1)  
  Serial.println();
  Serial.print(F("Send CMD[ "));
  for(i = 0; i < cmd_length; i++){    
    Serial.print(F("0x")); Serial.print(rs485_cmd[i],HEX);Serial.print(F(" "));
  }
  Serial.println(F(" ]"));  
#endif

  digitalWrite(RS485_TX_EN, HIGH);  //rs485 tx enable, change to tx mode
  for(i = 0; i < cmd_length; i++) rs485.write(rs485_cmd[i]);      
  rs485.flush();  //wait for transmit message
  digitalWrite(RS485_TX_EN, LOW); //rs485 rx enable (change to rx mode)
    
  delay(10);

  resp_id = 0;      //reset data counter
  resp_state = 0;   //state of processing response
  bWaitResp = true; //response state
  bRespOK = false;  //reset flag which indicate correct response  
  bRespTimeout = false;
  rs485_timeout = millis(); //timing waiting response from sensor    
}

void read_envi_param(short addr_reg, short param_length){
  int i, cmd_len = 6;
  
  func_code = 0x03;
  rs485_cmd[0] = DEFAULT_ENVI_SENS_ADDRESS;
  rs485_cmd[1] = 0x03;  //function code = 0x03
  rs485_cmd[2] = (byte)(addr_reg >> 8);
  rs485_cmd[3] = (byte)addr_reg;
  rs485_cmd[4] = (byte)(param_length >> 8);
  rs485_cmd[5] = (byte)param_length;  
    
  //tính toán 2 byte CRC với bản tin cần gửi qua RS485
  unsigned int crc16 = CRC16(rs485_cmd, cmd_len);
  byte HB_crc16 = (crc16 >> 8) & 0xFF;  
  byte LB_crc16 = (byte)(crc16 & 0xFF);

  //add 2 bytes CRC16 to command message
  rs485_cmd[6] = LB_crc16;
  rs485_cmd[7] = HB_crc16;  
  cmd_len += 2;
 
  //logging
#if (EN_RS485_DEBUG == 1)  
  Serial.println();
  Serial.print(F("READ [ "));
  for(i = 0; i < cmd_len; i++){    
    Serial.print(F("0x")); Serial.print(rs485_cmd[i],HEX);Serial.print(F(" "));
  }
  Serial.println(F(" ]"));
#endif    
        
  digitalWrite(RS485_TX_EN, HIGH);  //rs485 tx enable, change to tx mode
  for(i = 0; i < cmd_len; i++) rs485.write(rs485_cmd[i]);      
  rs485.flush();  //wait for transmit message
  digitalWrite(RS485_TX_EN, LOW); //rs485 rx enable (change to rx mode)
    
  delay(10);

  resp_id = 0;      //reset data counter
  resp_state = 0;   //state of processing response
  bWaitResp = true; //response state
  bRespOK = false;  //reset flag which indicate correct response  
  bRespTimeout = false;
  rs485_timeout = millis(); //timing waiting response from sensor    
}

//hàm ghi dữ liệu kiểu nguyên (Ushort) vào thanh ghi của cảm biến envi multiparameters senso
void write_envi_param(short addr_reg, short param_value){
  int i, cmd_len = 6;
  
  func_code = 0x06;
  rs485_cmd[0] = DEFAULT_ENVI_SENS_ADDRESS;
  rs485_cmd[1] = 0x06;  //function code = 0x06 (write to hold register)
  rs485_cmd[2] = (byte)(addr_reg >> 8);
  rs485_cmd[3] = (byte)addr_reg;
  rs485_cmd[4] = (byte)(param_value >> 8);
  rs485_cmd[5] = (byte)param_value;  
    
  //tính toán 2 byte CRC với bản tin cần gửi qua RS485
  unsigned int crc16 = CRC16(rs485_cmd, cmd_len);
  byte HB_crc16 = (crc16 >> 8) & 0xFF;  
  byte LB_crc16 = (byte)(crc16 & 0xFF);

  //add 2 bytes CRC16 to command message
  rs485_cmd[6] = LB_crc16;
  rs485_cmd[7] = HB_crc16;  
  cmd_len += 2;
 
  //logging
#if (EN_RS485_DEBUG == 1)  
  Serial.println();
  Serial.print(F("WRITE [ "));
  for(i = 0; i < cmd_len; i++){    
    Serial.print(F("0x")); Serial.print(rs485_cmd[i],HEX);Serial.print(F(" "));
  }
  Serial.println(F(" ]"));
#endif    
        
  digitalWrite(RS485_TX_EN, HIGH);  //rs485 tx enable, change to tx mode
  for(i = 0; i < cmd_len; i++) rs485.write(rs485_cmd[i]);      
  rs485.flush();  //wait for transmit message
  digitalWrite(RS485_TX_EN, LOW); //rs485 rx enable (change to rx mode)
    
  delay(10);

  resp_id = 0;      //reset data counter
  resp_state = 0;   //state of processing response
  bWaitResp = true; //response state
  bRespOK = false;  //reset flag which indicate correct response  
  bRespTimeout = false;
  rs485_timeout = millis(); //timing waiting response from sensor    
}

//hàm ghi dữ liệu kiểu thập phân (float) vào thanh ghi của cảm biến envi multiparameters senso
/**
Modbus hoạt động với thanh ghi 16-bit, nhưng số float (IEEE 754) có 32-bit, 
nên ta cần chia nhỏ thành 2 thanh ghi 16-bit:
  - High Register (các byte trọng số cao)
  - Low Register  (các byte trọng số thấp)
/**/
void write_envi_param(short addr_reg, float param_value){
  int i, cmd_len = 11;
  uint16_t high_reg, low_reg;

  // Chuyển đổi float thành 2 thanh ghi 16-bit
  float_to_modbus_registers(param_value, &high_reg, &low_reg);
  
  func_code = 0x06;
  rs485_cmd[0] = DEFAULT_ENVI_SENS_ADDRESS;
  rs485_cmd[1] = 0x06;  //function code = 0x06 (write to hold register)
  rs485_cmd[2] = (byte)(addr_reg >> 8);
  rs485_cmd[3] = (byte)addr_reg;
  
  rs485_cmd[4] = 0x00;     // Số lượng thanh ghi (High byte) = 2
  rs485_cmd[5] = 0x02;     // Số lượng thanh ghi (Low byte) = 2
  rs485_cmd[6] = 0x04;     // Số bytes dữ liệu = 4 bytes
  rs485_cmd[7] = (byte)(high_reg >> 8);   // Dữ liệu High Register (High byte)
  rs485_cmd[8] = (byte)high_reg;          // Dữ liệu High Register (Low byte)
  rs485_cmd[9] = (byte)(low_reg >> 8);    // Dữ liệu Low Register (High byte)
  rs485_cmd[10] = (byte)low_reg;          // Dữ liệu Low Register (Low byte)
     
  //tính toán 2 byte CRC với bản tin cần gửi qua RS485
  unsigned int crc16 = CRC16(rs485_cmd, cmd_len);
  byte HB_crc16 = (crc16 >> 8) & 0xFF;  
  byte LB_crc16 = (byte)(crc16 & 0xFF);

  //add 2 bytes CRC16 to command message
  rs485_cmd[11] = LB_crc16;
  rs485_cmd[12] = HB_crc16;  
  cmd_len += 2;
 
  //logging
#if (EN_RS485_DEBUG == 1)  
  Serial.println();
  Serial.print(F("WRITE [ "));
  for(i = 0; i < cmd_len; i++){    
    Serial.print(F("0x")); Serial.print(rs485_cmd[i],HEX);Serial.print(F(" "));
  }
  Serial.println(F(" ]"));
#endif    
        
  digitalWrite(RS485_TX_EN, HIGH);  //rs485 tx enable, change to tx mode
  for(i = 0; i < cmd_len; i++) rs485.write(rs485_cmd[i]);      
  rs485.flush();  //wait for transmit message
  digitalWrite(RS485_TX_EN, LOW); //rs485 rx enable (change to rx mode)
    
  delay(10);

  resp_id = 0;      //reset data counter
  resp_state = 0;   //state of processing response
  bWaitResp = true; //response state
  bRespOK = false;  //reset flag which indicate correct response  
  bRespTimeout = false;
  rs485_timeout = millis(); //timing waiting response from sensor    
}

//hàm thực hiện chuyển đổi giá trị float 32-bit thành 2 thanh ghi 16-bit (Big Endian)
//theo định dạng của chuẩn IEEE 754 áp dụng cho modbus
void float_to_modbus_registers(float value, uint16_t *high_reg, uint16_t *low_reg) {
  union {
    float f;
    uint32_t i;
  } converter;
  converter.f = value;

  *high_reg = (converter.i >> 16) & 0xFFFF;
  *low_reg  = converter.i & 0xFFFF;
}

//hàm thiết lập dữ liệu kiểu thập phân (float) vào lệnh gửi qua rs485
void floatToBytes(float value, byte *bytes_array) {
  union {
    float f;
    byte b[4];
  } u;
  u.f = value;
  
  for (int i = 0; i < 4; i++) {
    bytes_array[i] = u.b[i]; // nếu cảm biến yêu cầu big-endian thì đảo thứ tự
  }
}

unsigned int CRC16(byte *arrData, int DataLength){
  unsigned int i,j,CheckSum;
  CheckSum = 0xFFFF;
  for (j = 0; j < DataLength; j++){
    CheckSum = CheckSum ^ arrData[j];
    for(i = 0; i < 8; i++){
      if((CheckSum) & 0x0001 == 1) CheckSum = (CheckSum >> 1)^0xA001;
      else CheckSum = CheckSum >> 1;
    }
  }
  return CheckSum;   
}

unsigned int CRC16(const byte *arrData, int DataLength){
  unsigned int i,j,CheckSum;
  CheckSum = 0xFFFF;
  for (j = 0; j < DataLength; j++){
    CheckSum = CheckSum ^ arrData[j];
    for(i = 0; i < 8; i++){
      if((CheckSum) & 0x0001 == 1) CheckSum = (CheckSum >> 1)^0xA001;
      else CheckSum = CheckSum >> 1;
    }
  }
  return CheckSum;   
}

void init_rs485_comm(){
  rs485.begin(9600, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);   
  while(rs485.available())  rs485.read();

  rs485_timming = millis();  
    
  bWaitResp = false;
  bRespTimeout = true;  //don't wait response from sensor device  
}