const int pin = 1;

void send_request(int command, int a, int b, int c, int d, int e) {
  int checksum = (command + a + b + c + d + e);
  
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(2750);
  digitalWrite(pin, HIGH);
  delayMicroseconds(200);
  Serial.begin(4800);

  Serial.write((byte)0x0);
  Serial.write(command);
  Serial.write(a);
  Serial.write(b);
  Serial.write(c);
  Serial.write(d);
  Serial.write(e);
  Serial.write(checksum);
  
  Serial.end();
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}

void reset_code(int a, int b, int c, int d, int e) {
  send_request(0x75, a, b, c, d, e);
}

void try_code(int a, int b, int c, int d, int e) {
  send_request(0x71, a, b, c, d, e);
}

void setup() {
  reset_code(1,2,3,4,5);
  delay(1000);
  try_code(1,2,3,4,5);
}

void loop() {}
