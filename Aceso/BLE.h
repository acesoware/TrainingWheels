/*
 * BLE lib for aceso employees to connect their
 * Adafruit Feather devices to the Aceso App
 */

#ifndef BLE

enum BLE_MODE {OFF,INIT,ON};
void BLE_init();
void BLE_uart();
void BLE_Mode_Set(BLE_MODE mode);
BLE_MODE BLE_Mode_Get();

#endif
