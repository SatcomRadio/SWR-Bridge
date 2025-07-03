#include <LiquidCrystal.h>
#include <math.h>


// ─── Set to 1 temporarily to display detector voltage for calibration ───
#define CAL_MODE 0

// === Hardware pin assignments ===
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
const uint8_t PIN_FWD = A5;  // Forward detector
const uint8_t PIN_REF = A4;  // Reflected detector

// === Fixed parameters for 70 cm band ===
const unsigned int DIODE_mV = 410;   // Detector diode drop (mV)
const float POW_MW  = 7.2;   // *** Replace after calibration ***
const unsigned long FW_MW  = 1363;   // *** Replace after calibration ***
const unsigned long REF_MW  = 507;   // *** Replace after calibration ***

unsigned long POW_CAL  = pow(FW_MW, 2)/POW_MW/1000;
// PowCal= (VoltFWD²/Power) / 1000  (where power is expressed in Watt)

// Timing
const unsigned long REFRESH_INTERVAL = 200;  // ms
unsigned long nextRefresh = 0;

// ────────────────────────────  S E T U P  ───────────────────────────────
void setup() {
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("70 cm Pwr/SWR");
  lcd.setCursor(0, 1); lcd.print("Meter Ready   ");
  delay(2000);
  lcd.clear();
}

// ────────────────────────────  L O O P  ────────────────────────────────
void loop() {
  if (millis() >= nextRefresh) {
    nextRefresh = millis() + REFRESH_INTERVAL;
#if CAL_MODE
    showCalibrationVoltage();
#else
    refreshDisplay();
#endif
  }
}

// ======================================================================
//                     C A L I B R A T I O N   M O D E
// ======================================================================
#if CAL_MODE
void showCalibrationVoltage() {
  float voltFwd = analogRead(PIN_FWD);
  voltFwd = map(voltFwd, 0, 1023, 0, 5000) + DIODE_mV;  // mV
  float voltRef = analogRead(PIN_REF);
  voltRef = map(voltRef, 0, 1023, 0, 5000) + DIODE_mV;  // mV

  lcd.setCursor(0, 0);
  lcd.print("VoltFWD=");
  lcd.print((unsigned int)voltFwd);
  lcd.print("mV   ");
  lcd.setCursor(0, 1);  
  lcd.print("VoltREF=");
  lcd.print((unsigned int)voltRef);
  lcd.print("mV   ");
}
#endif

// ======================================================================
//                 R E G U L A R   M E T E R   D I S P L A Y
// ======================================================================
void refreshDisplay() {
  // ── Read detector voltages ───────────────────────────────────────────
  float voltFwd = analogRead(PIN_FWD);
  float voltRef = analogRead(PIN_REF);

  voltFwd = map(voltFwd, 0, 1023, 0, 5000) + DIODE_mV;  // mV
  voltRef = map(voltRef, 0, 1023, 0, 5000) + DIODE_mV;  // mV

  // ── Power calculation (W) ────────────────────────────────────────────
  double pwrW = pow(voltFwd, 2) / POW_CAL / 1000.0;

  // ── SWR calculation ──────────────────────────────────────────────────
  double swr = -1.0;  // negative ⇒ no meaningful reading
  if (voltFwd > (DIODE_mV + 2)) {
    const float vRatio = voltRef / voltFwd;
    if (vRatio < 1.0) swr = (1 + vRatio) / (1 - vRatio);
  }

  // ── LCD line 0 : Power ───────────────────────────────────────────────
  lcd.setCursor(0, 0);
  lcd.print("Pwr     ");  // clear field
  lcd.setCursor(4, 0);

  if (pwrW < 1.0) {
    const int mW = (int)(pwrW * 1000 + 0.5);
    lcd.print(mW); lcd.print("mW   ");
  } else {
    const int whole = (int)pwrW;
    const int tenth = (int)(pwrW * 10) % 10;
    lcd.print(whole); lcd.print('.'); lcd.print(tenth); lcd.print("W   ");
  }

  // ── LCD line 1 : SWR ────────────────────────────────────────────────
  lcd.setCursor(0, 1);
  lcd.print("SWR ");

  if (swr < 0) {
    lcd.print("-.-- 70cm");
  } else {
    const int swr10 = (int)(swr * 10 + 0.5);
    const int swrW  = swr10 / 10;
    const int swrD  = swr10 % 10;
    if (swrW < 10) lcd.print(' ');  // align
    lcd.print(swrW); lcd.print('.'); lcd.print(swrD); lcd.print(" 70cm");
  }
}
