#include <LiquidCrystal.h>

#define pumpControlPin 13
#define sleepPin 19
#define directionSignalPin 12
#define valve2Pin 11
#define valve3Pin 18
#define pressureSensorPin A0
#define leftArrowKeyPin 9
#define rightArrowKeyPin 10
#define enterKeyPin 8

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
#define UPPERPRESSURE_PARAM_MENU_CHANGE 7
#define UPPERPRESSURE_PARAM_MENU_CHANGE 8
#define TEST_TIME_MENU_CHANGE 9
#define SPEED_MENU_CHANGE 10

#define NO_DEVICE_SELECTED 0
#define IN_TEST_MENU 0
#define UNDERPRESSURE_TEST 0
#define OVERPRESSURE_TEST 1
#define DEFAULT_UPPERPRESSURE 400								// default upperpressure after program start - in daPa
#define DEFAULT_LOWERPRESSURE -400								// default lowerpressure after program start - in daPa
#define DEFAULT_TEST_TIME 10												// default test time after program start - in seconds
#define DISPLAY_DELAY_DURING_PUMPING_TEST 500 //cycles
#define DISPLAY_DELAY_DURING_LEAKING_TEST 1500 //cycles
#define MANOMETER_OFFSET_VALUE 501 // manometer offset value - used for calibration
#define PRESSURE_DEVIATION_BEFORE_TEST 80 //dPa
#define DEFAULT_PRESSURE_DEVIATION_LIMIT 10 //dPa

int delayTime;
int upperpressureValue = DEFAULT_UPPERPRESSURE;
int lowerpressureValue = DEFAULT_LOWERPRESSURE;
int rpm = DEFAULT_PUMP_SPEED;
int testTime = DEFAULT_TEST_TIME;
int menuLayer = 0;																	// divide the menu into 2 layers
double maxPressureDeviation[2] = {0, 0};



bool state;

int i;
int returnKeyDetect;
int readButtonState();
bool upperpressureStatus = true;
bool lowerpressureStatus = true;
void dispalyTestBeginingCommunicate();
void displayPressureMeasurement(double calculatedPressure);
void displayPressureDeviationMeasurement(double pressureDeviation);
void displayVentingCommunicate();
void displayTestResult(double pressureDeviationLimit);
void displayTestTime (int testTime, int testTimeTmp);
void displayPressureError();
void pressureError();
void displayBlockedError();
void blockedError();
void showMenu(int currentMenu_tmp);
void DoTheTest(int DUT);
double readPressure();
int option[2] = {BEGIN_TEST, BEGIN_TEST}
int DUT = NO_DEVICE_SELECTED;
int currentMenu = 1;
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup()
{
	Serial.begin(9600);
	// set up pump controller:
	pinMode(sleepPin, OUTPUT);
	pinMode(pumpControlPin OUTPUT);
	pinMode(directionSignalPin, OUTPUT);
	digitalWrite(sleepPin, sleepOn);
	digitalWrite(pumpControlPin, stopPump);
	pinMode(directionSignalPin, forward);		//set dafault direction 
	
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
	showMenu(START_MENU);
}

