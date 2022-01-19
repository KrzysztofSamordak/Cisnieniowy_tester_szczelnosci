#include <LiquidCrystal.h>
aa
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
#define UPPERPRESSURE_PARAM_MENU_CHANGE 12
#define LOWERPRESSURE_PARAM_MENU_CHANGE 13
#define TEST_TIME_MENU_CHANGE 14
#define SPEED_MENU_CHANGE 15
#define DO_NOTHING 0

#define NO_DEVICE_SELECTED 0
#define IN_TEST_MENU 0
#define UNDERPRESSURE_TEST 0
#define OVERPRESSURE_TEST 1
#define BOTH_TESTS 2

//config
#define DEFAULT_UPPERPRESSURE 400								// default upperpressure after program start - in daPa
#define DEFAULT_LOWERPRESSURE -400								// default lowerpressure after program start - in daPa
#define DEFAULT_TEST_TIME 10									// default test time after program start - in seconds
#define DISPLAY_DELAY_DURING_PUMPING_TEST 500							//cycles
#define DISPLAY_DELAY_DURING_LEAKING_TEST 1500 						//cycles
#define MANOMETER_OFFSET_VALUE 501 								// manometer offset value - used for calibration
#define PRESSURE_DEVIATION_BEFORE_TEST 80 							//dPa
#define DEFAULT_PRESSURE_DEVIATION_LIMIT 10 							//dPa
#define MAX_LOWERPRESSURE -1000								// maximal lowerpressure that can be set
#define MAX_UPPERPRESSURE 1000								// maximal upperpressure that can be set


int delayTime;
int param[4];											//store parameters' values for specyfied tests
param[0] = DEFAULT_UPPERPRESSURE;
param[1] = DEFAULT_LOWERPRESSURE;
param[2] = DEFAULT_TEST_TIME;
param[3] = DEFAULT_PUMP_SPEED;	
int add_value[4];										//store the value representing one point of param		
add_value[0] = 10;										//adding 1 value point to UPPERPRESSURE param will add 10 dPa
add_value[0] = 10;										//adding 1 value point to LOWERPRESSURE param will add 10 dPa
add_value[0] = 1;										//adding 1 value point to TEST_TIME param will add 1s
add_value[0] = 1;										//adding 1 value point to PUMP_SPEED param will add 1 speed level
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
void displayTestResult(int testsNumber);
void displayTestTime (int testTime, int testTimeTmp);
void displayPressureError();
void pressureError();
void displayBlockedError();
void blockedError();
void showMenu(int currentMenu_tmp);
void DoTheUnderpressureTest();
void DoTheOverpressureeTest();
double readPressure();
double maxPressureDeviation[2] = {0, 0};
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

void DoTheUnderpressureTest()
{
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
  while ((calculatedPressure = readPressure()) > lowerPressure && (digitalRead(enterKeyPin) != LOW)
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
  /*if ( pumpingTime >= PUMPING_MAX_TIME || pumpingTime <= PUMPING_MIN_TIME)
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
  */
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
      while ( (( testTime = ((millis() / 1000) - startTime) ) < testTimeTmp) && (digitalRead(enterKeyPin) != LOW)
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
      
      
  
void DoTheOverpressureeTest()
{
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
    /* if ( pumpingTime >= PUMPING_MAX_TIME || pumpingTime <= PUMPING_MIN_TIME)
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
    */
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
  double calculatedPressure = -(pressureSensorValue - MANOMETER_OFFSET_VALUE) * 2.5641025641025641025641025641026;					// entered value is the K factor (should also be calibrated)
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
int testsNumber = 0;
int option = readButtonState();				// store button state
if (option != DO_NOTHING)				
 {
	if(currentMenu < 10)							//first menu layer
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
				if(currentMenu != START_IN_MANUAL_MODE_MENU)
				{
					currentMenu++;
					showMenu(currentMenu);
				}
				break;
			case ENTER:
				if(currentMenu != START_MENU && currentMenu != START_IN_MANUAL_MODE_MENU)			// go to second menu layer
				{
					currentMenu+=10;
					showMenu(currentMenu);
				}else if (currentMenu == START_MENU)																	// automatic test handle
				{
					showMenu(TEST_BEGIN);
					if(lowerpressureStatus)																							// check if lowerPressure test should be done
					{
						DoTheUnderpressureTest();																				// do the lowerpressure test
						testsNumber = UNDERPRESSURE_TEST;														//save that only lowerpressure test result should be viewed
					}
					if(upperpressureStatus)																							// check if upperPressure test should be done														
					{
						DoTheOverpressureTest();																					//do the upperPressure test
						if(lowerpressureStatus)																						// if lowerpressure test have been done
						{
							testsNumber = BOTH_TESTS;																		// save that both tests results should be viewed
						}else
						{
						testsNumber = OVERPRESSURE_TEST;															// else save that only upperPressure test result should be viewed
						}
					}
					displayTestResult(testsNumber);																			// view test results and wait for clicking ENTER to accept it
					currentMenu = START_MENU;																				// after displaying test results go back to main menu
					showMenu(currentMenu);
				}else if (currentMenu == START_IN_MANUAL_MODE_MENU)									// manualc test handle
				{
					showMenu(TEST_BEGIN);
					while( digitalRead(leftArrowKeyPin) != buttonPressed && digitalRead(enterKeyPin) != buttonPressed)		//need to set up loop to not exit switch case
					{
						switch(readButtonState())																						//check which button have been clicked
						{
							case LEFT:
								digitalWrite(directionSignalPin, backward);														// if left arrow has been clicked, set direction signal pin to backward
								digitalWrite(pumpControlPin, runPump);															// run pump backward
								while(digitalRead(leftArrowKeyPin) == buttonPressed )									// checking if button is still pressed 
								{																														// if it is, just do nothing here (pump will be running because of setting pumpControlPin to runPump)
								}
								digitalWrite(pumpControlPin, stopPump);														// if button has been released just turn off pump
								break;
							case RIGHT:
								digitalWrite(directionSignalPin, forward);														// if right arrow has been clicked, set direction signal pin to backward
								digitalWrite(pumpControlPin, runPump);															// run pump backward
								while(digitalRead(rightArrowKeyPin) == buttonPressed )								// checking if button is still pressed 
								{																														// if it is, just do nothing here (pump will be running because of setting pumpControlPin to runPump)
								}
								digitalWrite(pumpControlPin, stopPump);														// if button has been released just turn off pump
								break;
							case ENTER:																										// if enter button has been pressed just change valve state
								if(digitalRead(valve3Pin) == valveOff)
								{
									digitalWrite(valve3Pin, valveOn)
								}else
								{
									digitalWrite(valve3Pin, valveOff)
								}
								break;
						}
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
					param[currentMenu - 12] -= add_value[currentMenu - 12];			//subtract one value point from the specyfied parameter
					showMenu(currentMenu);
				}
				break;
			case RIGHT:
				if(param[currentMenu - 12] < MAX_UPERPRESSURE)
				{
					param[currentMenu - 12] += add_value[currentMenu + 12];			//add one value point from the specyfied parameter
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


