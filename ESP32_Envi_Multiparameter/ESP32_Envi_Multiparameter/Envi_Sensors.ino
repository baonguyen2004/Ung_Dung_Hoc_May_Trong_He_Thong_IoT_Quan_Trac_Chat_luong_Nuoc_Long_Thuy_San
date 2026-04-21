void send_setpoint_to_sensor(float setpoint) {
    byte cmd[6]; // 0xAB + 0x10 + 4 byte float = 6 byte (chưa tính CRC)

    cmd[0] = 0xAB;  // Header / Slave address
    cmd[1] = 0x10;  // Function code "Write Setpoint"
        
    // chuyển float sang 4 byte (Big-endian như slave đang đọc)
    union { 
        float f;
        uint8_t b[4];
    } u;
    u.f = setpoint;
    cmd[2] = u.b[0];
    cmd[3] = u.b[1];
    cmd[4] = u.b[2];
    cmd[5] = u.b[3];

    // Gửi qua hàm send_rs485_cmd
    send_rs485_cmd(cmd, 6);
}

void get_sensor_data(byte param_kind){  
  //gửi yêu cầu cho thiết bị để đọc dữ liệu cảm biến
  switch (param_kind){
  case 0:      
  Serial.print("\nRead Address of Sensor");
    send_rs485_cmd(cmd_read_addr, sizeof(cmd_read_addr));      
    break;    
  /**
  case 1:
    Serial.print("\nRead baud rate");
    send_rs485_cmd(cmd_read_baud, sizeof(cmd_read_baud));      
    break;
  case 4:      
    Serial.print("\nRead Inside Humidity");
    send_rs485_cmd(cmd_read_humi, sizeof(cmd_read_humi));      
    break;
  /**/      
  case 1:      
    Serial.print("\nRead Temperature");
    //send_rs485_cmd(cmd_read_temp, sizeof(cmd_read_temp));      
    read_envi_param(ADDR_TEMP, 2);
    break;    
  case 2:
    Serial.print("\nRead PH of water");
    //send_rs485_cmd(cmd_read_ph, sizeof(cmd_read_ph));                  
    read_envi_param(ADDR_PH, 2);
    /**
    - Nước uống (TCVN 01:2009/BYT)	6.5 - 8.5
    - Nước mặt (sông, hồ, ao, suối)	6.5 - 8.5
    - Nước thải sau xử lý	5.5 - 9.0
    - Nước nuôi trồng thủy sản	6.5 - 9.0
    - Nước mưa	5.0 - 6.5 (thấp hơn nếu mưa axit)
    /**/
    break;  
  case 3:
    Serial.print("\nRead Salinity");
    //send_rs485_cmd(cmd_read_salinity, sizeof(cmd_read_salinity));                  
    read_envi_param(ADDR_SALINTY, 2);
    /** Salinity (ppt hoặc PSU)
    - Nước ngọt	< 0.5 ppt
    - Nước lợ	0.5 - 30 ppt
    - Nước biển	30 - 50 ppt
    - Nước muối (siêu mặn)	> 50 ppt
    /**/
    break;  

  default:    
    break;
  }  
}

 byte process_resp_envi_sensor(byte param_kind){  
  if((param_kind < 1) || (param_kind > 3))  return 0;  //error

  resp_data[0] = (byte)my_addr; //địa chỉ I2C của mạch cảm biến
  resp_data[1] = param_kind;     //kiểu (loại) tham số cảm biến      

  byte tx_len = 0;		

  switch (param_kind){
  case 0:    
    Serial.println("device already...");
    break;  
  case 1:   //read Temperature                
    fTemperature = 0; //reset giá trị nhiệt độ trước đó
    memcpy(&fTemperature, &resp_data[3], 4);      
    if(fTemperature == 0){ 
      /*nếu giá trị nhiệt độ đọc về là 0 thì có thể đầu rò cảm biến đang bị lỗi do:
      -nguồn cấp cho cảm biến yếu
      -do chương trình xử lý của đầu rò cảm biến chưa thiết  lập và tính toán xong*/
      return 0;
    }
    params[0] = fTemperature;
    //resp_data[2] = 1;  //number of parameters sensor
    break;  
  case 2:   //read PH                
    memcpy(&fPH, &resp_data[3], 4);        
    params[4] = fPH;
    break;          
  case 3:   //read Salinity                
    memcpy(&fSalinity, &resp_data[3], 4);        
    params[3] = fSalinity;  
    break; 
  default:    
    break;
  }  
  
  //logging dữ liệu ra cửa sổ COM
	view_data_value(param_kind);  	
  return param_kind;
}

  void view_data_value(byte param_kind){  
  if((param_kind < 1) || (param_kind > 3))  return;

  float ph_val;
  
  switch (param_kind){  
  case 1:   //read Temperature                
    Serial.print("\tWater Temperature: " + String(fTemperature,2));Serial.println(" °C");    
    break;  
  case 2:   //read PH                    
    Serial.println("\tWater PH: " + String(fPH,2));      
    break;          
  case 3:   //read Salinity                    
    Serial.print("\tWater Salinity: " + String(fSalinity,2));Serial.println(" ppt");        
    break; 
  default:    
    break;
  }
}