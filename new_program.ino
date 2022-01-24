#include <LiquidCrystal.h>
#include <Wire.h>
#include <EEPROM.h>

#define pumpControlPin 13
#define sleepPin 19
#define directionSignalPin 12
#define valve2Pin 11
#define valve3Pin 18
#define pressureSensorPin A0
#define leftArrowKeyPin 9
#define rightArrowKeyPin 10
#define enterKeyPin 8

#define buttonPressed LOW
#define stopPump HIGH
#define runPump LOW
#define forward HIGH
#define backward LOW
#define valveOff LOW
#define valveOn HIGH
#define sleepOn LOW
#define sleepOff HIGH
#define LEFT 1
#define RIGHT 2
#define ENTER 3
#define START_MENU 1
#define TEST_BEGIN 0
#define UPPERPRESSURE_PARAM_MENU 2
#define LOWERPRESSURE_PARAM_MENU 3
#define TEST_TIME_MENU 4
#define SPEED_MENU 5
#define START_IN_MANUAL_MODE_MENU 6
#define CALIBRATION_MENU 7
#define MANUAL_TEST_MENU 8
#define VERIFY_PASSWORD_MENU 9
#define CALIBRATION_BEGIN 20
#define CALIBRATION_STEP1 21
#define CALIBRATION_STEP2 22
#define CALIBRATION_STEP3 23
#define CALIBRATION_DONE 24
#define UPPERPRESSURE_PARAM_MENU_CHANGE 12
#define LOWERPRESSURE_PARAM_MENU_CHANGE 13
#define TEST_TIME_MENU_CHANGE 14
#define SPEED_MENU_CHANGE 15
#define DO_NOTHING 0
#define FAIL 0
#define PASS 1
#define SLAVE_I2C_ADRESS 4                  //adress that has been set for slave i2c bus device (pump control arduino)

#define NO_DEVICE_SELECTED 0
#define IN_TEST_MENU 0
#define UNDERPRESSURE_TEST 0
#define OVERPRESSURE_TEST 1
#define BOTH_TESTS 2

///////// config //////////
#define eeAddressOffset 0                             //Eeprom adress at which manometer offset is stored
#define eeAddressK 4                               //Eeprom adress at which manometer K factor is stored
#define pressureDeviationLimitForSelfTest 10                // maximal pressure deviation limit for self test - in daPa           
#define CALIBRATION_LOWERPRESSURE_VALUE -400      // lowerpressure value that as to be set for calibration - in daPa
#define CALIBRATION_UPPERPRESSURE_VALUE 600     // upperpressure value that as to be set for calibration - in daPa
#define READ_MANOMETER_OFFSET_SAMPLES 10    // number of samples that should been taken to calulate the offset value for manometer
#define DEFAULT_UPPERPRESSURE 400                 // default upperpressure after program start - in daPa
#define DEFAULT_LOWERPRESSURE -400                // default lowerpressure after program start - in daPa
#define DEFAULT_TEST_TIME 10                      // default test time after program start - in seconds
#define DEFAULT_PUMP_SPEED                        // default pump speed level after program start
#define DISPLAY_DELAY_DURING_PUMPING_TEST 500     //cycles
#define DISPLAY_DELAY_DURING_LEAKING_TEST 1500    //cycles             
#define PRESSURE_DEVIATION_BEFORE_TEST 80         //dPa
#define DEFAULT_PRESSURE_DEVIATION_LIMIT 10       //dPa
#define MAX_LOWERPRESSURE -1000                   // maximal lowerpressure that can be set
#define MAX_UPPERPRESSURE 1000                    // maximal upperpressure that can be set
int password[5] = {LEFT, RIGHT, RIGHT, RIGHT, ENTER};         // passowrd to enter calibration mode - only 5 signs can be set
double manometerOffset = 501;      //////// to do - odczytaj offset z internal eepromu       // manometer offset value - default is 501  
double manometerKFactor = 2.564102564102564;        // entered value is the K factor (should also be calibrated)
int delayTime;
double calculatedPressure;
bool valve3Status = false;
int param[4] = {DEFAULT_UPPERPRESSURE, DEFAULT_LOWERPRESSURE, DEFAULT_TEST_TIME, DEFAULT_PUMP_SPEED};                       //store parameters' values for specyfied tests
 
