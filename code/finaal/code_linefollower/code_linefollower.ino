#include "SerialCommand.h"
#include "EEPROMAnything.h"

#define SerialPort Serial
#define Baudrate 115200

#define enA 10
#define MotorLeftFoward 5
#define MotorLeftBackward 4
#define MotorRightFoward 6
#define MotorRightBackward 7
#define enB 9

SerialCommand sCmd(SerialPort);
bool debug;
bool run;
unsigned long previous, calculationTime;

const int sensor[] = {A5, A4, A3, A2, A1, A0};
float position;
struct param_t
{
  unsigned long cycleTime;
  int black[6];
  int white[6];
  int power;
  float diff;
  float kp;
  /* andere parameters die in het eeprom geheugen moeten opgeslagen worden voeg je hier toe ... */
} params;

int normalised[6];
float debugPosition;

void onUnknownCommand(char *command);
void onSet();
void onDebug();
void onCalibrate();
void onRun();
void onStop();

void setup()
{
  pinMode(MotorLeftFoward, OUTPUT);
  pinMode(MotorLeftBackward, OUTPUT);
  pinMode(MotorRightBackward, OUTPUT);
  pinMode(MotorRightFoward, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  digitalWrite(MotorLeftFoward, LOW);
  digitalWrite(MotorLeftBackward, LOW);
  digitalWrite(MotorRightBackward, LOW);
  digitalWrite(MotorRightFoward, LOW);
  digitalWrite(enA,HIGH);
  digitalWrite(enB,HIGH);
  
  SerialPort.begin(Baudrate);

  sCmd.addCommand("set", onSet);
  sCmd.addCommand("debug", onDebug);
  sCmd.addCommand("calibrate", onCalibrate);
  sCmd.addCommand("run", onRun);
  sCmd.addCommand("stop", onStop);
  sCmd.setDefaultHandler(onUnknownCommand);

  EEPROM_readAnything(0, params);

  SerialPort.println("ready");
}

void loop()
{
  sCmd.readSerial();

  unsigned long current = micros();
  if (current - previous >= params.cycleTime)
  {
    previous = current;

    // measure & normalize
    for (int i = 0; i < 6; i++)
    {
      normalised[i] = map(analogRead(sensor[i]), params.black[i], params.white[i], 0, 1000);
    }

    // interpolate
    float position = 0;
    int index = 0;
    for (int i = 1; i < 6; i++) if (normalised[i] < normalised[index]) index = i;

    if (normalised[index] > 3000) run = false;

    if (index == 0) position = -30;
    else if (index == 5) position = 30;
    else
    {
      int sNul = normalised[index];
      int sMinEen = normalised[index-1];
      int sPlusEen = normalised[index+1];

      float b = sPlusEen - sMinEen;
      b = b / 2;

      float a = sPlusEen - b - sNul;

      position = -b / (2 * a);
      position += index;
      position -= 2.5;

      position *= 15;
      }
      //debugPosition = position;
      float error = -position;
      float output = params.kp * error;

      output = constrain(output, -510,510);

      int powerLeft = 0;
      int powerRight = 0;

      if (run) if (output >=0)
      {
        powerLeft = constrain(params.power + params.diff * output, -255, 255);
        powerRight = constrain(powerLeft - output, -255, 255);
        powerLeft = powerRight + output;
      }
      else
      {
        powerRight = constrain(params.power - params.diff * output, -255, 255);
        powerLeft = constrain(powerRight + output, -255, 255);
        powerRight = powerLeft - output;
      }

      analogWrite(MotorLeftFoward, powerLeft > 0 ? powerLeft : 0);
      analogWrite(MotorLeftBackward, powerLeft < 0 ? -powerLeft : 0);
      analogWrite(MotorRightFoward, powerRight > 0 ? powerRight : 0);
      analogWrite(MotorRightBackward, powerRight < 0 ? -powerRight : 0);

  }

  unsigned long difference = micros() - current;
  if (difference > calculationTime) calculationTime = difference;
}

void onUnknownCommand(char *command)
{
  SerialPort.print("unknown command: \"");
  SerialPort.print(command);
  SerialPort.println("\"");
}

void onSet()
{
  char* param = sCmd.next();
  char* value = sCmd.next();  
  
  if (strcmp(param, "cycle") == 0) params.cycleTime = atol(value);
  if (strcmp(param, "power") == 0) params.power = atol(value);
  if (strcmp(param, "diff") == 0) params.diff = atof(value);
  if (strcmp(param, "kp") == 0) params.kp = atof(value);
  
  /* parameters een nieuwe waarde geven via het set commando doe je hier ... */
  
  EEPROM_writeAnything(0, params);
}

void onDebug()
{
  SerialPort.print("cycle time: ");
  SerialPort.println(params.cycleTime);
  
  /* parameters weergeven met behulp van het debug commando doe je hier ... */
  SerialPort.print("black: ");
  for (int i = 0; i< 6; i++)
  {
    SerialPort.print(params.black[i]);
    SerialPort.print(" ");
  }

  SerialPort.print("white: ");
  for (int i = 0; i< 6; i++)
  {
    SerialPort.print(params.white[i]);
    SerialPort.print(" ");
  }
  
  SerialPort.print("normalised: ");
  for (int i = 0; i < 6; i++)
  {
    SerialPort.print(normalised[i]);
    SerialPort.print(" ");
  }

  SerialPort.print("position: ");
  SerialPort.println(debugPosition);

  SerialPort.print("power: ");
  SerialPort.println(params.power);
  SerialPort.print("diff: ");
  SerialPort.println(params.diff);
  SerialPort.print("kp: ");
  SerialPort.println(params.kp);
  
  SerialPort.print("calculation time: ");
  SerialPort.println(calculationTime);
  calculationTime = 0;
}

void onCalibrate()
{
  char* param = sCmd.next();

  if (strcmp(param, "black") == 0)
  {
    SerialPort.print("start calibrating black... ");
    for (int i = 0; i < 6; i++) params.black[i]=analogRead(sensor[i]);
    SerialPort.println("done");
  }
  else if (strcmp(param, "white") == 0)
  {
    SerialPort.print("start calibrating white... ");    
    for (int i = 0; i < 6; i++) params.white[i]=analogRead(sensor[i]);  
    SerialPort.println("done");      
  }

  EEPROM_writeAnything(0, params);
}

void onRun()
{
  run = true;
}

void onStop()
{
  run = false;
}
