#include "LedControl.h"

// Define the number of MAX7219 modules you have connected
#define MAX_DEVICES 8

// Define the data pin connections for LedControl
#define DIN_PIN  12
#define CS_PIN   10
#define CLK_PIN  11

// Define the data pin connections for the meter
#define METER_PIN  9

// Define the data pin for the buttons
#define RESET_PIN  2
#define UP_PIN     3
#define DOWN_PIN   4

// Define LED device indexes
#define LED_COUNT  3
#define LED_TOTAL_PRICE  0
#define LED_TOTAL_LITERS 1
#define LED_PRICE_LITER  2

void printNumber(unsigned long number, byte device);
void floatToDecimalNumber(float number, byte device);

// Create a new controler
LedControl lc=LedControl(DIN_PIN,CLK_PIN,CS_PIN,MAX_DEVICES);

void setup() {
	Serial.begin(9600);
	Serial.println("Hello World!");

	initLedMatrix();
}

void loop() {
	floatToDecimalNumber(123.45, 1);
	delay(1000);
}

void initLedMatrix() {
	for (int device = 0; device < LED_COUNT; device++) {
		lc.shutdown(device, false);
		// Set the brightness to a medium values
		lc.setIntensity(device, 15);
		// and clear the display
		lc.clearDisplay(device);
	}
}

// Reference: https://curiousscientist.tech/blog/max7219-8digit-display
void printNumber(unsigned long number, byte device=0) {
	int numberOfDigits = 0;

	unsigned long tmp_number = number;

	while (number)
	{
		number /= 10;
		numberOfDigits++;
		//same exercise as in the float processing part
	}

	int tmp_numberOfDigits = 0; //copy of the numberOfDigits
	int displayDigits[numberOfDigits]; //array to store the individual digits

	tmp_numberOfDigits = numberOfDigits; //copy the number of digits

	while (numberOfDigits--) //filling up the array
	{
		displayDigits[numberOfDigits] = tmp_number % 10;
		tmp_number /= 10;
		//same exercise as in the float processing part
	}

	lc.clearDisplay(device);

	for (int i = 0; i < tmp_numberOfDigits; i++)
	{
		lc.setDigit(device, i, displayDigits[tmp_numberOfDigits - i - 1], false);
		//same exercise as in the float processing part
	}
}

// Reference: https://curiousscientist.tech/blog/max7219-8digit-display
void floatToDecimalNumber(float floatNumber, byte device=0)
{
  //Casting the float into long will cut off the decimal digits, we keep the integers (e.g.: 12587.74 becomes 12587)
  long integerpart = (long) floatNumber;

  //multiplying the float by 100, casting it to long, then taking the modulo 100 will give us 2 digits (10^2 = 100) after the decimal point
  //long decimalpart = ((long)(floatNumber * 100.00) % 100);

  long decimalpart = (long)(100 * (floatNumber - integerpart)); //e.g.: 100 * (12587.74-12587) = 100 * 0.74 = 74
  //Possible improvement: Sometimes the decimals of the floating point number are slightly different
  //For example, this is the floating point number: 87073.37, but the extracted decimal part is 36.
  //This can happen because I do not round the number. So 87073.37 can be 87073.3650212125... which is 87073.37 when it is rounded
  //But when I do the above operation, the two digits extracted are 3 and 6. For "more accuracy", float must be rounded first.

  //At this point we know the following things:
  //1.) we manually set the number of decimal digits to 2. it will be two digits whatever we do.
  //2.) we know the value of the integer part, but we don't (yet) explicitly know the number of digits (number can be 1245.4 but also 1.29...etc.)

  //Let's count the integer part digits
  int integerDigits = 0; //number of digits in the integer part
  long tmp_integerpart = integerpart; //temporary variable, copy of the original integer parts
  long tmp2_integerpart = integerpart; //temporary variable 2, copy of the original integer parts

  while (tmp_integerpart)
  {
    tmp_integerpart = tmp_integerpart / 10;
    integerDigits++;
    //What happens inside:
    //Original float number: 16807.34, integer part: 16807 -(/10)-> 1680 -(/10)-> 168 -(/10)-> 16 -(/10)-> 1
  }

  int digitsPosition = integerDigits + 2; //+2 comes from the 2 decimal digits

  //now we know the total number of digits - keep in mind that the max integer digits allowed is 6!
  //The next step is to fill up an array that stores all the digits.
  //The array is filled up in a same was as the display: decimal numbers will be 0th and 1st elements, we do this first
  long digits[digitsPosition]; //array with the needed size
  int decimalDigits = 2; //manually created number of digits
  long tmp_decimalpart = decimalpart; //copy the original decimal part value

  //I do this part "manually"
  digits[integerDigits + 1] = tmp_decimalpart % 10; //2nd decimal digit (e.g.: 634.23 -> 3)
  tmp_decimalpart /= 10;
  digits[integerDigits] = tmp_decimalpart % 10; //1st decimal digit (e.g.: 634.23 -> 2)

  //                                                                [4 3 2 1 0]
  //Example: 634.23 -> Int: 634, Dec: 23. Digits: 5. Int.Digits: 3. [3 2 4 3 6]  <----- reading direction

  while (integerDigits--)
  {
    digits[integerDigits] = tmp2_integerpart % 10;
    tmp2_integerpart /= 10;
    //Same exercise as above with the decimal part, but I put it in a loop because we have more digits
  }

  //Finally, we print the digits on the display

  lc.clearDisplay(device);

  for (int j = 0; j < digitsPosition; j++)
  {
    if (j == 2)
    {
      lc.setDigit(device, j, digits[digitsPosition - j - 1], true); //we know that we have to put a decimal point here (last argument is "true")
      //0: address, j: position, value, decimal point.
      //Additional -1 is because we count the digits as first, second, third...etc, but the array starts from 0.
    }
    else
    {
      lc.setDigit(device, j, digits[digitsPosition - j - 1], false);
      //Example: number is 634.23
      //j = 0: digits[5-0-1] = digits[4] -> 0th element of the display gets the last element of the array which is the second decimal:   3 (hundreths)
      //j = 1: digits[5-1-1] = digits[3] -> 1th element of the display gets the last-1 element of the array which is the first decimal:  2 (tenths)
      //j = 2: digits[5-2-1] = digits[2] -> 2nd element of the display gets the last-2 element of the array which is the first integer:  4 (ones)
      //j = 3: digits[5-3-1] = digits[1] -> 3rd element of the display gets the last-3 element of the array which is the second integer: 3 (tens)
      //j = 4: digits[5-4-1] = digits[0] -> 4th element of the display gets the last-4 element of the array which is the third integer:  6 (hundreds)
    }
  }
}