int add_value[4] = {10, 10, 1, 1};                     //store the value representing one point of param   
                                                       //adding 1 value point to UPPERPRESSURE param will add 10 dPa
                                                      //adding 1 value point to LOWERPRESSURE param will add 10 dPa
                                                      //adding 1 value point to TEST_TIME param will add 1s
                                                      //adding 1 value point to PUMP_SPEED param will add 1 speed level
bool state;
int i;
int returnKeyDetect;
int readButtonState();
bool upperpressureStatus = true;
bool lowerpressureStatus = true;
bool verifyPassword();
bool selfTest();
void dispalyTestBeginingCommunicate();
void displayPressureMeasurement(double calculatedPressure);
void displayPressureDeviationMeasurement(double pressureDeviation);
void displayVentingCommunicate();
void displayTestResult(int testsNumber);
void displayTestTime (int testTime, int testTimeTmp);
void displayPressureError();
void pressureError();
void displayBlockedError();
void blockedError();
void showMenu(int currentMenu_tmp);
void DoTheUnderpressureTest();
void DoTheOverpressureeTest();
void sendPumpSpeedToslave();
void handleManualTest();
void DoTheCalibration();
void displaySelfTestResult(int result);
void waitForENTER();
double readPressure();
double maxPressureDeviation[2] = {0, 0};
double readManometerOffset();
double readManometerKFactor();
int DUT = NO_DEVICE_SELECTED;
int currentMenu = 1;
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup()
{
  Serial.begin(9600);
  Wire.begin(); 
  // set up pump controller:
  pinMode(sleepPin, OUTPUT);
  pinMode(pumpControlPin, OUTPUT);
  pinMode(directionSignalPin, OUTPUT);
  digitalWrite(sleepPin, sleepOn);
  digitalWrite(pumpControlPin, stopPump);
  pinMode(directionSignalPin, forward);   //set dafault direction 
  
  // set up valves: 
  pinMode(valve2Pin, OUTPUT);
  pinMode(valve3Pin, OUTPUT);
  digitalWrite(valve2Pin, valveOff);
  digitalWrite(valve3Pin, valveOff);
  
  // set up keyboard:
  pinMode(leftArrowKeyPin, INPUT_PULLUP);
  pinMode(rightArrowKeyPin, INPUT_PULLUP);
  pinMode(enterKeyPin, INPUT_PULLUP);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  // read required calibration parameters from internal Eeprom
  //EEPROM.get(eeAddressOffset, manometerOffset);
  //EEPROM.get(eeAddressK, manometerKFactor);
  
  //Do the self-test and go to welcome screen
  selfTest();
  showMenu(START_MENU);
}
  ////////////////////// Main program loop //////////////////////
void loop()
{
  handleMenu(); 
}

int readButtonState()
{
  if ((digitalRead(leftArrowKeyPin)) == LOW)
  {
    return LEFT;
  } else if ((digitalRead(rightArrowKeyPin)) == LOW)
  {
    return RIGHT;
  } else if ((digitalRead(enterKeyPin)) == LOW)
  {
    return ENTER;
  }
  else 
  {
    return DO_NOTHING;
  }
}

