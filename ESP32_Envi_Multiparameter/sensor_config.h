#define DEFAULT_ENVI_SENS_ADDRESS 0x01

const byte cmd_read_addr[] = {0x01,0x03,0x00,0x0E,0x00,0x01}; //OK
const byte cmd_read_format[] = {0x01,0x03,0x06,0x05,0x00,0x01}; //OK
const byte cmd_read_baud[] = {0x01,0x03,0x00,0x12,0x00,0x01};

/**/
const byte cmd_read_temp[]        = {0x01,0x03,0x00,0x30,0x00,0x02};  //2 bytes to read
const byte cmd_read_ph[]          = {0x01,0x03,0x00,0x4A,0x00,0x02};  //2 bytes to read
const byte cmd_read_salinity[]    = {0x01,0x03,0x00,0x58,0x00,0x02};  //2 bytes to read
/**/

#define NUM_TIME_REQ_OPTIC 20		// tương đương với khoảng thời gian 1 tiếng

  enum ADDR_PARAM{
    ADDR_MODBUS = 0x000E,  //modbus address of sensor

    ADDR_TEMP = 0x0030,  //address of temperature parameter
		ADDR_PH = 0x004A,  //address of  parameter
    ADDR_SALINTY = 0x0058,  //address of  parameter
	};


//các dữ liệu cảm biến
float fTemperature;   //cảm biến nhiệt độ (độ C)
float fPH;            //độ PH của nước
float fSalinity;      //độ mặn của nước (tính từ giá trị Conductivity) (ppt)
unsigned long sensor_val;
bool bReqSens = false;

	// Danh sách ngưỡng
	const int numParams = 8;
	int currentParam = 0;
	
	float params[numParams] = {0};  // Ví dụ dữ liệu	

	unsigned long lastSendTime = 0;
	const unsigned long sendInterval = 20000;
	
	bool bMonitor = false;  //cờ báo hiệu xử lý giám sát để hiệu chỉnh tham số
	bool bCalibrated = false; //cờ báo hiệu thực hiện xong hiệu chỉnh
	bool bCalibrating = false;	//cờ báo hiệu bắt đầu thực hiện hiệu chỉnh
	
	unsigned long prev_read_sens;
	unsigned int timming_sensor = 3000;
	byte params_sensor_value = 1;
	bool bSensResp = false;

	bool bGetSensors = false;	//cờ xác định trạng thái xử lý đọc dữ liệu cảm biến	
	bool bTxSensor = false;	//cờ báo hiệu đã gửi dữ liệu cảm biến cho GW	

	const byte my_addr = 0xB1;	//địa chỉ lora của nút mạng