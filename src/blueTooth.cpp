
#include <bluetooth.h>
#include <stdarg.h>

SoftwareSerial BTSerial(RX_PIN, TX_PIN);
BTcmd BTcmd(BTSerial);

// inits
BlueTooth::BlueTooth()
{
  mBTserial = &BTSerial;
  mBTcmd = &BTcmd;
}

void BlueTooth::init()
{
  pinMode(KEY_PIN, OUTPUT);
  pinMode(SWITCH_PIN, OUTPUT);
  // pinMode(STATE_PIN, INPUT);

  start(false);
}

// Power management
void BlueTooth::start(const bool on)
{
  mON = on;
  // digitalWrite(KEY_PIN, on ? HIGH : LOW); // AT on/off, on shhas to be done before powering on HC-05
  digitalWrite(SWITCH_PIN, on ? HIGH : LOW); // HC-05 on/off
  Serial << (on ? "Bluetooth starting" : "Bluetooth stopped") << endl;

  if (on) {
    BTSerial.begin(38400);  // HC-05 default speed in AT command or BT_BAUD
    delay(300); // needed to let HC-05 booting
    // cmdAT("NAME?");
    // cmdAT("ADDR?");
    // cmdAT("VERSION?");
    // cmdAT("PSWD?");
    // cmdAT("UART?");
    // // cmdAT("AT+ORGL");
    // cmdAT("NAME=\"Esk8\"");
    // cmdAT("UART=%ld,0,0", BT_BAUD);
    //
    // digitalWrite(KEY_PIN, LOW);
    // cmdAT("RESET");

    BTSerial.begin(BT_BAUD);  // HC-05 default speed in AT command or BT_BAUD
  }
}

void BlueTooth::toggle()
{
  start(mON ? false : true);
}

void BlueTooth::cmdAT(const char *fmt, ...)
{
  static char cmd[80];
  static char ack[800];

  // build cmd
  va_list args;
  va_start (args, fmt);
  vsnprintf(cmd, sizeof(cmd), fmt, args);
  va_end (args);

  Serial << "AT cmd: " << cmd << endl;
  // send cmd & wait for the end of serial transmission
  BTSerial << "AT+" << cmd << "\r\n";
  BTSerial.flush();

  // read acknowledge
  int read = BTSerial.readBytesUntil('\n', ack, sizeof(ack)); // wait for ack, blocks till '\n' or timeout    //Serial.setTimeout(1000) -- default
  Serial << (read ? ack : "TIMEOUT!") << endl;
}