void showMenu(int currentMenu_tmp)
{
  switch (currentMenu_tmp)
  {
  case START_MENU:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nacisnij <ENTER> aby");
    lcd.setCursor(0, 1);
    lcd.print("rozpoczac test");
    lcd.setCursor(6, 3);
    lcd.print("zmien parametry ->");
    lcd.display();
    break;
    
    case TEST_BEGIN:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Rozpoczynam test");
    lcd.display();
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("Rozpoczynam test.");
    lcd.display();
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("Rozpoczynam test..");
    lcd.display();
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("Rozpoczynam test...");
    lcd.display();
    delay(1000);
    break;

    case UPPERPRESSURE_PARAM_MENU:
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("<<NADCISNIENIE>>");
    lcd.setCursor(0, 1);
    lcd.print("Test aktywny:    " );
    lcd.setCursor(18, 1);
    if(upperpressureStatus)
    {
      lcd.print("TAK");
    }else
    {
      lcd.print("NIE");
    }
    lcd.setCursor(0, 2);
    lcd.print("Target: ");
    lcd.setCursor(8, 2);
    lcd.print(param[0]);
    lcd.setCursor(12, 2);
    lcd.print("daPa");
    lcd.setCursor(0, 3);
    lcd.print("<- back");
    lcd.setCursor(9, 3);
    lcd.print("<zmien>");
    lcd.setCursor(18, 3);
    lcd.print("next->");
    lcd.display();
    break;
    

  case LOWERPRESSURE_PARAM_MENU:
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("<<PODCISNIENIE>>");
    lcd.setCursor(0, 1);
    lcd.print("Test aktywny:    " );
    lcd.setCursor(18, 1);
    if(lowerpressureStatus)
    {
      lcd.print("TAK");
    }else
    {
      lcd.print("NIE");
    }
    lcd.setCursor(0, 2);
    lcd.print("Target: ");
    lcd.setCursor(8, 2);
    lcd.print(param[1]);
    lcd.setCursor(12, 2);
    lcd.print("daPa");
    lcd.setCursor(0, 3);
    lcd.print("<- back");
    lcd.setCursor(9, 3);
    lcd.print("<zmien>");
    lcd.setCursor(18, 3);
    lcd.print("next->");
    lcd.display();
    break;
    
  case TEST_TIME_MENU:
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("<<CZAS TESTU>>");
    lcd.setCursor(9, 1);
    lcd.print(param[3]);
    lcd.setCursor(12, 1);
    lcd.print("s");
    lcd.setCursor(0, 3);
    lcd.print("<- back");
    lcd.setCursor(9, 3);
    lcd.print("<zmien>");
    lcd.setCursor(18, 3);
    lcd.print("next->");
    lcd.display();
    break;
    
  case SPEED_MENU:
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("<<PREDKOSC POMPKI>>");
    lcd.setCursor(9, 1);
    lcd.print(param[4]);
    lcd.setCursor(12, 1);
    lcd.print("rpm");
    lcd.setCursor(0, 3);
    lcd.print("<- back");
    lcd.setCursor(9, 3);
    lcd.print("<zmien>");
    lcd.setCursor(18, 3);
    lcd.print("next->");
    lcd.display();
    break;
    
  case START_IN_MANUAL_MODE_MENU:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nacisnij <ENTER> aby");
    lcd.setCursor(0, 1);
    lcd.print("rozpoczac test");
    lcd.setCursor(0, 3);
    lcd.print("<-zmien parametry");
    lcd.display();
    break; 
    
    
  case UPPERPRESSURE_PARAM_MENU_CHANGE:
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("<<NADCISNIENIE>>");
    lcd.setCursor(0, 1);
    lcd.print("Target: ");
    lcd.setCursor(8, 1);
    lcd.print(param[0]);
    lcd.setCursor(12, 1);
    lcd.print("daPa");
    lcd.setCursor(0, 3);
    lcd.print("<-  -");
    lcd.setCursor(7, 3);
    lcd.print("<zaakceptuj>");
    lcd.setCursor(19, 3);
    lcd.print("+  ->");
    lcd.display();
    break;
    
  case LOWERPRESSURE_PARAM_MENU_CHANGE:
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("<<PODCISNIENIE>>");
    lcd.setCursor(0, 1);
    lcd.print("Target: ");
    lcd.setCursor(8, 1);
    lcd.print(param[1]);
    lcd.setCursor(12, 1);
    lcd.print("daPa");
    lcd.setCursor(0, 3);
    lcd.print("<-  -");
    lcd.setCursor(7, 3);
    lcd.print("<zaakceptuj>");
    lcd.setCursor(19, 3);
    lcd.print("+  ->");
    lcd.display();
    break;
  case TEST_TIME_MENU_CHANGE:
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("<<CZAS TESTU>>");
    lcd.setCursor(9, 1);
    lcd.print(param[2]);
    lcd.setCursor(12, 1);
    lcd.print("s");
    lcd.setCursor(0, 3);
    lcd.print("<-  -");
    lcd.setCursor(7, 3);
    lcd.print("<zaakceptuj>");
    lcd.setCursor(19, 3);
    lcd.print("+  ->");
    lcd.display();
    
  case SPEED_MENU_CHANGE:
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("<<PREDKOSC POMPKI>>");
    lcd.setCursor(9, 1);
    lcd.print(param[3]);
    lcd.setCursor(12, 1);
    lcd.print("rpm");
    lcd.setCursor(0, 3);
    lcd.print("<-  -");
    lcd.setCursor(7, 3);
    lcd.print("<zaakceptuj>");
    lcd.setCursor(19, 3);
    lcd.print("+  ->");
    lcd.display();
    break;
  case MANUAL_TEST_MENU:
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Zawor: ");
    lcd.setCursor(7, 1);
    if(valve3Status)
    {
      lcd.print("zamkniety");
    }else
    {
      lcd.print("otwarty");
    }
    lcd.setCursor(0, 3);
    lcd.print("<-  -");
    lcd.setCursor(7, 3);
    lcd.print("<zawor>");
    lcd.setCursor(19, 3);
    lcd.print("+  ->");
    lcd.display();
    break;
  
  case CALIBRATION_MENU:
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("<<KALIBRACJA>>");
    lcd.setCursor(0, 3);
    lcd.print("<- back");
    lcd.setCursor(9, 3);
    lcd.print("<zmien>");
    lcd.display();
    break;
  
   case VERIFY_PASSWORD_MENU:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Aby dokonac strojenia");
    lcd.setCursor(0, 1);
    lcd.print("testera, podaj haslo: ");
    lcd.display();
    break;
  
   case CALIBRATION_BEGIN:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Rozpoczynam strojenie");
    lcd.display();
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("Rozpoczynam strojenie.");
    lcd.display();
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("Rozpoczynam strojenie..");
    lcd.display();
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("Rozpoczynam strojenie...");
    lcd.display();
    delay(500);
    break;

   case CALIBRATION_STEP1:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Odlacz wszystko od");
  lcd.setCursor(0, 1);
    lcd.print("gniazda cisnienia i");
  lcd.setCursor(0, 2);
    lcd.print("nacisnij <ENTER>");
  lcd.display();
    break; 
  
   case CALIBRATION_STEP2:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("podlacz referencyjny");
  lcd.setCursor(0, 1);
    lcd.print("manometr do gniazda");
  lcd.setCursor(0, 2);
    lcd.print("cisnienia, podaj ");
  lcd.setCursor(17, 2);
  lcd.print(CALIBRATION_LOWERPRESSURE_VALUE);
  lcd.setCursor(20, 2);
  lcd.print("daPa");
    lcd.setCursor(0, 3);
  lcd.print("i nacisnij <ENTER>");
  lcd.display();
    break;
   case CALIBRATION_STEP3:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("podlacz referencyjny");
  lcd.setCursor(0, 1);
    lcd.print("manometr do gniazda");
  lcd.setCursor(0, 2);
    lcd.print("cisnienia, podaj ");
  lcd.setCursor(17, 2);
  lcd.print(CALIBRATION_UPPERPRESSURE_VALUE);
  lcd.setCursor(20, 2);
  lcd.print("daPa");
    lcd.setCursor(0, 3);
  lcd.print("i nacisnij <ENTER>");
  lcd.display();
    break;
   case CALIBRATION_DONE:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Strojenie zakonczone!");
  lcd.setCursor(0, 1);
  lcd.print("Nacisnij <ENTER> aby");
  lcd.setCursor(0, 2);
  lcd.print("wyjsc.");
  lcd.display();
    break;
  
    default:;
  }
}