void loop()
{
  option[0] = option[1];
  option[1] = readButtonState();
  if (option[0] != option[1])
  {
    switch (option[1])
    {
      case LEFT:
        DUT = NO_DEVICE_SELECTED;
        currentMenu = AIRFIX;
        showMenu(AIRFIX);
       
        break;
      case RIGHT:
        DUT = NO_DEVICE_SELECTED;
        currentMenu = AQUASTIM;
        showMenu(AQUASTIM);
        break;
      case ENTER:
        if (option[0] == IN_TEST_MENU & DUT != NO_DEVICE_SELECTED & option[1] == ENTER)
        {
          DoTheTest(DUT);
          option[1] = AIRFIX;
          DUT = NO_DEVICE_SELECTED;
          showMenu(AIRFIX);
        } else if (DUT == NO_DEVICE_SELECTED)
        {
          currentMenu = TEST_MENU;
          showMenu(TEST_MENU);
          delay(300); // delay to avoid double click
        }
        break;
      default:;
    }
  }
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
	} else if (option[0] == ENTER & digitalRead(enterKeyPin) == HIGH)
	{
		option[0] = IN_TEST_MENU;
		return IN_TEST_MENU;
	} else
	{
		return option[0];
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
		lcd.print(upperpressureValue);
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
		lcd.print(lowerpressure);
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
		lcd.print(testTime);
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
		lcd.print(rpm);
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
		lcd.print(upperpressureValue);
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
		
	case LOWERRPRESSURE_PARAM_MENU_CHANGE:
		lcd.clear();
		lcd.setCursor(4, 0);
		lcd.print("<<PODCISNIENIE>>");
		lcd.setCursor(0, 1);
		lcd.print("Target: ");
		lcd.setCursor(8, 1);
		lcd.print(lowerpressureValue);
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
		lcd.print(testTime);
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
		lcd.print(rpm);
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

    default:;
	}
}

