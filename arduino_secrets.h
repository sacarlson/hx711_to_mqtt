#define SECRET_BROKER_URL "your_mqtt_broker.site"
#define SECRET_BROKER_PORT 1883
#define SECRET_BROKER_USER "buser"
#define SECRET_BROKER_PASS "bpass"
#define SECRET_BROKER_TOPIC "water/level"

// to read SSID and PASS from eeprom leave SECRET_SSID values blank ""
// to override EEprom wifi settings set SSID and password here
#define SECRET_SSID ""
//#define SECRET_SSID "wifi_ssid"
#define SECRET_PASS "wifi_password"

// set ENABLE_POWER_SAVE to 0 to disable power save mode.
#define ENABLE_POWER_SAVE 1
// power save settings for lifepo4 battery set (2 cells)
// lefepo4 battery cell range of charge is 2.5v to 3.65 float 3.2v
// settings bellow 5.3v will disable desired power save mode
// mode 1 sleep for 60 seconds
#define VOLTAGE_PS_MODE1 6.68
//#define VOLTAGE_PS_MODE1 6.35
// mode 2 sleep for 1 hour
#define VOLTAGE_PS_MODE2 5.2
//#define VOLTAGE_PS_MODE2 6.25
// mode 3 sleep for 24 hours
#define VOLTAGE_PS_MODE3 5.2
//#define VOLTAGE_PS_MODE3 6.1

// this value 0.069 was used for voltage spliter of 220k and 10k to A0
#define A2D_VOLT_CALFACTOR 0.069

// to read zero_refa and next 4 from eeprom set these to 0
#define UZERO_REFA 0
#define UCAL_FACTORA 0
//#define UZERO_REFB 0
//#define UCAL_FACTORB 0

// to override eeprom settings set these values to what you want here.
//#define UZERO_REFA 646154
//#define UCAL_FACTORA 50093.0
#define UZERO_REFB -1017130
#define UCAL_FACTORB 50093.0