void DoTheUnderpressureTest()
{
  sendPumpSpeedToslave();
  unsigned long startTime = 0;
  unsigned long pumpingTime = 1;
  double refPressure = 0;
  double pressureDeviation = 0;
  int testTime = 0; 
  int lowerPressure = param[1];
  int upperPressure = param[0];
  int testTimeTmp = param[2];
  i = 0;
  
  //dispalyTestBeginingCommunicate();

  digitalWrite(valve3Pin, valveOn);
  digitalWrite(sleepPin, sleepOff);
  digitalWrite(directionSignalPin, backward);
  startTime = ( millis() / 1000 );
  calculatedPressure = readPressure();
  displayPressureMeasurement(calculatedPressure);
  digitalWrite(pumpControlPin, runPump);
  while ( (calculatedPressure = readPressure()) > lowerPressure && (digitalRead(enterKeyPin) != LOW) )
  {
    if(i >= DISPLAY_DELAY_DURING_LEAKING_TEST)
      {
        displayPressureMeasurement(calculatedPressure);
        i = 0;
      }else
      {
        i++;
      }
  }
  digitalWrite(pumpControlPin, stopPump);
  digitalWrite(sleepPin, sleepOn);
    testTime = 0;
    i = 0;  
    displayPressureMeasurement(calculatedPressure);
    delay(3000);
    displayPressureMeasurement(calculatedPressure);
    displayPressureDeviationMeasurement(pressureDeviation);
    startTime = ( millis() / 1000 );
    refPressure = readPressure();
    displayPressureMeasurement(refPressure);
    refPressure = abs(refPressure);
    if( ( abs(calculatedPressure) - refPressure ) > PRESSURE_DEVIATION_BEFORE_TEST)
    {
      pressureError();
    }else
    {
      displayPressureDeviationMeasurement(pressureDeviation);
      while (( ( testTime = ((millis() / 1000) - startTime) ) < testTimeTmp) && (digitalRead(enterKeyPin) != LOW) )
      {
        calculatedPressure = readPressure();
        pressureDeviation = refPressure - abs(calculatedPressure);
        maxPressureDeviation[UNDERPRESSURE_TEST] = pressureDeviation;
        if (i >= DISPLAY_DELAY_DURING_LEAKING_TEST)
        {
          displayTestTime (testTime, testTimeTmp);
          displayPressureDeviationMeasurement(pressureDeviation);
          i = 0;
        } else
        {
          i++;
        }
      }
    }
      digitalWrite(valve3Pin, valveOff);
}
      