void DoTheTest(int DUT)
{
  unsigned long startTime;
  unsigned long pumpingTime = 1;
  double refPressure;
  double pressureDeviation = 0;
  int testTime = 0; 
  int lowerPressure;
  int upperPressure;
  int testTimeTmp;
  int pressureDeviationLimit;
  returnKeyDetect = 0;
  i = 0;
  if (DUT == AIRFIX)
  {
    lowerPressure = LOWERPRESSURE_AIRFIX;
    upperPressure = UPPERPRESSURE_AIRFIX;
    testTimeTmp = TEST_TIME_AIRFIX;
    pressureDeviationLimit = PRESSURE_DEVIATION_LIMIT_AIRFIX;
  } else if (DUT == AQUASTIM)
  {
    lowerPressure = LOWERPRESSURE_AQUASTIM;
    upperPressure = UPPERPRESSURE_AQUASTIM;
    testTimeTmp = TEST_TIME_AQUASTIM;
    pressureDeviationLimit = PRESSURE_DEVIATION_LIMIT_AQUASTIM;
  }

  dispalyTestBeginingCommunicate();

  // Underpressure test begin - currently not used

  /*
  
  digitalWrite(valve3Pin, valveOn);
  digitalWrite(sleepPin, sleepOff);
  digitalWrite(directionSignalPin, backward);
  startTime = ( millis() / 1000 );
  
  calculatedPressure = readPressure();
  displayPressureMeasurement(calculatedPressure);
  while ((calculatedPressure = readPressure()) > lowerPressure && ( ( pumpingTime = ((millis() / 1000) - startTime))  < PUMPING_MAX_TIME ))
  {
    do_the_step(delayTime);
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
  if ( pumpingTime >= PUMPING_MAX_TIME || pumpingTime <= PUMPING_MIN_TIME)
  {
    if(pumpingTime <= PUMPING_MIN_TIME)
    {
      blockedError();
    }else
    {
      pressureError();
    }
  }else
  {
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
      while ( ( testTime = ((millis() / 1000) - startTime) ) < testTimeTmp)
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
      digitalWrite(valve3Pin, valveOff);
      displayVentingCommunicate();
  
    // Underpressure test end
    
    */
    
    // Overpressure test begin

    startTime = 0;
    i = 0;
    pumpingTime = 0;
    refPressure = 0;
    pressureDeviation = 0;
    testTime = 0;
    lcd.clear();
    digitalWrite(directionSignalPin, forward);
    digitalWrite(sleepPin, sleepOff);
    digitalWrite(valve3Pin, valveOn);
    startTime = ( millis() / 1000 );
    
      calculatedPressure = readPressure();
      displayPressureMeasurement(calculatedPressure);
       
    while ((calculatedPressure = readPressure()) < upperPressure && ( ( pumpingTime = ((millis() / 1000) - startTime))  < PUMPING_MAX_TIME ))
    {
      do_the_step(delayTime);
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
    if ( pumpingTime >= PUMPING_MAX_TIME || pumpingTime <= PUMPING_MIN_TIME)
    {
      if(pumpingTime <= PUMPING_MIN_TIME)
    {
      blockedError();
    }else
    {
      pressureError();
    }
    }else
    {
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
      if( ( calculatedPressure - refPressure ) > PRESSURE_DEVIATION_BEFORE_TEST)
      {
        pressureError();
      }else
      {
        while ( ( testTime = ((millis() / 1000) - startTime) ) < testTimeTmp)
        {
          calculatedPressure = readPressure();
          pressureDeviation = refPressure - abs(calculatedPressure);
         // if (pressureDeviation > maxPressureDeviation[OVERPRESSURE_TEST])
        // {
           maxPressureDeviation[OVERPRESSURE_TEST] = pressureDeviation;
        //  }
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
        delay(500);
        displayTestResult(pressureDeviationLimit);
        returnKeyDetect = readButtonState();
        while(returnKeyDetect != ENTER)
        {
          returnKeyDetect = readButtonState();
        }
          while(returnKeyDetect == ENTER)
        {
          returnKeyDetect = readButtonState(); 
        }
      // Overpressure test end
        }
      }
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

void displayTestResult(double pressureDeviationLimit)
{
  lcd.clear();
  lcd.setCursor(5, 0);
  if (abs( maxPressureDeviation[UNDERPRESSURE_TEST] ) > pressureDeviationLimit  || abs( maxPressureDeviation[OVERPRESSURE_TEST] ) > pressureDeviationLimit )
  {
    lcd.print("<< FAIL >>");
    lcd.setCursor(0, 1);
    lcd.print("Ulot przy:");
    lcd.setCursor(0, 2);
    lcd.print("-nadcisnieniu:");
    lcd.setCursor(14, 2);
    lcd.print( abs( maxPressureDeviation[OVERPRESSURE_TEST] ));
  }else
  {
    lcd.print("<< PASS >>");
    lcd.setCursor(0, 2);
    lcd.print("Ulot ponizej ");
    lcd.setCursor(13, 2);
    lcd.print(pressureDeviationLimit);
    lcd.setCursor(15, 2);
    lcd.print("dPa");   
  }

  lcd.display();
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
  double calculatedPressure = -(pressureSensorValue - MANOMETER_OFFSET_VALUE) * 2.5641025641025641025641025641026;
  return (calculatedPressure);
}

void pressureError()
{
  digitalWrite(valve3Pin, valveOff);
  displayPressureError();
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

void blockedError()
{
  digitalWrite(valve3Pin, valveOff);
  displayBlockedError();
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

//Pump control code
void do_the_step (int delayTime_tmp)
{
  digitalWrite(stepSignalPin, LOW);
  delayMicroseconds(1000);
  digitalWrite(stepSignalPin, HIGH);
  delayMicroseconds(1000);
}
	
void handleMenu()
{ 
option[0] = option[1];					// store current and 
option[1] = readButtonState();		// previous button state
if (option[0] != option[1])				// do something if button state has been changed from previous state
 {
	 switch (option[1])
	 {
		 if(menuLayer == 0)
		 {
			case LEFT:
				//DUT = NO_DEVICE_SELECTED;
				if(currentMenu != TEST_BEGIN)
				{
					currentMenu--;
					showMenu(currentMenu);
				}
				break;
			case RIGHT:
				//DUT = NO_DEVICE_SELECTED;
				if(currentMenu != START_IN_MANUAL_MODE_MENU)
				{
					currentMenu++;
					showMenu(currentMenu);
				}
				break;
			case ENTER:
				if(currentMenu != START_MENU && currentMenu != START_IN_MANUAL_MODE_MENU)
				{
					currentMenu+=10;
					menuLayer++;
					showMenu(currentMenu);
				}else if (currentMenu == START_MENU)
				{
					showMenu(TEST_BEGIN);
					//do the test and return to menu
				}else if (currentMenu == START_IN_MANUAL_MODE_MENU)
				{
					showMenu(TEST_BEGIN);
					//do the test and return to menu
				}
				break;
			default:;
		 }else
		 {
			 
			 
			 
		 }
		 
		 
		}
	}
}