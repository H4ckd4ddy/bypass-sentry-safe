static const int TX = 6;

void packet(int cmd, int a, int b, int c, int d, int e) {
  int cs = (cmd + a + b + c + d + e);
  
  pinMode(TX, OUTPUT);
  digitalWrite(TX, LOW);
  delayMicroseconds(2750);
  digitalWrite(TX, HIGH);
  delayMicroseconds(200);
  Serial.begin(4800);

  Serial.write((byte) 0x0);
  Serial.write(cmd);
  Serial.write(a);
  Serial.write(b);
  Serial.write(c);
  Serial.write(d);
  Serial.write(e);
  Serial.write(cs);
  
  Serial.end();
  pinMode(TX, OUTPUT);
  digitalWrite(TX, HIGH);
}

void set_comb(int a, int b, int c, int d, int e) {
  packet(0x75, a, b, c, d, e);
}

void input_comb(int a, int b, int c, int d, int e) {
  packet(0x71, a, b, c, d, e);
}

void setup() {
  set_comb(5,3,2,1,0);
  delay(1000);
  input_comb(5,3,2,1,0);
}

void loop() {}
