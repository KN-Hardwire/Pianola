#include <Arduino.h>
#include <stdio.h>

//Wejścia
#define SI 8 //dane wejściowe
#define MAX_SQUAREWAVE_PINS 10

//Wyjścia
#define SCK 18 //zegar rejstru przesowanegoo
#define RCK 3//zegar przepisujący na wyjścia
#define nSCLR 17 //czyszczenie rejstru
//Matryca LED
#define W1 4
#define W2 5
#define W3 6
#define W4 7
//#define nG 27 //na stałe podpiąłem pod gnd

const int inputs[] = {W1, W2, W3, W4};
const int outputs[] = {SI, SCK, RCK, nSCLR};
bool dane[] = {1, 0, 0, 0, 0, 0, 0, 0};
int lastTime = 0;
int currentTime = 0;
 
struct waveData{
  int pin;
  unsigned long last_time;
  bool state;
};

int squarewave(int pin_number, int frequency);

void setup() {
  // Initialization of pins:
  for (int p : outputs){
    pinMode(p, OUTPUT);
  }
  for (int p : inputs)
  {
    pinMode(p, INPUT);
  }
  //Initialization of serial
  Serial.begin(115200);
}

void loop() {
  digitalWrite(nSCLR, HIGH);
  squarewave(SCK,2000);
  static bool lastSckState = 0; // nie chcę nadpisywać wartości przy kolejnej pętli!
  static int bitIndex = 0;
  bool currentSckState = digitalRead(SCK); //Można też czytać piny ustawione jako wyjścia - cool

  if (currentSckState == HIGH && lastSckState == 0){
    digitalWrite(SI, dane[bitIndex]);
    bitIndex++;
    if (bitIndex >= 4){
      bitIndex = 0;
       //pRZY MATRYCY 4 NA 4 CHCĘ, ŻEBY PO 4 TAKTACH REJSTR SIĘ ZRESETOWAŁ

    }
    digitalWrite(RCK, LOW);
    digitalWrite(RCK, HIGH);
  }
  lastSckState = currentSckState;

  for (int i = 0; i < 4; i++){
    if (digitalRead(inputs[i]) == 1 && (millis() - lastTime) >= 250){
      Serial.println(bitIndex *4 + i + 1);
      lastTime = millis();
    }
  }
}

// put function definitions here:
// od razu się rzuca, że fajnie by byłozrobić funkcję delay i funkcję dla przebiegów wysyłających okresowo jakąś ramkę danych

int squarewave(int pin_number, int frequency) {
  static waveData waves[MAX_SQUAREWAVE_PINS];
  static int pins_registered = 0;

  waveData* currentWave = nullptr; // tworzenie pustego wskaźnika i określanie rozmiaru elementu w tym segmencie pamięci (pod nasz tym struct)
  // było to ważne również, żeby procecsor wiedział jak zinterpretować dane pod tym adresem

  //Wyszukiwanie pinu w liście zarejstrowanych już wcześniej
  for (int i = 0; i < pins_registered; i++){
    if (waves[i].pin == pin_number){ // w roli przypomnienia .pin to dopisek co z structa w tym momencie bierzemy
      currentWave = &waves[i]; // * - idź w to miejsce, & - gdzie to stoi
      break;
    }
  }

  //Dodanie pinu:
  if (currentWave == nullptr){
    if (pins_registered < MAX_SQUAREWAVE_PINS){
      currentWave = &waves[pins_registered]; //tabele w c definiujemy od zera stąd odniesienie od ilości pinów siedzi i nie musimy dodawać do tego ilość + 1
      //Strzałka łączy * i .coś bezpośrednio podpinając się pod adres danego elementu
      currentWave->pin = pin_number;
      currentWave->last_time = micros();
      currentWave->state = 0;
      pins_registered++;
    } 
    else {
      return 0;
    }
  }

  if (frequency < 0){
    return 1;
  }

  unsigned long ideal_time =  500000 / frequency;
  if (micros() - currentWave->last_time >= ideal_time) {
    //Do dokończenia
    currentWave->last_time = micros();
    currentWave->state = !currentWave->state;
    digitalWrite(currentWave->pin, currentWave->state);
  }

  return 0;
}