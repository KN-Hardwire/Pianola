#include <Arduino.h>
#include <stdio.h>

#include "USB.h"
#include "USBMIDI.h"

// Wejścia
#define potentiometer1 1
#define MAX_SQUAREWAVE_PINS 10

// Wyjścia
#define SCK 18   // zegar rejstru przesowanegoo
#define RCK 3    // zegar przepisujący na wyjścia
#define nSCLR 17 // czyszczenie rejstru
#define SI 8 // dane wejściowe rejstru

// Matryca LED
#define W1 4
#define W2 5
#define W3 6
#define W4 7

USBMIDI Midi;

const int inputs[] = {W1, W2, W3, W4, potentiometer1};
const int outputs[] = {SI, SCK, RCK, nSCLR};
bool dane[] = {1, 0, 0, 0, 0, 0, 0, 0};

struct waveData {
  int pin;
  unsigned long last_time;
  bool state;
};

int squarewave(int pin_number, int frequency);
int midiVelocity(unsigned long time);

void setup() {
  // Konfiguracja pinów:
  for (int p : outputs) {
    pinMode(p, OUTPUT);
  }
  for (int p : inputs) {
    pinMode(p, INPUT);
  }

  Serial.begin(115200);
  // ZMIANA: Natywna konfiguracja USB
  USB.PID(0x8765); // Wymuszenie re-enumeracji w Windows bo się nie chciał
                   // połączyć jako midi
  USB.productName("ESP_MIDI_CONTROLLER");
  USB.manufacturerName("Pianola");

  Midi.begin();
  USB.begin();
}

void loop() {
  // ===============================
  // Część kodu odpowiedzialna za HARDWARE
  // ===============================
  static int bitIndex = 0; // Jedna pętla loop zczytuje tylko jedną kolumnę!
  static bool lastSckState = 0;

  // Można też czytać piny ustawione jako wyjścia - cool
  bool currentSckState = digitalRead(SCK);
  digitalWrite(nSCLR, HIGH);
  squarewave(SCK, 2000);

  if (currentSckState == HIGH && lastSckState == 0) {
    digitalWrite(SI, dane[bitIndex]);
    bitIndex++;
    // restart
    if (bitIndex >= 4) {
      bitIndex = 0;
    }
    digitalWrite(RCK, LOW);
    digitalWrite(RCK, HIGH);
  }
  lastSckState = currentSckState;

  //======================================================
  // Część kodu odpowiedzialna za obliczanie danych MIDI
  //======================================================
  static int lastTimeSerial = 0;
  static int lastNoteNumber = 60;
  static bool noteIsPlaying[17] = {false};
  static long noteStartTime[4] = {0}; // czyścimy tabelkę (jak podamy mniej
                                      // arg to reszta uzupełnia się zerami)

  enum ContactType {
    CONTACT_START,
    CONTACT_STOP,
    CONTACT_FIXED
  }; // Czytelność instr war

  struct KeyContact {
    enum ContactType type;
    int noteOffset;
  };

  // clang-format off
  const KeyContact matrixMap[16] =
  {
    // Pierwsze 8 przycisków. Składnia: {[typ], [offset]},...
    {CONTACT_START, 0}, {CONTACT_STOP, 0}, // Klawisz 1
    {CONTACT_START, 1}, {CONTACT_STOP, 1}, // Klawisz 2
    {CONTACT_START, 2}, {CONTACT_STOP, 2}, // Klawisz 3
    {CONTACT_START, 3}, {CONTACT_STOP, 3}, // Klawisz 4
    // Reszta
    {CONTACT_FIXED, 4}, {CONTACT_FIXED, 5}, {CONTACT_FIXED, 6}, {CONTACT_FIXED, 7},
    {CONTACT_FIXED, 8}, {CONTACT_FIXED, 9}, {CONTACT_FIXED, 10}, {CONTACT_FIXED, 11} 
  };
  // clang-format on

  for (int i = 0; i < 4; i++) {
    int buttonNumber = bitIndex * 4 + i;
    int currentNote = 60 + matrixMap[buttonNumber].noteOffset;
    bool isPressed = (digitalRead(inputs[i]) == HIGH); // cool zastępstwo za if'a

    if (isPressed && !noteIsPlaying[buttonNumber]) {
      // Debugging
      if (digitalRead(inputs[i]) == 1) {
        if ((millis() - lastTimeSerial) >= 250) {
          Serial.print("Numer przycisku: ");
          Serial.println(buttonNumber);
          Serial.print("Wysłana nuta: ");
          Serial.println(currentNote);
          lastTimeSerial = millis();
        }
        // Obliczenia
        if (matrixMap[buttonNumber].type == CONTACT_START) {
          noteStartTime[matrixMap[buttonNumber].noteOffset] =
              micros(); // crazy syntax
        } else if (matrixMap[buttonNumber].type == CONTACT_STOP) {
          unsigned long delta_t =
              micros() - noteStartTime[matrixMap[buttonNumber].noteOffset];
          // Wyślij komunikat MIDI
          Midi.noteOn(currentNote, midiVelocity(delta_t), 1);
        } else if (matrixMap[buttonNumber].type == CONTACT_FIXED) {
          Midi.noteOn(currentNote, 100, 1);
        }
        noteIsPlaying[buttonNumber] = true;
      }
    }
    if (!isPressed && noteIsPlaying[buttonNumber]) {
      // sprzątanie
      Midi.noteOff(currentNote, 0, 1);
      noteIsPlaying[buttonNumber] = false;
    }
  }
      // Dodanie potencjometru
    static unsigned long lastAnalogTime = 0;
    static int lastPotValue = 0;

    if (millis() - lastAnalogTime >= 20) {
        int value = analogRead(potentiometer1);
        int currentPotValue = map(value, 0, 4095, 0, 127);

        if (abs(currentPotValue - lastPotValue) > 1) {
            Midi.controlChange(1, currentPotValue, 1);
            lastPotValue = currentPotValue;
        }
        lastAnalogTime = millis();
    }
}

