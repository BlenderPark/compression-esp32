#include <Arduino.h>
#include <math.h>
#include <WiFi.h>

uint16_t samples[1024];
uint16_t original[1024];
uint16_t reconstructed[1024];
uint16_t rawOutput[1024];
int rawBits = 0;
int rleBits = 0;
uint8_t rleOutput[3072];
int rlePos = 0;

void setup() {
  WiFi.mode(WIFI_OFF);
  btStop();
  setCpuFrequencyMhz(240);
  Serial.begin(115200);
  delay(2000);
}

void generateConstant() {
  for (int i = 0; i < 1024; i++) {
    samples[i] = 32768;
  }
}

void generateEventDriven() {
  for (int i = 0; i < 1024; i++) {
    if (i % 128 == 0) {
      samples[i] = 49152;
    } else {
      samples[i] = 32768;
    }
  }
}

void generateRandom(int trialNumber) {
  randomSeed(42 + trialNumber - 1);
  for (int i = 0; i < 1024; i++) {
    samples[i] = random(0, 65536);
  }
}

void generatePeriodic() {
  for (int i = 0; i < 1024; i++) {
    samples[i] = 32768 + (int16_t)(8192 * sin(2 * PI * i / 128.0));
  }
}

void saveOriginal() {
  for (int i = 0; i < 1024; i++) {
    original[i] = samples[i];
  }
}

void deltaZigzag() {
  uint16_t transformed[1024];
  transformed[0] = samples[0];
  for (int i = 1; i < 1024; i++) {
    int16_t error = (int16_t)samples[i] - (int16_t)samples[i - 1];
    transformed[i] = (uint16_t)((error << 1) ^ (error >> 15));
  }
  for (int i = 0; i < 1024; i++) {
    samples[i] = transformed[i];
  }
}

void encodeRaw() {
  for (int i = 0; i < 1024; i++) {
    rawOutput[i] = samples[i];
  }
  rawBits = 1024 * 16;
}

void decodeRaw() {
  for (int i = 0; i < 1024; i++) {
    reconstructed[i] = rawOutput[i];
  }
}

bool verifyRaw() {
  for (int i = 0; i < 1024; i++) {
    if (reconstructed[i] != original[i]) {
      return false;
    }
  }
  return true;
}

void encodeDeltaRLE() {
  rlePos = 0;
  rleBits = 0;
  deltaZigzag();
  uint16_t currentValue = samples[0];
  uint8_t currentCount = 1;
  for (int i = 1; i < 1024; i++) {
    if (samples[i] == currentValue) {
      if (currentCount == 255) {
        rleOutput[rlePos++] = highByte(currentValue);
        rleOutput[rlePos++] = lowByte(currentValue);
        rleOutput[rlePos++] = currentCount;
        rleBits += 24;
        currentCount = 0;
      }
      currentCount++;
    } else {
      rleOutput[rlePos++] = highByte(currentValue);
      rleOutput[rlePos++] = lowByte(currentValue);
      rleOutput[rlePos++] = currentCount;
      rleBits += 24;
      currentValue = samples[i];
      currentCount = 1;
    }
  }
  rleOutput[rlePos++] = highByte(currentValue);
  rleOutput[rlePos++] = lowByte(currentValue);
  rleOutput[rlePos++] = currentCount;
  rleBits += 24;
}

void decodeDeltaRLE() {
  uint16_t deltas[1024];
  int deltaPos = 0;

  // Step 1: reconstruct delta stream from RLE output
  for (int i = 0; i < rlePos; i += 3) {
    uint16_t value = ((uint16_t)rleOutput[i] << 8) | rleOutput[i + 1];
    uint8_t count = rleOutput[i + 2];
    for (int j = 0; j < count; j++) {
      deltas[deltaPos++] = value;
    }
  }

  // Step 2: reverse zigzag on each delta
  int16_t signedDeltas[1024];
  for (int i = 0; i < 1024; i++) {
    uint16_t z = deltas[i];
    signedDeltas[i] = (int16_t)((z >> 1) ^ -(z & 1));
  }

  // Step 3: reconstruct original samples
  reconstructed[0] = (uint16_t)signedDeltas[0];
  for (int i = 1; i < 1024; i++) {
    reconstructed[i] = (uint16_t)((int16_t)reconstructed[i - 1] + signedDeltas[i]);
  }
}

bool verifyDeltaRLE() {
  for (int i = 0; i < 1024; i++) {
    if (reconstructed[i] != original[i]) {
      return false;
    }
  }
  return true;
}
#include <Arduino.h>
#include <math.h>
#include <WiFi.h>

uint16_t samples[1024];
uint16_t original[1024];
uint16_t reconstructed[1024];
uint16_t rawOutput[1024];
int rawBits = 0;
int rleBits = 0;
uint8_t rleOutput[3072];
int rlePos = 0;

void setup() {
  WiFi.mode(WIFI_OFF);
  btStop();
  setCpuFrequencyMhz(240);
  Serial.begin(115200);
  delay(2000);
}

