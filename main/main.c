#include <stdio.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define ADC_CHANNEL     ADC_CHANNEL_0    // GPIO3 for ADC1_CHANNEL_3
#define ADC_ATTEN       ADC_ATTEN_DB_11  // Allows measuring up to ~3.6V
#define ADC_UNIT        ADC_UNIT_1       // ADC1
#define VOLTAGE_DIVIDER_RATIO 2.0        // Voltage divider ratio for 1kΩ + 1kΩ
#define DEFAULT_VREF    1100             // Default reference voltage in mV (adjust as needed)

#define BATTERY_VOLTAGE_MAX 4.2          // Maximum battery voltage (fully charged)
#define BATTERY_VOLTAGE_MIN 3.0          // Minimum battery voltage (discharged)

static esp_adc_cal_characteristics_t *adc_chars;

// Function to initialize the ADC
void init_adc() {
    // Configure ADC width
    adc1_config_width(ADC_WIDTH_BIT_12);

    // Configure ADC channel attenuation
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);

    // Characterize the ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT, ADC_ATTEN, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
}

// Function to read the battery voltage
float read_battery_voltage() {
    // Get raw ADC reading
    int adc_raw = adc1_get_raw(ADC_CHANNEL);

    // Debug: Print raw ADC value
    printf("Raw ADC Value: %d\n", adc_raw);

    // Convert raw reading to voltage in mV
    uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(adc_raw, adc_chars);

    // Debug: Print voltage at ADC pin
    printf("Voltage at ADC Pin: %ld mV\n", voltage_mv);

    // Calculate the actual battery voltage
    float battery_voltage = (voltage_mv / 1000.0) * VOLTAGE_DIVIDER_RATIO;

    return battery_voltage;
}

// Function to calculate the battery level percentage
int calculate_battery_percentage(float voltage) {
    if (voltage >= BATTERY_VOLTAGE_MAX) {
        return 100; // Fully charged
    } else if (voltage <= BATTERY_VOLTAGE_MIN) {
        return 0; // Fully discharged
    } else {
        return (int)((voltage - BATTERY_VOLTAGE_MIN) / (BATTERY_VOLTAGE_MAX - BATTERY_VOLTAGE_MIN) * 100);
    }
}

void app_main() {
    // Initialize the ADC
    init_adc();

    while (1) {
        // Read battery voltage
        float battery_voltage = read_battery_voltage();

        // Calculate battery percentage
        int battery_percentage = calculate_battery_percentage(battery_voltage);

        // Print battery voltage and percentage
        printf("Battery Voltage: %.2f V\n", battery_voltage);
        printf("Battery Level: %d%%\n", battery_percentage);

        // Delay for 1 second
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