// ===================
// Funkcje:
// ===================

// od razu się rzuca, że fajnie by byłozrobić funkcję delay i funkcję dla
// przebiegów wysyłających okresowo jakąś ramkę danych na jakimś pinei

int squarewave(int pin_number, int frequency) {
  static waveData waves[MAX_SQUAREWAVE_PINS];
  static int pins_registered = 0;
  // tworzenie pustego wskaźnika i określanie rozmiaru elementu w
  // tym segmencie pamięci (pod nasz tym struct)
  // było to ważne również, żeby procecsor wiedział jak zinterpretować dane
  // pod tym adresem
  waveData *currentWave = nullptr;

  // Wyszukiwanie pinu w liście zarejstrowanych już wcześniej
  for (int i = 0; i < pins_registered; i++) {
    if (waves[i].pin == pin_number) { 
      currentWave = &waves[i];        // * - idź w to miejsce, & - gdzie to stoi
      break;
    }
  }

  // Dodanie pinu:
  if (currentWave == nullptr) {
    if (pins_registered < MAX_SQUAREWAVE_PINS) {
      // tabele w c definiujemy od zera stąd odniesienie od ilości pinów
      // siedzi i nie musimy dodawać do tego ilość + 1 Strzałka łączy "*"" i
      // ".coś" bezpośrednio podpinając się pod adres danego elementu
      currentWave = &waves[pins_registered];
      currentWave->pin = pin_number;
      currentWave->last_time = micros();
      currentWave->state = 0;
      pins_registered++;
    } else {
      return 0;
    }
  }
  // dodałem bo crashowało
  if (currentWave == nullptr)
    return 0;

  if (frequency < 0) {
    return 1;
  }

  unsigned long ideal_time = 500000 / frequency;
  if (micros() - currentWave->last_time >= ideal_time) {
    // Do dokończenia
    currentWave->last_time = micros();
    currentWave->state = !currentWave->state;
    digitalWrite(currentWave->pin, currentWave->state);
  }

  return 0;
}

int midiVelocity(unsigned long time) {
  // proof of concept
  const int MAX_TIME = 900000;
  const int MIN_TIME = 50000;

  if (time <= MIN_TIME) return 127;
  if (time >= MAX_TIME) return 1;

  const int spaceInterval = (MAX_TIME - MIN_TIME) / 128;
  unsigned long steps = (time - MIN_TIME) / spaceInterval;
  
  return 127 - steps;
}