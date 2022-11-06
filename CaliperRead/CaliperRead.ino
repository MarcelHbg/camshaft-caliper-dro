#define PIN_DATA 12
#define PIN_CLK 10

int clock = 0;
int lastClock = 0;
int inBit = 0;
int input = 0;
int cnt = 0;

unsigned long startTime = 0;
unsigned long lastTime = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_DATA, INPUT);
  pinMode(PIN_CLK, INPUT);
  Serial.begin(9600);
  Serial.println("Ready: ");
}

void loop() {
  // put your main code here, to run repeatedly:
  lastClock = clock;
  clock = digitalRead(PIN_CLK);

  if (lastClock == 1 && clock == 0) { // falling edge detected
    lastTime = startTime;
    startTime = millis();
    unsigned long duration = startTime - lastTime;

    inBit = digitalRead(PIN_DATA);

    input |= inBit << cnt; // set bit at position
    cnt++;

    if (cnt >= 24) {
      double result = (double)input / 100.0; // calculate float value
      Serial.println(result);
    }

    if (duration > 80) { // reset at start of bit sequenz
      input = 0;
      cnt = 0;
    }
    
  }
}