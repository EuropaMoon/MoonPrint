
#include "HotEnd.h"

HotEnd::HotEnd() : values() {
    memset(values, 0, sizeof(values));
    pinMode(FAN_PIN, OUTPUT);
    digitalWrite(FAN_PIN, HIGH);
}

float HotEnd::getTemperature() {
    int readOut = analogRead(NTC_PIN);

    float T0 = 25 + ABSZERO;
    double RN = RESISTOR * (readOut / MAXANALOGREAD) / (1 - (readOut / MAXANALOGREAD));
    return float(T0 * 3976 / (3976 + T0 * log(RN / 100000)) - ABSZERO);
}

void HotEnd::setTemperature(float temp) {
    targetTemperature = temp;
}

bool HotEnd::update(LCD &lcd) {
    currentTemp = getTemperature();

    // print temperature on lcd
    lcd.print(0, 1, String(currentTemp));

    if(currentTemp < MIN_TEMP || currentTemp > MAX_TEMP) {
        analogWrite(HOT_END_PIN, 0);
        return false;
    }

    // log error over time
    if(millis() - lastProbe >= 250) {
        memcpy(values, &values[1], sizeof(values) - sizeof(float));
        values[sizeof(values) / sizeof(float) - 1] = targetTemperature - currentTemp;
        lastProbe = millis();

        // Calculate sum
        arraySum = 0;
        for(float value : values) {
            arraySum += value;
        }
    }

    // Calculate analog output
    mv = values[sizeof(values) / sizeof(float) - 1] * Kp + arraySum * Ki +
         ((values[0] - values[4])) * Kd + b;

    if (mv < 0) {
        mv = 0;
    } else if(mv > 255) {
        mv = 255;
    }

    analogWrite(HOT_END_PIN, int(mv));

    if(targetTemperature * 1.05 > targetTemperature - arraySum / (float(sizeof(values)) / sizeof(float)) &&
       targetTemperature * 0.95 < targetTemperature - arraySum / (float(sizeof(values)) / sizeof(float))) {
        return true;
    }

    return false;
}
