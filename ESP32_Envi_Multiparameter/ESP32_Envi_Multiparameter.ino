#include "sys_config.h"
#include "MyLora.h"

MyLoRa lora;

// 👉 THÊM BIẾN ĐẾM SENSOR
uint8_t sensor_count = 0;

void loop(){      	  
	rs485_sensor_comm();  

  // 👉 CHỈ GỬI KHI ĐỦ 3 SENSOR
  if(bTxSensor){
    bTxSensor = false;   // reset cờ

    String payload = String("{\"temperature\":") + fTemperature +
                     ",\"pH\":" + fPH +
                     ",\"salinity\":" + fSalinity + "}";

    lora.send(payload);

    Serial.print("Sending: ");
    Serial.println(payload);
  }
}

// ================= RS485 FSM =================
void rs485_sensor_comm(){
	enum PROC_STATE{
		STATE_IDLE = 0,
		STATE_GET_SENS = 1,
		STATE_WAIT_RESP = 2,
		STATE_CHANGING = 3,		
		STATE_DELAY = 9
	};

	static unsigned char process_state = STATE_IDLE;  
	static char next_state;  
  static unsigned int time_wait;
	static unsigned long timing_sens_process;
	static byte num_retry_get_sens = 0;

	switch(process_state){

	case STATE_IDLE:
		if(num_retry_get_sens == 0)	time_wait = 8000;
		else time_wait = 2000;
		
		next_state = STATE_GET_SENS;
		process_state = STATE_DELAY;
		timing_sens_process = millis(); 	
		break;
	
	case STATE_GET_SENS:
		while(rs485.available()){	
			rs485.read();
			if(millis() - timing_sens_process > 2000)	break;
		}

		bReqSens = true;				
		get_sensor_data(params_sensor_value);              		
		process_state = STATE_WAIT_RESP;
		break;
	
	case STATE_WAIT_RESP:
		waiting_response_rs485();

		if(bRespOK){
			bSensResp = true;
						
			if(process_resp_envi_sensor(params_sensor_value) != params_sensor_value){
				num_retry_get_sens++;

				if(num_retry_get_sens < 2){
					process_state = STATE_IDLE;						
				}else{
					num_retry_get_sens = 0;
					process_state = STATE_CHANGING;
				}
			}
			else{
				num_retry_get_sens = 0;
				process_state = STATE_CHANGING;

        // 👉 ĐẾM SENSOR THÀNH CÔNG
        sensor_count++;

        // 👉 CHỈ GỬI KHI ĐỦ 3
        if(sensor_count >= 3){
          sensor_count = 0;
          bTxSensor = true;
        }
			}

			bRespOK = false;
			bReqSens = false;			
		}

		if((bRespTimeout == true) && (bReqSens == true)){
			bReqSens = false;
			bSensResp = false;
			process_state = STATE_CHANGING;
		}
		break;
		
	case STATE_CHANGING:
		if(bCalibrating == false){	
			process_state = STATE_IDLE;	
			
			if(params_sensor_value < 3) {
        params_sensor_value++;      
      } else {
        params_sensor_value = 1;     
      }	
		}				
		break;		
	
	case STATE_DELAY:
		if(millis() - timing_sens_process > time_wait){
      timing_sens_process = millis();
      process_state = next_state;
    }
		break;
	
	default:
		process_state = STATE_IDLE;
		next_state = process_state;
		timing_sens_process = millis();    
		break;
	}	
}

// ================= SETUP =================
void setup() { 
  Serial.begin(115200);  
	Wire.begin(SDA_PIN, SCL_PIN);
  delay(1000);  

	init_system();  
	Serial.println("Setup Done!");

  lora.begin();

  params_sensor_value = 1;
	bGetSensors = false;	
	bTxSensor = false;
}

// ================= INIT =================
void init_system(){			  	
  pinMode(RS485_TX_EN, OUTPUT);

  digitalWrite(RS485_TX_EN, RS485_RX_PIN_VALUE);

  init_rs485_comm();   
  bReqSens = false;  
}