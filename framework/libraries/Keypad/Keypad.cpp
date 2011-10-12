/* $Id: Keypad.cpp 1198 2011-06-14 21:08:27Z bhagman $
||
|| @author         Alexander Brevig <abrevig@wiring.org.co>
|| @url            http://wiring.org.co/
|| @url            http://alexanderbrevig.com/
|| @contribution   Mark Stanley
|| @contribution   Brett Hagman <bhagman@wiring.org.co>
||
|| @description
|| | This is a Hardware Abstraction Library for matrix Keypads.
|| | This library provides a simple interface for using matrix
|| | keypads. It supports the use of multiple keypads with the
|| | same or different sets of keys.  It also supports user
|| | selectable pins and definable keymaps.
|| |
|| | Wiring Cross-platform Library
|| #
||
|| @license Please see cores/Common/License.txt.
||
*/

#include "Keypad.h"

/*
|| @constructor
|| | Initialize the Keypad
|| #
||
|| @parameter userKeymap  a user specified key map of the keypad
|| @parameter row         an array of pins that are connected to the row of the keymap
|| @parameter col         an array of pins that are connected to the coloumn of the keymap
|| @parameter rows        the number of rows (the length og the row array)
|| @parameter cols        the number of coloumns (the length og the col array)
*/
Keypad::Keypad(char *userKeymap, byte *row, byte *col, byte rows, byte cols)
{
  rowPins = row;
  columnPins = col;

  size.rows = rows;
  size.columns = cols;

  begin(userKeymap);

  lastUpdate = 0;
  debounceTime = 50;
  holdTime = 1000;
  keypadEventListener = 0;
  currentKey = NO_KEY;
  state = IDLE;

  initializePins();
}

/*
|| @description
|| | Let the user define a keymap - assume the same row- / columncount as defined in constructor
|| #
*/
void Keypad::begin(char *userKeymap)
{
  keymap = userKeymap;
}

/*
|| @description
|| | Returns the keycode of the pressed key, or NO_KEY if no key is pressed
|| | The keycode is retrieved from the keymap array
|| #
*/
char Keypad::getKey()
{
  char key = NO_KEY; // Assume that no key is pressed, this is the default return for getKey()
  for (byte c = 0; c < size.columns; c++)
  {
    digitalWrite(columnPins[c], LOW);	// Activate the current column.
    for (byte r = 0; r < size.rows; r++) // Scan all the rows for a key press.
    {
      //  The user pressed a button for more then debounceTime microseconds.
      if (currentKey == keymap[c + (r * size.columns)])
      {
        // Button hold
        if (((millis() - lastUpdate) >= holdTime) && digitalRead(rowPins[r]) == LOW)
        {
          transitionTo(HOLD);
        }
        // Button release
        if (((millis() - lastUpdate) >= debounceTime) && digitalRead(rowPins[r]) == HIGH)
        {
          transitionTo(RELEASED);
          currentKey = NO_KEY;
        }
      }
      // Button pressed event.  The user pressed a button.
      else if (((millis() - lastUpdate) >= debounceTime) && digitalRead(rowPins[r]) == LOW)
      {
        digitalWrite(columnPins[c], HIGH);	// De-activate the current column.
        key = keymap[c + (r * size.columns)];
        lastUpdate = millis();
        goto EVALUATE_KEY; 			// Save resources and do not attempt to parse two keys at a time
      }
    }
    digitalWrite(columnPins[c], HIGH);	// De-activate the current column.
  }

EVALUATE_KEY:
  if (key != NO_KEY && key != currentKey)
  {
    currentKey = key;
    transitionTo(PRESSED);
    return currentKey;
  }
  else
  {
    return NO_KEY;
  }
}

/*
|| @description
|| | Get the state of this keypad
|| #
||
|| @return The current state of this keypad [ IDLE | PRESSED | RELEASED | HOLD ]
*/
KeypadState Keypad::getState()
{
  return state;
}

/*
|| @description
|| | Set the debounce time of this keypad
|| | If changes / presses occur within the debounce interval, they will be ignored
|| #
||
|| @parameter debounce This sets the debounce time for this keypad, value in ms
*/
void Keypad::setDebounceTime(unsigned int debounce)
{
  debounceTime = debounce;
}

/*
|| @description
|| | Set the hold time interval of this keypad
|| | If a key is pressed longer than interval, the hold event will fire
|| #
||
|| @parameter hold This sets the hold time for this keypad, value in ms
*/
void Keypad::setHoldTime(unsigned int hold)
{
  holdTime = hold;
}

/*
|| @description
|| | Add an event listener to this keypad
|| | The event listener function has to be of void type, and expect a char as the parameter
|| #
||
|| @parameter listener The void function that handles events, must accept a char as its parameter
*/
void Keypad::addEventListener(void (*listener)(char))
{
  keypadEventListener = listener;
}

/// private methods

void Keypad::transitionTo(KeypadState newState)
{
  if (state != newState)
  {
    state = newState;
    if (keypadEventListener != NULL)
    {
      keypadEventListener(currentKey);
    }
  }
}

void Keypad::initializePins()
{
  for (byte r = 0; r < size.rows; r++)
  {
    for (byte c = 0; c < size.columns; c++)
    {
      pinMode(columnPins[c], OUTPUT);
      digitalWrite(columnPins[c], HIGH);
    }
    //configure row pin modes and states
    pinMode(rowPins[r], INPUT);
    digitalWrite(rowPins[r], HIGH);
  }
}

