## Hệ thống quan trắc chất lượng nước sử dụng IoT & Machine Learning

---

## 📌 Giới thiệu

Dự án xây dựng hệ thống **giám sát chất lượng nước trong nuôi trồng thủy sản ngoài biển** theo thời gian thực.

Hệ thống sử dụng:
- ESP32 + LoRa để thu thập và truyền dữ liệu  
- Cảm biến đo các thông số nước (pH, nhiệt độ, độ mặn, ...)  
- Machine Learning để phân tích và cảnh báo  

👉 Mục tiêu: giúp người nuôi theo dõi từ xa và phát hiện sớm rủi ro.

---

## 🧠 Công nghệ sử dụng

- Phần cứng: ESP32, LoRa RA-01, cảm biến nước  
- Giao tiếp: RS485 (Modbus RTU), SPI, WiFi  
- Machine Learning: Prophet  
- Server & Web Dashboard  

---

## 🏗️ Hệ thống tổng thể

### 📷 Sơ đồ hệ thống
<!-- Thêm ảnh sơ đồ hệ thống -->
![System Architecture](images/system.png)

---

## 🔧 Phần cứng

### 📷 Cảm biến sử dụng
<!-- Thêm ảnh cảm biến -->
![Sensor](images/sensor.png)

---

## 📊 Kết quả

### 📷 Giao diện giám sát
<!-- Thêm ảnh dashboard -->
![Dashboard](images/dashboard.png)

### 📷 Dữ liệu thực nghiệm
<!-- Thêm ảnh biểu đồ / kết quả -->
![Result](images/result.png)