void DoTheOverpressureTest()
{
  sendPumpSpeedToslave();
  unsigned long pumpingTime = 1;
  unsigned long startTime = 0;
  double refPressure = 0;
  double pressureDeviation = 0;
  int testTime = 0; 
  int lowerPressure = param[1];
  int upperPressure = param[0];
  int testTimeTmp = param[2];
  i = 0;
  
  lcd.clear();
  digitalWrite(directionSignalPin, forward);
  digitalWrite(sleepPin, sleepOff);
  digitalWrite(valve3Pin, valveOn);
  startTime = ( millis() / 1000 );
  calculatedPressure = readPressure();
  displayPressureMeasurement(calculatedPressure);
  digitalWrite(pumpControlPin, runPump);
  while ((calculatedPressure = readPressure()) < upperPressure && (digitalRead(enterKeyPin) != LOW) )
  {
    if(i >= DISPLAY_DELAY_DURING_LEAKING_TEST)
      {
        displayPressureMeasurement(calculatedPressure);
        i = 0;
      }else
      {
        i++;
      }
  }
    digitalWrite(sleepPin, sleepOn);
    digitalWrite(pumpControlPin, stopPump);
    
      testTime = 0;
      i = 0;  
      displayPressureMeasurement(calculatedPressure);
      delay(3000);
      displayPressureMeasurement(calculatedPressure);
      displayPressureDeviationMeasurement(abs(pressureDeviation));
      startTime = ( millis() / 1000 );
      refPressure = abs(readPressure());
      displayPressureMeasurement(refPressure);
      displayPressureDeviationMeasurement(pressureDeviation);
      while ( ( testTime = ((millis() / 1000) - startTime) ) < testTimeTmp)
      {
        calculatedPressure = readPressure();
        pressureDeviation = refPressure - abs(calculatedPressure);
        maxPressureDeviation[OVERPRESSURE_TEST] = pressureDeviation;
        if (i >= DISPLAY_DELAY_DURING_LEAKING_TEST)
        {
          displayTestTime (testTime, testTimeTmp);
          displayPressureDeviationMeasurement(pressureDeviation);
          i = 0;
        } else
        {
          i++;
        }
      }
      digitalWrite(valve3Pin, valveOff);     
}

void sendPumpSpeedToslave()
{
  Wire.beginTransmission(SLAVE_I2C_ADRESS);           // transmit to device #SLAVE_I2C_ADRESS
  Wire.write(param[3]);                                           // sends pump speed
  Wire.endTransmission();                                    // stop transmitting
}

void dispalyTestBeginingCommunicate()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Rozpoczynam test");
  lcd.setCursor(0, 1);
  lcd.print("szczelnosci");
  lcd.display();
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("szczelnosci.");
  delay(600);
  lcd.setCursor(0, 1);
  lcd.print("szczelnosci..");
  delay(600);
  lcd.setCursor(0, 1);
  lcd.print("szczelnosci...");
  delay(600);
  lcd.clear();
}

