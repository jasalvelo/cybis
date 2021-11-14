const int Emegnet = 9;
const int LED = 8;
const int Sensor = A0;  // analog in
int count = 1;

#define I2C_LOWDRIVER_ADRESS (0x59)   //Defaultadress for Lowdriver 1011001
#include <Wire.h>

boolean bPumpState[4]={false,false,false,false};    // bool means T/F  //only use pump #1
uint8_t nPumpVoltageByte[4] = {0x00,0x00,0x00,0x00}; 
uint8_t nFrequencyByte = 0x40;
uint8_t bBoardversion = 0;

void selectControlRegisters() {                   // for lawdriver_init
  Wire.beginTransmission(I2C_LOWDRIVER_ADRESS);
  Wire.write(0xFF); 
  Wire.write(0x00);
  Wire.endTransmission();  
}

void selectMemoryRegisters() {                   // for landriver_init
  Wire.beginTransmission(I2C_LOWDRIVER_ADRESS);
  Wire.write(0xFF); 
  Wire.write(0x01);
  Wire.endTransmission();  
}

void Board_init() {
  pinMode(A3,INPUT_PULLUP);   //Boardversion detection A3=GND -> rev3
  bBoardversion = ((digitalRead(A3)==HIGH) ? 2 : 3); 
}

void Driver_setvoltage(uint8_t _pump, uint8_t _voltage) {
  bPumpState[0]=true;
  Lowdriver_setvoltage(_voltage);
}

void Driver_setfrequency(uint16_t _frequency) {
  Lowdriver_setfrequency(_frequency);
}

void Pump_switchOff(uint8_t _pump=0) {
  bPumpState[0] = false;
  Lowdriver_setvoltage(nPumpVoltageByte[0]);
}

void Pump_switchOn(uint8_t _pump=0) {
  bPumpState[0] = true;
  Lowdriver_setvoltage(nPumpVoltageByte[0]);
}        

void Lowdriver_init() {
  selectControlRegisters();
  Wire.beginTransmission(I2C_LOWDRIVER_ADRESS);
  Wire.write(0x01);//Select (Control) Register 0x01
  Wire.write(0x02);//Set Gain 0-3 (0x00-0x03 25v-100v)
  Wire.write(0x00);//Take device out of standby mode
  Wire.write(0x01);//Set sequencer to play WaveForm ID #1
  Wire.write(0x00);//End of sequence
  Wire.endTransmission();
  selectMemoryRegisters();
  Wire.beginTransmission(I2C_LOWDRIVER_ADRESS);
  Wire.write(0x00); //Select Register 0x00
  Wire.write(0x05); //Header size –1
  Wire.write(0x80); //Start address upper byte (page), also indicates Mode 3
  Wire.write(0x06); //Start address lower byte (in page address)
  Wire.write(0x00); //Stop address upper byte
  Wire.write(0x09); //Stop address Lower byte
  Wire.write(0x00); //Repeat count, play WaveForm once                      // 0 = infinite loop
  Wire.write(0x00); //Initial Amplitude 0V
  Wire.write(0x0C); //Initial Frequency (100Hz)
  Wire.write(100);  //cycles
  Wire.write(0x00); //envelope
  Wire.endTransmission();
  delay(10);
  selectControlRegisters();
  Wire.beginTransmission(I2C_LOWDRIVER_ADRESS);
  Wire.write(0x02); //Set page register to control space
  Wire.write(0x01); //Set GO bit (execute WaveForm sequence)ss
  Wire.endTransmission();
  bPumpState[0] = false;
  nPumpVoltageByte[0] = 255;
}

void Lowdriver_setvoltage (uint8_t _voltage) {
  float temp = _voltage; temp*=255; temp/=150;   //150Vpp = 0xFF
  nPumpVoltageByte[0] = constrain(temp,0,255);
  Wire.beginTransmission(I2C_LOWDRIVER_ADRESS);
  Wire.write(0x02); //Stop Waveform playback
  Wire.write(0x00);
  Wire.endTransmission();
  selectMemoryRegisters();
  Wire.beginTransmission(I2C_LOWDRIVER_ADRESS);
  Wire.write(0x06); //Set page register to control space
  Wire.write((bPumpState[0] ? nPumpVoltageByte[0] : 0));    //0-255
  Wire.endTransmission(); 
  delay(10);
  selectControlRegisters();
  Wire.beginTransmission(I2C_LOWDRIVER_ADRESS);
  Wire.write(0x02); //Start Waveform playback
  Wire.write(0x01);
  Wire.endTransmission();
  Serial.println("voltage set");
}

void Lowdriver_setfrequency(uint16_t _frequency) {
  float temp = _frequency; temp/=7.8125;
  nFrequencyByte = constrain(temp,1,255);
  if (nFrequencyByte==0) nFrequencyByte=1;
  Wire.beginTransmission(I2C_LOWDRIVER_ADRESS);
  Wire.write(0x02); //Stop Waveform playback
  Wire.write(0x00);
  Wire.endTransmission();
  selectMemoryRegisters();
  Wire.beginTransmission(I2C_LOWDRIVER_ADRESS);
  Wire.write(0x07); //Set page register to control space
  Wire.write(nFrequencyByte);    //0-255
  Wire.endTransmission(); 
  delay(10);
  selectControlRegisters();
  Wire.beginTransmission(I2C_LOWDRIVER_ADRESS);
  Wire.write(0x02); //Start Waveform playback
  Wire.write(0x01);
  Wire.endTransmission();
  Serial.println("frequency set");
}

void setup(){
  Serial.begin(9600);
  Wire.begin();
  pinMode(A3,INPUT_PULLUP);  
  pinMode (LED, OUTPUT);
  pinMode (Emegnet, OUTPUT);
  pinMode (Sensor, INPUT);
  Board_init();
  Lowdriver_init(); // two times to prevent bug (from the company)
  delay (1000);
  Lowdriver_init(); 
  delay (1000);
  Driver_setfrequency (400);  // 1-800
  delay (1000);
  Driver_setvoltage (1,10);   // 1-150
  delay (1000);
  Pump_switchOn(1);
  delay (1000);
  Pump_switchOff(1);
  delay (1000);
  Pump_switchOn(1);
  delay (1000);
  Pump_switchOff(1);
  delay (1000);
  Serial.println("test start");
}

void loop (){
  // we should make our respective methods and call them here
}
