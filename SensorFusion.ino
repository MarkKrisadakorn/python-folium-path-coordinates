#include <TinyGPSPlus.h>
#include <Adafruit_AHTX0.h>
#include <QMC5883LCompass.h>

#define ADXL345_ADDRESS (0x53)  // ADXL345 accelerometer address
#define ITG3205_ADDRESS (0x68)  // ITG3205 gyroscope address


TinyGPSPlus gps;
Adafruit_AHTX0 aht;
QMC5883LCompass compass;

sensors_event_t humidity, temp;
int x, y, z;
int16_t ax, ay, az;
int16_t gx, gy, gz;

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  Serial1.begin(9600,SERIAL_8N1, 26, 27);
  Serial2.begin(9600,SERIAL_8N1, 4, 25);
  Wire.begin();

  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }

  compass.init();

  // Initialize ADXL345
  writeTo(ADXL345_ADDRESS, 0x2D, 0); // Power register
  writeTo(ADXL345_ADDRESS, 0x2D, 16); // Measure mode

  // Initialize ITG3205
  writeTo(ITG3205_ADDRESS, 0x3E, 0); // Power register
  writeTo(ITG3205_ADDRESS, 0x15, 0x07); // Full-scale range and DLPF setting


}

void loop() {
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }

  // Only print if location and date/time are valid
  if (gps.location.isUpdated() && gps.date.isValid() && gps.time.isValid()) {
    // Format: date,time,lat,lon,altitude,satellites
    char buffer[100];
    snprintf(buffer, sizeof(buffer), 
             "%02d/%02d/%04d,%02d:%02d:%02d,%.6f,%.6f,%.2f,%d,",
             gps.date.day(), gps.date.month(), gps.date.year(),
             gps.time.hour(), gps.time.minute(), gps.time.second(),
             gps.location.lat(), gps.location.lng(),
             gps.altitude.meters(), gps.satellites.value());

    Serial1.print(buffer);
  
  // Data from AHT10 sensor
  
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  Serial1.print(temp.temperature);
  Serial1.print(",");
  Serial1.print(humidity.relative_humidity);
  Serial1.print(",");


  // Data from 9 DoF sensor
  // Read accelerometer data
  
  readFrom(ADXL345_ADDRESS, 0x32, 6, &ax);
  Serial1.print(ax); 
  Serial1.print(",");
  Serial1.print(ay);
  Serial1.print(","); 
  Serial1.print(az);
  Serial1.print(","); 

  // Read gyroscope data

  readFrom(ITG3205_ADDRESS, 0x1D, 6, &gx);
  Serial1.print(gx);
  Serial1.print(","); 
  Serial1.print(gy);
  Serial1.print(","); 
  Serial1.print(gz);
  Serial1.print(","); 

  // Read compass values
  compass.read();
  // Return XYZ readings
  x = compass.getX();
  y = compass.getY();
  z = compass.getZ();

  Serial1.print(x);
  Serial1.print(",");
  Serial1.print(y);
  Serial1.print(",");
  Serial1.println(z);
  }
}


void writeTo(int device, byte address, byte val) {
  Wire.beginTransmission(device);
  Wire.write(address);
  Wire.write(val);
  Wire.endTransmission();
}

void readFrom(int device, byte address, int num, int16_t *values) {
  Wire.beginTransmission(device);
  Wire.write(address);
  Wire.endTransmission();
  
  Wire.requestFrom(device, num);

  int i = 0;
  while (Wire.available() && i < num) {
    values[i] = Wire.read() | (Wire.read() << 8);
    i++;
  }
}