void displayTestResult(int testsNumber)
{
  int returnKeyDetect = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ulot przy:");
  lcd.setCursor(0, 1);
  if(upperpressureStatus)
  {
    lcd.print("-nadcisnieniu:");
    lcd.setCursor(14, 1);
    lcd.print( abs( maxPressureDeviation[OVERPRESSURE_TEST] ));
    lcd.setCursor(20, 1);
    lcd.print("daPa"); 
  }
  if(lowerpressureStatus)
  {
    lcd.setCursor(0, 2);
    lcd.print("-podcisnieniu:");
    lcd.setCursor(14, 2);
    lcd.print( abs( maxPressureDeviation[UNDERPRESSURE_TEST] ));
    lcd.setCursor(20, 2);
    lcd.print("daPa");   
  }
      lcd.display();
    waitForENTER();
}

void displaySelfTestResult(int result)
{
  if(result == PASS)
  {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("SELF TEST");
    lcd.setCursor(5, 1);
    lcd.print("<<PASS>>");
    lcd.setCursor(5, 2);
    lcd.print("Wcisnij <ENTER> aby");
    lcd.setCursor(5, 3);
    lcd.print("przejsc dalej");   
  }else
  {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("SELF TEST");
    lcd.setCursor(5, 1);
    lcd.print("<<FAIL>>");
    lcd.setCursor(5, 2);
    lcd.print("Zawiadom dzial techniczny");   
  }
    lcd.display();
  waitForENTER();
}

int calculate_frequency(int rpm_tmp)
{
  float frequency_tmp = rpm_tmp * 8;
  float T_tmp = (1 / frequency_tmp) * 1000;
  int delayTime_tmp = T_tmp / 2;
  return (delayTime_tmp);
}

void displayPressureMeasurement(double calculatedPressure)
{
  lcd.setCursor(0, 0);
  lcd.print("Cisnienie: ");
  lcd.setCursor(11, 0);
  lcd.print(calculatedPressure);
  lcd.setCursor(17, 0);
  lcd.print("dPa");
  lcd.display();
}

void displayPressureDeviationMeasurement(double pressureDeviation)
{
  lcd.setCursor(0, 2);
  lcd.print("Ulot: ");
  lcd.setCursor(6, 2);
  lcd.print(pressureDeviation, 2);
  //lcd.setCursor(12, 2);
  //lcd.print("dPa");
  lcd.display();
}

void displayTestTime (int testTime, int testTimeTmp)
{
  lcd.setCursor(0, 1);
  lcd.print("Czas: ");
  lcd.setCursor(6, 1);
  lcd.print("  ");
  lcd.setCursor(6, 1);
  lcd.print( (testTimeTmp - testTime) );
  lcd.display();
}

void displayPressureError()
{
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("<< FAIL >>");
  lcd.setCursor(0, 1);
  lcd.print("Nieszczelny uklad,");
  lcd.setCursor(0, 2);
  lcd.print("sprawdz wezyki!");
  lcd.display();
}

void displayBlockedError()
{
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("<< FAIL >>");
  lcd.setCursor(0, 1);
  lcd.print("Zablokowany uklad,");
  lcd.setCursor(0, 2);
  lcd.print("sprawdz pozycje");
  lcd.setCursor(0, 3);
  lcd.print("dzwigni!");
  lcd.display();
}

void displayVentingCommunicate()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Udraznianie ukladu");
  lcd.display();
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("Udraznianie ukladu.");
  lcd.display();
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("Udraznianie ukladu..");
  lcd.display();
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("Udraznianie ukladu...");
  lcd.display();
}

double readPressure()
{
  int pressureSensorValue = analogRead(pressureSensorPin);
  double calculatedPressure = -(pressureSensorValue - manometerOffset) * manometerKFactor;         
  return (calculatedPressure);
}

void pressureError()
{
  digitalWrite(valve3Pin, valveOff);
  displayPressureError();
  waitForENTER();
}

void blockedError()
{
  digitalWrite(valve3Pin, valveOff);
  displayBlockedError();
  waitForENTER();
}

//Pump control code
void do_the_step (int delayTime_tmp)
{
  digitalWrite(pumpControlPin, LOW);
  delayMicroseconds(1000);
  digitalWrite(pumpControlPin, HIGH);
  delayMicroseconds(1000);
}
  
