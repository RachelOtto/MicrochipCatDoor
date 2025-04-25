#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xil_types.h"

// Get device IDs from xparameters.h
#define MODE_ID XPAR_AXI_GPIO_BUTTONS_DEVICE_ID
#define LOCK_ID XPAR_AXI_GPIO_LED_DEVICE_ID
#define STATUS_ID XPAR_AXI_GPIO_RGB_DEVICE_ID
#define MICROCHIP_ID XPAR_AXI_GPIO_SWITCHES_DEVICE_ID
#define WEIGHT_ID XPAR_AXI_GPIO_PSWITCHES_DEVICE_ID

#define MODE_CHANNEL 1
#define LOCK_CHANNEL 1
#define STATUS_CHANNEL 1
#define MICROCHIP_CHANNEL 1
#define WEIGHT_CHANNEL 1

#define MODE_MASK 0b0011
#define LOCK_MASK 0b0001
#define STATUS_MASK 0b0111
#define MICROCHIP_MASK 0b1111
#define WEIGHT_MASK 0b1111

int main() {

XGpio_Config *cfg_ptr;
XGpio LOCK_device, MODE_device, STATUS_device, MICROCHIP_device, WEIGHT_device;
const u32 minweight = 0b1000;
const u32 maxweight = 0b1111;
u32 mode, microchip, microchip_stored, weight, flag;
int count;

xil_printf("Entered function main\r\n");

// Initialize LOCK Device
cfg_ptr = XGpio_LookupConfig(LOCK_ID);
XGpio_CfgInitialize(&LOCK_device, cfg_ptr, cfg_ptr->BaseAddress);

// Initialize Button Device
cfg_ptr = XGpio_LookupConfig(MODE_ID);
XGpio_CfgInitialize(&MODE_device, cfg_ptr, cfg_ptr->BaseAddress);

// Initialize STATUS Device
cfg_ptr = XGpio_LookupConfig(STATUS_ID);
XGpio_CfgInitialize(&STATUS_device, cfg_ptr, cfg_ptr->BaseAddress);

// Initialize MICROCHIPitches Device
cfg_ptr = XGpio_LookupConfig(MICROCHIP_ID);
XGpio_CfgInitialize(&MICROCHIP_device, cfg_ptr, cfg_ptr->BaseAddress);

// Initialize PMICROCHIPitches Device
cfg_ptr = XGpio_LookupConfig(WEIGHT_ID);
XGpio_CfgInitialize(&WEIGHT_device, cfg_ptr, cfg_ptr->BaseAddress);

// Set Button Tristate
XGpio_SetDataDirection(&MODE_device, MODE_CHANNEL, MODE_MASK);

// Set LOCK Tristate
XGpio_SetDataDirection(&LOCK_device, LOCK_CHANNEL, 0);

// Set STATUS Tristate
XGpio_SetDataDirection(&STATUS_device, STATUS_CHANNEL, 0);

// Set MICROCHIPitches Tristate
XGpio_SetDataDirection(&MICROCHIP_device, MICROCHIP_CHANNEL, MICROCHIP_MASK);

// Set PMICROCHIPitches Tristate
XGpio_SetDataDirection(&WEIGHT_device, WEIGHT_CHANNEL, WEIGHT_MASK);

typedef enum {CD_IDLE, CD_CONFIGURE, CD_SCANNING, CD_FAIL, CD_SUCCESS} CD_State;
CD_State state = CD_IDLE;

while (1) {

mode = XGpio_DiscreteRead(&MODE_device, MODE_CHANNEL);
mode &= MODE_MASK;
weight = XGpio_DiscreteRead(&WEIGHT_device, WEIGHT_CHANNEL);
weight &= WEIGHT_MASK;

switch(state) {
case -1:
state = CD_IDLE;
break;
case CD_IDLE:
XGpio_DiscreteWrite(&LOCK_device, LOCK_CHANNEL, 0); //Locked
XGpio_DiscreteWrite(&STATUS_device, STATUS_CHANNEL, 0b0000); //Status Light

if (mode == 1)
state = CD_CONFIGURE;
else if ((weight >= minweight) && (weight <= maxweight))
state = CD_SCANNING;
else
state = CD_IDLE;
break;
case CD_CONFIGURE:
XGpio_DiscreteWrite(&LOCK_device, LOCK_CHANNEL, 0); // Locked
XGpio_DiscreteWrite(&STATUS_device, STATUS_CHANNEL, 0b0111); //White Status Light

microchip = XGpio_DiscreteRead(&MICROCHIP_device, MICROCHIP_CHANNEL);
microchip &= MICROCHIP_MASK;
count = count + 1;

if (microchip != 0){
if (microchip != microchip_stored){
microchip_stored = microchip;
count = 0;
state = CD_SUCCESS;
count = 0;
}
}
else if (mode == 0b0001){
if (count >= 100000){
count = 0;
state = CD_IDLE;
}
}
else{
state = CD_CONFIGURE;
}
break;
case CD_SCANNING:
XGpio_DiscreteWrite(&LOCK_device, LOCK_CHANNEL, 0);
XGpio_DiscreteWrite(&STATUS_device, STATUS_CHANNEL, 0);
XGpio_DiscreteWrite(&STATUS_device, STATUS_CHANNEL, 0b0100);

microchip = XGpio_DiscreteRead(&MICROCHIP_device, MICROCHIP_CHANNEL);
microchip &= MICROCHIP_MASK;

if ((microchip == microchip_stored) && (weight >= minweight && weight <= maxweight)){
XGpio_DiscreteWrite(&LOCK_device, LOCK_CHANNEL, 0b0001);
XGpio_DiscreteWrite(&STATUS_device, STATUS_CHANNEL, 0);
state = CD_SUCCESS;
}
else if (weight <= minweight || weight >= maxweight)
state = CD_IDLE;
else
state = CD_SCANNING;
break;
case CD_SUCCESS:
XGpio_DiscreteWrite(&STATUS_device, STATUS_CHANNEL, 0b0010);
count = count+1;
if (count >=  ){
count = 0;
state = CD_IDLE;
}
else
state = CD_SUCCESS;
break;
}
}}
