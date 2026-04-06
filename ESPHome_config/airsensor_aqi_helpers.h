#pragma once

// =============================================================================
// airsensor_aqi_helpers.h
//
// Shared AQI logic for all AirSensor units.
// Included by each YAML via `esphome: includes:`.
//
// Responsibilities:
//   - Map raw sensor readings to a 5-level air quality scale
//   - Derive a single worst-of score across all pollutants
//   - Provide display strings and colours for each level
//
// Level scale:
//   0 = no data (NaN)   — grey
//   1 = Excellent        — green
//   2 = Good             — yellow
//   3 = Fair             — orange
//   4 = Poor             — red-orange
//   5 = Very Poor        — red
//
// Thresholds are aligned with Home Assistant's default AQI card breakpoints.
// =============================================================================

#include <cmath>

#include "esphome/core/color.h"

namespace airsensor {

// Bundles all sensor readings passed to AQI functions in a single call.
struct Readings {
  float co2;   // ppm  — from SCD4x
  float voc;   // VOC Index (1–500 dimensionless) — from SEN5x
  float nox;   // NOx Index (1–500 dimensionless) — from SEN5x
  float pm25;  // µg/m³ — from SEN5x
  float pm10;  // µg/m³ — from SEN5x
};

// Returns a level 0–5 by comparing value against four ascending thresholds.
// Returns 0 if value is NaN (sensor not ready / unavailable).
inline int level_from_thresholds(float value, float t1, float t2, float t3, float t4) {
  if (std::isnan(value)) {
    return 0;
  }
  if (value <= t1) {
    return 1;
  }
  if (value <= t2) {
    return 2;
  }
  if (value <= t3) {
    return 3;
  }
  if (value <= t4) {
    return 4;
  }
  return 5;
}

// CO2 in ppm:  ≤800 / ≤1000 / ≤1400 / ≤2000 / >2000
inline int co2_level(float value) {
  return level_from_thresholds(value, 800.0f, 1000.0f, 1400.0f, 2000.0f);
}

// VOC Index:  ≤100 / ≤200 / ≤300 / ≤500 / >500
inline int voc_level(float value) {
  return level_from_thresholds(value, 100.0f, 200.0f, 300.0f, 500.0f);
}

// NOx Index:  ≤50 / ≤100 / ≤200 / ≤400 / >400
inline int nox_level(float value) {
  return level_from_thresholds(value, 50.0f, 100.0f, 200.0f, 400.0f);
}

// PM2.5 µg/m³ (WHO / EPA breakpoints):  ≤12 / ≤35 / ≤55 / ≤150 / >150
inline int pm25_level(float value) {
  return level_from_thresholds(value, 12.0f, 35.0f, 55.0f, 150.0f);
}

// PM10 µg/m³:  ≤20 / ≤50 / ≤100 / ≤250 / >250
inline int pm10_level(float value) {
  return level_from_thresholds(value, 20.0f, 50.0f, 100.0f, 250.0f);
}

// Returns the highest level across all pollutants (worst-of logic).
// A level of 0 on any individual pollutant (no data) does not drag the
// worst level down — it is simply ignored.
inline int worst_level(const Readings &readings) {
  int worst = 0;
  int current = co2_level(readings.co2);
  if (current > worst) {
    worst = current;
  }
  current = voc_level(readings.voc);
  if (current > worst) {
    worst = current;
  }
  current = nox_level(readings.nox);
  if (current > worst) {
    worst = current;
  }
  current = pm25_level(readings.pm25);
  if (current > worst) {
    worst = current;
  }
  current = pm10_level(readings.pm10);
  if (current > worst) {
    worst = current;
  }
  return worst;
}

// Human-readable label shown on the display for each level.
inline const char *status_text(int level) {
  switch (level) {
    case 1:
      return "EXCELLENT";
    case 2:
      return "GOOD";
    case 3:
      return "FAIR";
    case 4:
      return "POOR";
    case 5:
      return "VERY POOR";
    default:
      return "NO DATA";
  }
}

// UI colours ----------------------------------------------------------------

// Muted grey used for inactive bar segments and secondary labels.
inline esphome::Color dim_gray() {
  return esphome::Color(153, 153, 153);  // #999999
}

// Deep blue background fill for the round display.
inline esphome::Color background_color() {
  return esphome::Color(36, 41, 191);  // #2429BF
}

// Colour associated with each AQI level.  Level 0 (no data) returns dim_gray.
inline esphome::Color level_color(int level) {
  switch (level) {
    case 1:
      return esphome::Color(0, 217, 77);    // #00D94D — green
    case 2:
      return esphome::Color(191, 179, 0);   // #BFB300 — yellow
    case 3:
      return esphome::Color(255, 140, 0);   // #FF8C00 — orange
    case 4:
      return esphome::Color(255, 77, 0);    // #FF4D00 — red-orange
    case 5:
      return esphome::Color(255, 0, 26);    // #FF001A — red
    default:
      return dim_gray();
  }
}

}  // namespace airsensor
