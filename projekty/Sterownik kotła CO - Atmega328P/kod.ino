//------------ DEFINITINS -----------------//
// keys definitions
#define SW_UP 17
#define SW_DOWN 16
#define SW_OK 15
#define SW_ESC 14
// buzzer definitions
#define BUZZER 13
// outputs definitions
#define PROT_RELAY 12
#define AIR_CTRL 11
#define WATER_CTRL 10
// lcd connections
#define LCD_SCK 9
#define LCD_DIN 8
#define LCD_CD 7
#define LCD_CE 6
#define LCD_RST 5
// temperature sensors
#define T_SENS 4
// defined errors
#define DS_ERROR 1
#define START_ERROR 2
#define OVERHEATING 3
#define END_OF_HEATING 4
#define CLOCK_ERROR 5
// limits for paramaters
#define OVERHEAT_TEMP 85
#define MAX_TEMP 80
#define MIN_TEMP 40
#define MIN_FAN_PWR 30
#define MAX_FAN_PWR 100
#define MAX_PUMP_TEMP_DIF 15
#define MIN_PUMP_TEMP_DIF 1
// default eeprom val
#define EEPROM_DEF_PUMP_TEMP_DIFF 8
#define EEPROM_DEF_MIN_FAN_PWR 40
#define EEPROM_DEF_MAX_FAN_PWR 100
#define EEPROM_DEF_MAX_START_TIME 18000
#define EEPROM_DEF_SET_TEMP 55
// other
#define OCR_1SEC 49999
#define EEPROM_START_ADDR 0
#define EEPROM_SET_TEMP_ADDR 100
//------------ INCLUDES -----------------//
// lcd
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
// clock
#include <Wire.h>
#include <DS1307.h>
// temp sensors
#include <OneWire.h>
#include <DallasTemperature.h>
// memory
#include <EEPROM.h>

//------- CLASS AND STRUCTURES -----------//
struct Furnace {
  public:
    volatile float fan_temp;     // current water temp in furnace
    volatile float pump_temp;    // current water temp on return
    int min_fan_pwr = MIN_FAN_PWR;
    int max_fan_pwr = MAX_FAN_PWR;
    volatile int pump_temp_diff;
    volatile int set_temp;
    volatile uint16_t start_flag = 1;
    uint16_t max_start_time; // 30 min for start
    int8_t last_pwm_state = 0;
    float last_fan_temp = 0;
};

