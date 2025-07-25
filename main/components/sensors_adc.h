#ifndef SENSORS_ADC_H
#define SENSORS_ADC_H

void iniciar_adc(void);

void temperatura_NTC(void *arg);
void lectura_potenciometro(void *arg);

void adc_pot_start(void);
void adc_ntc_start(void);
void configurar_gpio_ntc(void);

void sensors_manager_init(void);
esp_err_t sensors_manager_get_data(sensores_data_t *datos);

void tarea_evaluar_temp(void *pvParameters);


#endif /* SENSORS_ADC_H */