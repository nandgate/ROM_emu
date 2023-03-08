uint16_t addr;
uint16_t user;

#define B0      2
#define B1      3
#define B2      4
#define B3      5
#define B4      6
#define B5      7
#define B6      8
#define B7      9
#define P_WE    10
#define P_OE    11
#define P_CS    12
#define MUX     14
#define ADDRH   15
#define ADDRL   16
#define DIR     17
#define T_RESET 18
#define T_EN    19
#define PWR     7

void Init_HW(void) {
  pinMode(MUX, OUTPUT);     digitalWrite(MUX, HIGH);
  pinMode(P_WE, OUTPUT);    digitalWrite(P_WE, HIGH);
  pinMode(P_OE, OUTPUT);    digitalWrite(P_OE, HIGH);
  pinMode(P_CS, OUTPUT);    digitalWrite(P_CS, HIGH);
  pinMode(ADDRH, OUTPUT);   digitalWrite(ADDRH, HIGH);
  pinMode(ADDRL, OUTPUT);   digitalWrite(ADDRL, HIGH);
  pinMode(DIR, OUTPUT);     digitalWrite(DIR, LOW);
  pinMode(T_RESET, OUTPUT); digitalWrite(T_RESET, HIGH);
  pinMode(T_EN, OUTPUT);    digitalWrite(T_EN, HIGH);

}

uint8_t BusIn(void) {
  uint8_t value = 0;

  pinMode(B0, INPUT);
  pinMode(B1, INPUT);
  pinMode(B2, INPUT);
  pinMode(B3, INPUT);
  pinMode(B4, INPUT);
  pinMode(B5, INPUT);
  pinMode(B6, INPUT);
  pinMode(B7, INPUT);

  value |= (digitalRead(B0) == HIGH) ? 0x01 : 0;
  value |= (digitalRead(B1) == HIGH) ? 0x02 : 0;
  value |= (digitalRead(B2) == HIGH) ? 0x04 : 0;
  value |= (digitalRead(B3) == HIGH) ? 0x08 : 0;
  value |= (digitalRead(B4) == HIGH) ? 0x10 : 0;
  value |= (digitalRead(B5) == HIGH) ? 0x20 : 0;
  value |= (digitalRead(B6) == HIGH) ? 0x40 : 0;
  value |= (digitalRead(B7) == HIGH) ? 0x80 : 0;

  return value;
}

void BusOut(uint8_t value) {
  pinMode(B0, OUTPUT);
  pinMode(B1, OUTPUT);
  pinMode(B2, OUTPUT);
  pinMode(B3, OUTPUT);
  pinMode(B4, OUTPUT);
  pinMode(B5, OUTPUT);
  pinMode(B6, OUTPUT);
  pinMode(B7, OUTPUT);

  digitalWrite(B0, (value & 0x01) ? HIGH : LOW);
  digitalWrite(B1, (value & 0x02) ? HIGH : LOW);
  digitalWrite(B2, (value & 0x04) ? HIGH : LOW);
  digitalWrite(B3, (value & 0x08) ? HIGH : LOW);
  digitalWrite(B4, (value & 0x10) ? HIGH : LOW);
  digitalWrite(B5, (value & 0x20) ? HIGH : LOW);
  digitalWrite(B6, (value & 0x40) ? HIGH : LOW);
  digitalWrite(B7, (value & 0x80) ? HIGH : LOW);
}

void Strobe(uint8_t pin) {
  digitalWrite(pin, LOW);
  digitalWrite(pin, HIGH);
}

uint8_t Read(uint16_t address) {
  uint8_t data;

  Serial.print("RD");
  Serial.println(address, HEX);

  BusOut(addr >> 8);
  Strobe(ADDRH);
  BusOut(addr & 0xFF);
  Strobe(ADDRL);

  digitalWrite(DIR, LOW);
  digitalWrite(P_CS, LOW);
  digitalWrite(P_OE, LOW);
  data= BusIn();
  digitalWrite(P_OE, HIGH);
  digitalWrite(P_CS, HIGH);
  
  return data;
}

void Write(uint8_t address, uint8_t data) {
  Serial.print("WR");
  Serial.println(address, HEX);

  BusOut(addr >> 8);
  Strobe(ADDRH);
  BusOut(addr & 0xFF);
  Strobe(ADDRL);

  digitalWrite(DIR, HIGH);
  digitalWrite(P_CS, LOW);
  digitalWrite(P_WE, LOW);
  BusOut(data);
  digitalWrite(P_WE, HIGH);
  digitalWrite(P_CS, HIGH);
  digitalWrite(DIR, LOW);
}

void ShowInfo(void) {
  Serial.write("\nRom_emu,0.1\n");
}

void HexDigit(int ch) {
  user <<= 4;

  if (isDigit(ch)) {
    user |= (ch-'0');
  }
  else {
    ch = toLowerCase(ch);
    user |= ch-'a' + 10;
  }
}

bool DoCmd(int cmd) {
  if(isHexadecimalDigit(cmd)) {
    HexDigit(cmd);
    return false;
  } 
  else switch(cmd) {
    case '\r':
      return false;;
    case '?':
      ShowInfo();
      break;
    case '\n':
      Write(addr++, user);
      break;      
    case '+':
      addr++;
      break;
    case '-':
      addr--;
      break;
    case ' ':
      addr = user;
      break;
  }
  return true;
}

void prompt(void) {
  Serial.print('\n');
  Serial.print(addr, HEX);
  Serial.print(": ");
  Serial.print(Read(addr), HEX);
  Serial.write('>');
}

void setup() {
  Serial.begin(19200);
  Init_HW();
  pinMode(LED_BUILTIN, OUTPUT);
  ShowInfo();
  addr = 0;
  user = 0;
  prompt();
}

void loop() {
  if(Serial.available() > 0) {
    int ch = Serial.read();
    if(DoCmd(ch)) {
      prompt();
    }
  }
}