// PCD8544
//Adafruit_PCD8544 display = Adafruit_PCD8544(SCK, DIN, CD, CE, RST);
Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_SCK, LCD_DIN, LCD_CD, LCD_CE, LCD_RST);
// DS3231
DS1307 clock;
RTCDateTime dt;
// DS18B20
OneWire ourWire(T_SENS);
DallasTemperature sensors(&ourWire);
uint8_t ds18b20_fan[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // sensors address
uint8_t ds18b20_pump[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t no_sensors = 0; // number of sensors
volatile int flag_get_temp = 0;
volatile int flag_check_sensors = 0;
volatile int flag_calc_pwr = 1;
volatile int flag_get_time = 0;
volatile int eeprom_update_flag = 0;

volatile uint8_t keys = 0;

volatile int menu_mode = 0;

Furnace fur;

// pseudo scheduler
volatile uint16_t event_counter = 0;

//------- SETUP FUNCTION --------------//
void setup() {
  int a = 0;
  EEPROM.get(0, a);
  if (a == 255) { // if new uC
    WriteInitValeuToEeprom(); // write initial values
  }
  ReadParamFromEeprom();
  DisplayInit(); // lcd init
  sensors.begin();// temp sensors init
  CheckSensors();
  sensors.requestTemperatures();
  delay(750);
  PinsInit();
  PwmInit();
  flag_get_time = 10;
  clock.begin(); // clock init
  flag_get_time = 0;
  fur.pump_temp = sensors.getTempC(ds18b20_pump);
  fur.fan_temp = sensors.getTempC(ds18b20_fan);
  fur.last_fan_temp = fur.fan_temp;
  fur.last_pwm_state = 10;
  PwmSet(fur.last_pwm_state);
  ProtRelayOn();
}
//------- MAIN FUNCTION ---------------//
void loop() {
  if (menu_mode == 0) {
    // meas temp event
    if (flag_get_temp) {
      fur.pump_temp = sensors.getTempC(ds18b20_pump);
      fur.fan_temp =  sensors.getTempC(ds18b20_fan);
      sensors.requestTemperatures();
      flag_get_time = 10;
      dt = clock.getDateTime();
      flag_get_time = 0;
      flag_get_temp = 0;
      DisplayDrawMainScreen();// draw screen event
    }
    if (flag_calc_pwr == 1) {
      CalcAndSetFanPwr();
      flag_calc_pwr = 0;
    }
    // check sensors are broken
    if (flag_check_sensors) {
      CheckSensors();
      flag_check_sensors = 0;
    }
    if (eeprom_update_flag == 1) {
      EEPROM.update(EEPROM_SET_TEMP_ADDR, (int)fur.set_temp);
      eeprom_update_flag == 0;

    }
  }
  if (menu_mode == 1) {
    DrawMenu();
    DisplayDrawMainScreen();// draw screen event
  }
}
//------- OTHER FUNCTIONS ---------------//
void DisplayInit(void) {
  // lcd init
  display.begin();
  display.setContrast(55);
  display.clearDisplay();
  // draw spalshscreen
  display.setCursor(15, 0);
  display.print("Sterownik");
  display.setCursor(27, 9);
  display.print("kotla");
  display.setTextSize(2);
  display.setCursor(18, 18);
  display.print("C.O.");
  display.setCursor(0, 40);
  display.setTextSize(1);
  display.print("wyk P. Lawnik");
  display.display();
}

void DisplayDrawMainScreen() {
  display.clearDisplay();// clear display buffer
  // draw time
  display.setCursor(0, 0);
  display.print("Czas:");
  if (dt.hour < 10) display.print('0');
  display.print(dt.hour);   display.print(":");
  if (dt.minute < 10) display.print('0');
  display.print(dt.minute); display.print(":");
  if (dt.second < 10) display.print('0');
  display.print(dt.second);
  // draw rest
  display.setCursor(0, 8);
  display.print("T.CO    ");
  display.print(fur.fan_temp);
  display.setCursor(0, 16);
  display.print("T.WODY");
  display.print(fur.pump_temp);
  display.setCursor(0, 24);
  display.print("T.ZAD  ");
  display.print(fur.set_temp);
  display.setCursor(0, 32);
  display.print("BIEG: ");
  display.print(fur.last_pwm_state);
  display.setCursor(0, 40);
  if (fur.start_flag > 0) display.print("START");
  else display.print("PRACA");

  display.display();// update
}

void CheckSensors() {
  byte address[8];
  no_sensors = 0;
  ourWire.reset_search();
  while (ourWire.search(address)) {
    if (address[0] != 0x28)
      continue;
    if (OneWire::crc8(address, 7) == address[7]) // chech crc code
    {
      if (no_sensors == 0) {  // if sensors list is empty
        for (uint8_t k = 0; k < 8; k++) {
          ds18b20_fan[k] = address[k];
        }
        no_sensors++;
      }
      if (no_sensors == 1) {  // if its one sensor on list
        uint8_t same_id_flag = 0;
        for (uint8_t i = 0; i < 8; i++) {
          if (ds18b20_fan[i] == address[i]) same_id_flag++;
        }
        if (same_id_flag != 8) {
          for (uint8_t k = 0; k < 8; k++) {
            ds18b20_pump[k] = address[k];
          }
          no_sensors++;
        }
      }
    }
  }
  if (no_sensors != 2) halt(DS_ERROR); //!!!!!!!!!!!!!!!!!!!!!!!!!
};

// initialization pins
void PinsInit(void) {
  // pwm and relay
  pinMode(PROT_RELAY, OUTPUT);
  pinMode(AIR_CTRL, OUTPUT);
  pinMode(WATER_CTRL, OUTPUT);
  digitalWrite(PROT_RELAY, LOW);
  digitalWrite(AIR_CTRL, LOW);
  digitalWrite(WATER_CTRL, LOW);
  // key
  pinMode(SW_UP, INPUT_PULLUP);
  pinMode(SW_DOWN, INPUT_PULLUP);
  pinMode(SW_OK, INPUT_PULLUP);
  pinMode(SW_ESC, INPUT_PULLUP);
  // buzzer
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
}

// pwm for fan functions
void PwmStart(void) {
  TIMSK1 |= (1 << OCIE1B);
}
void PwmStop(void) {
  TIMSK1 &= ~(1 << OCIE1B);
}

// set power of fan <0-5>
void PwmSet(uint8_t power) {
  if (power > 10) power = 0;
  if (power > 0) {
    long int min_pwr = (fur.min_fan_pwr);
    long int max_pwr = (fur.max_fan_pwr);
    min_pwr *= OCR_1SEC;
    max_pwr *= OCR_1SEC;
    min_pwr /= 100;
    max_pwr /= 100;

    int pwr_step = (max_pwr - min_pwr) / 10;
    int ocr_val = min_pwr + power * pwr_step;
    OCR1B = OCR_1SEC - ocr_val;
    PwmStart();
  }
  else {
    PwmStop();
  }

}

void PwmInit() {
  TCCR1A = 0; TCCR1B = 0; TCNT1  = 0;   // clear timer registers at start
  OCR1A =  OCR_1SEC;                      // 1Hz
  TCCR1B |= (1 << WGM12);               // ctc mode
  TCCR1B |= (1 << CS11) | (1 << CS10);  // prescaler 64
  TIMSK1 |= (1 << OCIE1A);  // enable compare interrupth
}

void DisablePwmTimer(void) {
  TCCR1B = 0;
}

void halt(uint8_t error_code) {
  display.clearDisplay();// clear display buffer
  // hard fault errors
  if (error_code == DS_ERROR) {
    DisablePwmTimer();
    WaterPumpOn(); // enable pump
    PwmStop();// disable fan
    ProtRelayOff();
    display.setCursor(0, 0);
    display.print("DS18B20 ERROR");
    display.display();
    BuzzerOn();
    while (1);
  }
  if (error_code == OVERHEATING) {
    DisablePwmTimer();
    WaterPumpOn(); // enable pump
    PwmStop();// disable fan
    ProtRelayOff();
    display.setCursor(0, 0);
    display.print("OVERHEAT!!");
    display.display();
    BuzzerOn();
    while (1);
  }
  if (error_code == CLOCK_ERROR) {
    DisablePwmTimer();
    WaterPumpOn(); // enable pump
    PwmStop();// disable fan
    ProtRelayOff();
    display.setCursor(0, 0);
    display.print("DS1307 ERROR");
    display.display();
    BuzzerOn();
    while (1);
  }
  // work errors
  if (error_code == START_ERROR) {
    WaterPumpOn(); // enable pump
    PwmStop();// disable fan
    display.setCursor(0, 0);
    display.print("Start error");
    display.setCursor(0, 16);
    display.print("Press ESC");
    display.display();
    BuzzerOn();
    while (CheckButtons() != 8);
    PwmSet(10);// max power
    fur.start_flag = 1; // reset start timer
    BuzzerOff();
  }
  if (error_code == END_OF_HEATING) {
    WaterPumpOff(); // enable pump
    PwmStop();// disable fan
    display.setCursor(0, 0);
    display.print("End of heating");
    display.setCursor(0, 16);
    display.print("Press ESC");
    display.display();
    BuzzerOn();
    while (CheckButtons() != 8);
    WaterPumpOn();
    PwmSet(10);// max power
    fur.start_flag = 1; // reset start timer
    BuzzerOff();
  }
}

void WaterPumpOn(void) {
  digitalWrite(WATER_CTRL, HIGH);
}

void WaterPumpOff(void) {
  digitalWrite(WATER_CTRL, LOW);
}

void ProtRelayOn() {
  digitalWrite(PROT_RELAY, HIGH);
}
void ProtRelayOff() {
  digitalWrite(PROT_RELAY, LOW);
}

void BuzzerOn(void) {
  digitalWrite(BUZZER, HIGH);
}
void BuzzerOff(void) {
  digitalWrite(BUZZER, LOW);
}

void CalcAndSetFanPwr() {
  // star mode
  if (fur.start_flag > 0) { // if start mode
    if ((fur.fan_temp < fur.set_temp)) PwmSet(10);  // if not to reach set temp 
    else {  // if its reached						// stay wtih max power
      fur.start_flag = 0;     // of start timer
      fur.last_pwm_state = 0; // disable fan
      PwmSet(0);              // one more time
      fur.last_fan_temp = fur.fan_temp; // update last temp
    }
    if (fur.start_flag > fur.max_start_time) halt(START_ERROR); // if start time has 
  }																//expired throw error
  // normal mode
  if (fur.start_flag == 0)  // if controller is in normal mode
  {
    // between set_temp-1&&set_temp+1
    if (((fur.fan_temp - fur.set_temp) < 1) && ((fur.set_temp - fur.fan_temp) < 1)) {
      fur.last_fan_temp = fur.fan_temp;
      return;
    }

    if (fur.fan_temp < fur.set_temp) {  // if curre	nt temp<set temp
      if (fur.last_fan_temp >= fur.fan_temp) { // if last temp>current temp
        fur.last_fan_temp = fur.fan_temp;     // update temp
        fur.last_pwm_state++;                 // power++
        if (fur.last_pwm_state > 10) fur.last_pwm_state = 10;
        PwmSet(fur.last_pwm_state);         // set power
        return;                             // end function
      }
      fur.last_fan_temp = fur.fan_temp;
    }
    if (fur.fan_temp >= fur.set_temp) {   // same as upper if
      if ((fur.fan_temp >= fur.last_fan_temp)) {
        fur.last_fan_temp = fur.fan_temp;
        fur.last_pwm_state--;               // but opposite direction
        if (fur.last_pwm_state < 0) fur.last_pwm_state = 0;
        PwmSet(fur.last_pwm_state);
        return;
      }
      fur.last_fan_temp = fur.fan_temp;
    }
  }
}

void ReadParamFromEeprom() {
  int eeprom_address_cnt = EEPROM_START_ADDR;
  EEPROM.get(eeprom_address_cnt, fur.min_fan_pwr);
  eeprom_address_cnt += sizeof(int);
  EEPROM.get(eeprom_address_cnt, fur.max_fan_pwr);
  eeprom_address_cnt += sizeof(int);
  EEPROM.get(eeprom_address_cnt, fur.pump_temp_diff);
  eeprom_address_cnt += sizeof(volatile int);
  EEPROM.get(eeprom_address_cnt, fur.max_start_time);
  eeprom_address_cnt += sizeof(uint16_t);
  EEPROM.get(EEPROM_SET_TEMP_ADDR, fur.set_temp);
}

void WriteInitValeuToEeprom(void) {
  int eeprom_address_cnt = EEPROM_START_ADDR;
  EEPROM.put(eeprom_address_cnt, (int)EEPROM_DEF_MIN_FAN_PWR);
  eeprom_address_cnt += sizeof(int);
  EEPROM.put(eeprom_address_cnt, (int)EEPROM_DEF_MAX_FAN_PWR);
  eeprom_address_cnt += sizeof(int);
  EEPROM.put(eeprom_address_cnt, (int)EEPROM_DEF_PUMP_TEMP_DIFF);
  eeprom_address_cnt += sizeof(volatile int);
  EEPROM.put(eeprom_address_cnt, (int)EEPROM_DEF_MAX_START_TIME);
  eeprom_address_cnt += sizeof(uint16_t);
  EEPROM.put(EEPROM_SET_TEMP_ADDR, (int)EEPROM_DEF_SET_TEMP);
}

void SaveParamToEeprom(void) {
  int eeprom_address_cnt = EEPROM_START_ADDR;
  EEPROM.update(eeprom_address_cnt, fur.min_fan_pwr);
  eeprom_address_cnt += sizeof(int);
  EEPROM.update(eeprom_address_cnt, fur.max_fan_pwr);
  eeprom_address_cnt += sizeof(int);
  EEPROM.update(eeprom_address_cnt, fur.pump_temp_diff);
  eeprom_address_cnt += sizeof(volatile int);
  EEPROM.update(eeprom_address_cnt, fur.max_start_time);
  eeprom_address_cnt += sizeof(uint16_t);
}

void DrawMenu(void) {
  PwmStop();              // disable fan
  digitalWrite(AIR_CTRL, LOW); // hard disable fan
  digitalWrite(PROT_RELAY, LOW);
  WaterPumpOn();
  uint8_t menu_cnt = 0;
  while (1) {
    if (menu_cnt == 0) {
      while (1) {
        display.clearDisplay();// clear display buffer    display.setCursor(0, 0);
        display.print("Maksymalny");
        display.setCursor(0, 8);
        display.print("czas");
        display.setCursor(0, 16);
        display.print("rozpalania ");
        display.setCursor(0, 24);
        display.print("w sekundach:");
        display.setCursor(0, 32);
        display.print(fur.max_start_time / 5);
        display.display();
        uint8_t buttons_sts = CheckButtons();
        if (buttons_sts == 1) {
          fur.max_start_time += 50; // +10 sec
          if (fur.max_start_time > 36000) fur.max_start_time = 36000;
        }
        if (buttons_sts == 2) {
          fur.max_start_time -= 50; // -10 sec
          if (fur.max_start_time < 3000) fur.max_start_time = 3000;
        }
        if (buttons_sts == 4) {
          SaveParamToEeprom();
          menu_cnt++;
          break;
        }
        if (buttons_sts == 8) {
          menu_mode = 0;
          SaveParamToEeprom();
          digitalWrite(PROT_RELAY, HIGH);
          return;
        }
      }
    }
    if (menu_cnt == 1) {
      while (1) {
        display.clearDisplay();// clear display buffer    display.setCursor(0, 0);
        display.print("Minimalna");
        display.setCursor(0, 8);
        display.print("moc");
        display.setCursor(0, 16);
        display.print("dmuchawy");
        display.setCursor(0, 24);
        display.print("w procentach");
        display.setCursor(0, 32);
        display.print(fur.min_fan_pwr);
        display.display();
        uint8_t buttons_sts = CheckButtons();
        if (buttons_sts == 1) {
          fur.min_fan_pwr++;
          if (fur.min_fan_pwr > (fur.max_fan_pwr - 20)) {
            fur.min_fan_pwr = fur.max_fan_pwr - 20;
          }
        }
        if (buttons_sts == 2) {
          fur.min_fan_pwr--;
          if (fur.min_fan_pwr < MIN_FAN_PWR) fur.min_fan_pwr = MIN_FAN_PWR;
        }
        if (buttons_sts == 4) {
          SaveParamToEeprom();
          menu_cnt++;
          break;
        }
        if (buttons_sts == 8) {
          menu_mode = 0;
          SaveParamToEeprom();
          digitalWrite(PROT_RELAY, HIGH);
          return;
        }
      }
    }
    if (menu_cnt == 2) {
      while (1) {
        display.clearDisplay();// clear display buffer    display.setCursor(0, 0);
        display.print("Maksymalna");
        display.setCursor(0, 8);
        display.print("moc");
        display.setCursor(0, 16);
        display.print("dmuchawy");
        display.setCursor(0, 24);
        display.print("w procentach");
        display.setCursor(0, 32);
        display.print(fur.max_fan_pwr);
        display.display();
        uint8_t buttons_sts = CheckButtons();
        if (buttons_sts == 1) {
          fur.max_fan_pwr++;
          if (fur.max_fan_pwr > MAX_FAN_PWR) fur.max_fan_pwr = MAX_FAN_PWR;
        }
        if (buttons_sts == 2) {
          fur.max_fan_pwr--;
          if (fur.max_fan_pwr < (fur.min_fan_pwr + 20)) {
            fur.max_fan_pwr = (fur.min_fan_pwr + 20);
          }
        }
        if (buttons_sts == 4) {
          SaveParamToEeprom();
          menu_cnt++;
          break;
        }
        if (buttons_sts == 8) {
          menu_mode = 0;
          digitalWrite(PROT_RELAY, HIGH);
          SaveParamToEeprom();
          return;
        }
      }
    }
    if (menu_cnt == 3) {
      while (1) {
        display.clearDisplay();// clear display buffer    display.setCursor(0, 0);
        display.print("Prog");
        display.setCursor(0, 8);
        display.print("zalaczania");
        display.setCursor(0, 16);
        display.print("pompy");
        display.setCursor(0, 24);
        display.print("T_ZAD-T_POWR");
        display.setCursor(0, 32);
        display.print(fur.pump_temp_diff);
        display.display();
        uint8_t buttons_sts = CheckButtons();
        if (buttons_sts == 1) {
          fur.pump_temp_diff++;
          if (fur.pump_temp_diff > MAX_PUMP_TEMP_DIF)
            fur.pump_temp_diff = MAX_PUMP_TEMP_DIF;
        }
        if (buttons_sts == 2) {
          fur.pump_temp_diff--;
          if (fur.pump_temp_diff < MIN_PUMP_TEMP_DIF)
            fur.pump_temp_diff = MIN_PUMP_TEMP_DIF;
        }
        if (buttons_sts == 4) {
          menu_cnt++;
          if (menu_cnt == 4) menu_cnt = 0;
          SaveParamToEeprom();
          break;
        }
        if (buttons_sts == 8) {
          menu_mode = 0;
          digitalWrite(PROT_RELAY, HIGH);
          SaveParamToEeprom();
          return;
        }
      }
    }
  }
}

uint8_t CheckButtons(void) {
  uint8_t key_status = 0;
  if ((digitalRead(SW_UP) == LOW) || (digitalRead(SW_DOWN) == LOW) ||
      (digitalRead(SW_OK) == LOW) || (digitalRead(SW_ESC) == LOW)) {
    _delay_ms(20);
    if ((digitalRead(SW_UP) == LOW) || (digitalRead(SW_DOWN) == LOW) ||
        (digitalRead(SW_OK) == LOW) || (digitalRead(SW_ESC) == LOW)) {
      if ((digitalRead(SW_UP) == LOW)) key_status += 1; // up code
      if ((digitalRead(SW_DOWN) == LOW)) key_status += 2;// down code
      if ((digitalRead(SW_OK) == LOW)) key_status += 4; // ok code
      if ((digitalRead(SW_ESC) == LOW)) key_status += 8; // esc code
      while ((digitalRead(SW_UP) == LOW) || (digitalRead(SW_DOWN) == LOW) ||
             (digitalRead(SW_OK) == LOW) || (digitalRead(SW_ESC) == LOW));// wait to unpush keys
    }
  }
  if (key_status > 8) key_status = 0;
  return key_status;
}

// timers interrupths
ISR(TIMER1_COMPA_vect)
{
  if (!menu_mode) {
    if (fur.fan_temp > OVERHEAT_TEMP) { // hard fault
      noInterrupts();// disable interrupts
      halt(OVERHEATING); // overheat error
    }
    if (flag_get_time > 9) flag_get_time++; // hard clock fault
    if (flag_get_time > 29) halt(CLOCK_ERROR);

    if ((fur.fan_temp - fur.pump_temp) > fur.pump_temp_diff) WaterPumpOn();
    else WaterPumpOff(); // pump driver

    digitalWrite(AIR_CTRL, LOW);  // pwm for fan
    event_counter++;              // scheduler ++
    if (fur.start_flag > 0) fur.start_flag++; // start mode flag

    if ((event_counter % 5) / 4) flag_get_temp = 1;; // call temp event every 800ms
    if ((event_counter % 51) / 50) flag_check_sensors = 1; // call check sensor event every 10sec
    if ((event_counter % 601) / 600) flag_calc_pwr = 1; //call calculate power event every 120 sec
    // check key status
    noInterrupts();
    keys = CheckButtons();     // check key status
    interrupts();
     // switch setted temp
    if (keys & 1) { 
      keys = 0;
      fur.set_temp++;
      if (fur.set_temp > MAX_TEMP) fur.set_temp = 80;
      flag_get_temp = 1;
      eeprom_update_flag = 1;
    }
    if (keys & 2) {
      keys = 0;
      fur.set_temp--;
      if (fur.set_temp < MIN_TEMP) fur.set_temp < 40;
      flag_get_temp = 1;
      eeprom_update_flag = 1;
    }
    if (keys & 4 ) menu_mode = 1;
  }
}

ISR(TIMER1_COMPB_vect)
{
  if (!menu_mode){
    digitalWrite(AIR_CTRL, HIGH);// pwm for fan
  }
}