void handleMenu()
{ 
int i = 0;
int testsNumber = 0;
int option = readButtonState();       // store button state
if (option != DO_NOTHING)       
 {
  if(currentMenu < 10)              //first menu layer
  {
     switch (option)
    {
      case LEFT:
        if(currentMenu != TEST_BEGIN)
        {
          currentMenu--;
          showMenu(currentMenu);
        }
        break;
      case RIGHT:
        if(currentMenu != CALIBRATION_MENU)
        {
          currentMenu++;
          showMenu(currentMenu);
        }
        break;
      case ENTER:
        if(currentMenu != START_MENU && currentMenu != START_IN_MANUAL_MODE_MENU && currentMenu != CALIBRATION_MENU)     // go to second menu layer
        {
          currentMenu+=10;
          showMenu(currentMenu);
        }else if (currentMenu == START_MENU)                                  // automatic test handle
        {
          showMenu(TEST_BEGIN);
          if(lowerpressureStatus)                                                    // check if lowerPressure test should be done
          {
            DoTheUnderpressureTest();                                            // do the lowerpressure test
            testsNumber = UNDERPRESSURE_TEST;                            //save that only lowerpressure test result should be viewed
          }
          if(upperpressureStatus)                                                    // check if upperPressure test should be done                           
          {
      DoTheOverpressureTest();                                              //do the upperPressure test
            if(lowerpressureStatus)                                                 // if lowerpressure test have been done
            {
        testsNumber = BOTH_TESTS;                                         // save that both tests results should be viewed
            }else
            {
        testsNumber = OVERPRESSURE_TEST;                                // else save that only upperPressure test result should be viewed
            }
          }
          displayTestResult(testsNumber);                                                    // view test results and wait for clicking ENTER to accept it
          currentMenu = START_MENU;                                                    // after displaying test results go back to main menu
          showMenu(currentMenu);
        }else if (currentMenu == START_IN_MANUAL_MODE_MENU)                   // manual test handle
        {
      showMenu(TEST_BEGIN);
      handleManualTest();
      showMenu(currentMenu);
        }else if (currentMenu == CALIBRATION_MENU)
    {
      showMenu(VERIFY_PASSWORD_MENU);
      if (verifyPassword())
      {
        DoTheCalibration();
        showMenu(currentMenu);
      }else
      {
        showMenu(currentMenu);
      }
    }
        break;
      default:;
    }
  }else //second menu layer (changing parameters)
  {
    switch (option)
    {
      case LEFT:
        if(param[currentMenu - 12] > MAX_LOWERPRESSURE)
        {
          param[currentMenu - 12] -= add_value[currentMenu - 12];     //subtract one value point from the specyfied parameter
          showMenu(currentMenu);
        }
        break;
      case RIGHT:
        if(param[currentMenu - 12] < MAX_UPPERPRESSURE)
        {
          param[currentMenu - 12] += add_value[currentMenu + 12];     //add one value point from the specyfied parameter
          showMenu(currentMenu);
        }
        break;
      case ENTER:
        currentMenu-=10;
        showMenu(currentMenu);
        break;
      default:;
    }
  } 
  }
}