void generateConstant() {
  for (int i = 0; i < 1024; i++) {
    samples[i] = 32768;
  }
}

void generateEventDriven() {
  for (int i = 0; i < 1024; i++) {
    if (i % 128 == 0) {
      samples[i] = 49152;
    } else {
      samples[i] = 32768;
    }
  }
}

void generateRandom(int trialNumber) {
  randomSeed(42 + trialNumber - 1);
  for (int i = 0; i < 1024; i++) {
    samples[i] = random(0, 65536);
  }
}

void generatePeriodic() {
  for (int i = 0; i < 1024; i++) {
    samples[i] = 32768 + (int16_t)(8192 * sin(2 * PI * i / 128.0));
  }
}

void saveOriginal() {
  for (int i = 0; i < 1024; i++) {
    original[i] = samples[i];
  }
}

void deltaZigzag() {
  uint16_t transformed[1024];
  transformed[0] = samples[0];
  for (int i = 1; i < 1024; i++) {
    int16_t error = (int16_t)samples[i] - (int16_t)samples[i - 1];
    transformed[i] = (uint16_t)((error << 1) ^ (error >> 15));
  }
  for (int i = 0; i < 1024; i++) {
    samples[i] = transformed[i];
  }
}

void encodeRaw() {
  for (int i = 0; i < 1024; i++) {
    rawOutput[i] = samples[i];
  }
  rawBits = 1024 * 16;
}

void decodeRaw() {
  for (int i = 0; i < 1024; i++) {
    reconstructed[i] = rawOutput[i];
  }
}

bool verifyRaw() {
  for (int i = 0; i < 1024; i++) {
    if (reconstructed[i] != original[i]) {
      return false;
    }
  }
  return true;
}

void encodeDeltaRLE() {
  rlePos = 0;
  rleBits = 0;
  deltaZigzag();
  uint16_t currentValue = samples[0];
  uint8_t currentCount = 1;
  for (int i = 1; i < 1024; i++) {
    if (samples[i] == currentValue) {
      if (currentCount == 255) {
        rleOutput[rlePos++] = highByte(currentValue);
        rleOutput[rlePos++] = lowByte(currentValue);
        rleOutput[rlePos++] = currentCount;
        rleBits += 24;
        currentCount = 0;
      }
      currentCount++;
    } else {
      rleOutput[rlePos++] = highByte(currentValue);
      rleOutput[rlePos++] = lowByte(currentValue);
      rleOutput[rlePos++] = currentCount;
      rleBits += 24;
      currentValue = samples[i];
      currentCount = 1;
    }
  }
  rleOutput[rlePos++] = highByte(currentValue);
  rleOutput[rlePos++] = lowByte(currentValue);
  rleOutput[rlePos++] = currentCount;
  rleBits += 24;
}

void decodeDeltaRLE() {
  uint16_t deltas[1024];
  int deltaPos = 0;

  // Step 1: reconstruct delta stream from RLE output
  for (int i = 0; i < rlePos; i += 3) {
    uint16_t value = ((uint16_t)rleOutput[i] << 8) | rleOutput[i + 1];
    uint8_t count = rleOutput[i + 2];
    for (int j = 0; j < count; j++) {
      deltas[deltaPos++] = value;
    }
  }

  // Step 2: reverse zigzag on each delta
  int16_t signedDeltas[1024];
  for (int i = 0; i < 1024; i++) {
    uint16_t z = deltas[i];
    signedDeltas[i] = (int16_t)((z >> 1) ^ -(z & 1));
  }

  // Step 3: reconstruct original samples
  reconstructed[0] = (uint16_t)signedDeltas[0];
  for (int i = 1; i < 1024; i++) {
    reconstructed[i] = (uint16_t)((int16_t)reconstructed[i - 1] + signedDeltas[i]);
  }
}

bool verifyDeltaRLE() {
  for (int i = 0; i < 1024; i++) {
    if (reconstructed[i] != original[i]) {
      return false;
    }
  }
  return true;
}

void loop() {
  generateConstant();
  saveOriginal();
  encodeRaw();
  decodeRaw();
  Serial.println(verifyRaw() ? "Raw PASS" : "Raw FAIL");

  generateConstant();
  saveOriginal();
  encodeDeltaRLE();
  decodeDeltaRLE();
  Serial.println(verifyDeltaRLE() ? "DeltaRLE PASS" : "DeltaRLE FAIL");

  while(true);
}
void loop() {
  generateConstant();
  saveOriginal();
  encodeRaw();
  decodeRaw();
  Serial.println(verifyRaw() ? "Raw PASS" : "Raw FAIL");

  generateConstant();
  saveOriginal();
  encodeDeltaRLE();
  decodeDeltaRLE();
  Serial.println(verifyDeltaRLE() ? "DeltaRLE PASS" : "DeltaRLE FAIL");

  while(true);
}