void handleManualTest()
{
  sendPumpSpeedToslave();
    showMenu(MANUAL_TEST_MENU);
    calculatedPressure = readPressure();
    displayPressureMeasurement(calculatedPressure);
    digitalWrite(sleepPin, sleepOff);
    while( digitalRead(leftArrowKeyPin) != buttonPressed && digitalRead(enterKeyPin) != buttonPressed)    //need to set up loop to not exit switch case
    {
        switch(readButtonState())                                           //check which button have been clicked
        {
            case LEFT:
        digitalWrite(directionSignalPin, backward);                           // if left arrow has been clicked, set direction signal pin to backward
        digitalWrite(pumpControlPin, runPump);                              // run pump backward
        while(digitalRead(leftArrowKeyPin) == buttonPressed )                 // checking if button is still pressed 
        {   
          calculatedPressure = readPressure();
          displayPressureMeasurement(calculatedPressure);
          if(i >= DISPLAY_DELAY_DURING_PUMPING_TEST)
          { 
            displayPressureMeasurement(calculatedPressure);
            i = 0;
          }else
          {
            i++;
          }
        }
        digitalWrite(pumpControlPin, stopPump);                           // if button has been released just turn off pump             
        break;
            case RIGHT:
                digitalWrite(directionSignalPin, forward);                            // if right arrow has been clicked, set direction signal pin to backward
                digitalWrite(pumpControlPin, runPump);                              // run pump backward
                while(digitalRead(rightArrowKeyPin) == buttonPressed )                // checking if button is still pressed 
                { 
          calculatedPressure = readPressure();
          displayPressureMeasurement(calculatedPressure);
          if(i >= DISPLAY_DELAY_DURING_PUMPING_TEST)
          { 
            displayPressureMeasurement(calculatedPressure);
            i = 0;
          }else
          {
            i++;
          }                                                           
                }
                digitalWrite(pumpControlPin, stopPump);                           // if button has been released just turn off pump
                break;
            case ENTER:                                                   // if enter button has been pressed just change valve state
                if(digitalRead(valve3Pin) == valveOff)
                {
          digitalWrite(valve3Pin, valveOn);
          valve3Status = true;
                }else
                {
          digitalWrite(valve3Pin, valveOff);
          valve3Status = false;
                }
                break;
        }
    }
    digitalWrite(valve3Pin, valveOff);
    digitalWrite(pumpControlPin, stopPump);
    digitalWrite(sleepPin, sleepOn);  
}

bool verifyPassword()
{
  int enteredPassword[5];
  bool returnValue = true;
  for (int i = 0; i < 5; i++)
  {
    while(readButtonState() == DO_NOTHING)
    {
    }
    enteredPassword[i] = readButtonState();
    if(enteredPassword[i] != password[i])
    {
      returnValue = false;
    }
  }
  return returnValue;
}

void DoTheCalibration()
{
  double tmp;
  showMenu(CALIBRATION_BEGIN);
  manometerOffset = readManometerOffset();
  do{
    EEPROM.put(eeAddressOffset, manometerOffset);
    delay(500);   
  } while(EEPROM.get(eeAddressOffset, tmp) != manometerOffset);
  manometerKFactor = readManometerKFactor();
  do{
      EEPROM.put(eeAddressK, manometerKFactor);
      delay(500);
  } while(EEPROM.get(eeAddressK, tmp) != manometerKFactor);
  showMenu(CALIBRATION_DONE);
  waitForENTER();
}

double readManometerOffset()
{
  double readPressureValue = 0;
  
  showMenu(CALIBRATION_STEP1);
  waitForENTER();
  for (int i = 0; i < READ_MANOMETER_OFFSET_SAMPLES; i++)
  {
    readPressureValue += readPressure();
  }
  readPressureValue = readPressureValue / READ_MANOMETER_OFFSET_SAMPLES;
  return readPressureValue;
}

double readManometerKFactor()
{
  double calculatedPressure;
  double K = 0;
  int pressureSensorValue = analogRead(pressureSensorPin);
  
  showMenu(CALIBRATION_STEP2);
  waitForENTER();
  calculatedPressure = -(pressureSensorValue - manometerOffset) * manometerKFactor;
  K = -(calculatedPressure / ( -(pressureSensorValue - manometerOffset)) );
  showMenu(CALIBRATION_STEP3);
  waitForENTER();
  calculatedPressure = -(pressureSensorValue - manometerOffset) * manometerKFactor;
  K = K + (calculatedPressure / ( -(pressureSensorValue - manometerOffset)));
  K = K / 2;
  return K;
}

bool selfTest()
{
  DoTheUnderpressureTest();
  delay(1000);  
  DoTheOverpressureTest();
  if (abs( maxPressureDeviation[UNDERPRESSURE_TEST] ) > pressureDeviationLimitForSelfTest  || abs( maxPressureDeviation[OVERPRESSURE_TEST] ) > pressureDeviationLimitForSelfTest )
  {
    displaySelfTestResult(FAIL);
  }else
  {
    displaySelfTestResult(PASS);      
  }
}

void waitForENTER()
{
  returnKeyDetect = readButtonState();
  while(returnKeyDetect != ENTER)
  {
    returnKeyDetect = readButtonState();
  }
  while(returnKeyDetect == ENTER)
  {
    returnKeyDetect = readButtonState(); 
  }
}