// File: usbd_cdc_if.c

// Volatile globals are used for communication between the main
// loop, which takes data and later prints it, and the serial-
// recieve interrupt service routine, which sets the flags to 
// trigger printing or clearing data.  The variables are defined
// in main.c. 
extern volatile int do_send_readings;
extern volatile int do_clear;


static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
	if (Buf[0] == 'G') {
		do_send_readings = 1;
	} else if (Buf[0] == 'C') {
		do_clear = 1;
	}

	// We did not write the code from here to the end of the function
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
}



### Blink
// File: main.c

while (1)
{
	/* USER CODE END WHILE */

	/* USER CODE BEGIN 3 */
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	HAL_GPIO_TogglePin(GPIOA, Header1_Pin);
	HAL_Delay(500);
}

### Analog Read


### Final Code
// File: main.c

// Apparently const's are nicer than #DEFINE's?
const int num_readings = 16384; // How many readings to store

volatile int do_send_readings = 0; // really a bool 
volatile int do_clear = 0;

int main(void) {

	int32_t readings[num_readings];
	int valid_readings = 0; // If < num_readings, then the rest of
													// the readings are uninitialized
	int curr_reading = 0; // Oldest readings, and next to be written

	// Turn on the IR and green LEDs
	HAL_GPIO_WritePin(IRLED_GPIO_Port, IRLED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GreenLED_GPIO_Port, GreenLED_Pin, GPIO_PIN_SET);

  while (1) {
		if (valid_readings < num_readings) { // Delete for rolling buffer
			HAL_ADC_Start(&hadc1);
			HAL_ADC_PollForConversion(&hadc1, 100); // Green
			readings[curr_reading] = HAL_ADC_GetValue(&hadc1);
			HAL_ADC_Stop(&hadc1);

			if (valid_readings < num_readings) valid_readings++;
			curr_reading = (curr_reading + 1) % num_readings;
		}

		if (do_send_readings) {
			// send data
			char data[16];
			for(int i = 0; i < valid_readings; i++) {
				sprintf(data, "%ld\n", readings[(i+curr_reading) % num_readings]);
				CDC_Transmit_FS((unsigned char*)data, strlen(data));
			}
			do_send_readings = 0;
		}
		if (do_clear) {
			// clear data
			valid_readings = 0;
			curr_reading = 0;
			do_clear = 0;
		}

		HAL_Delay(1); // Sleep for 1ms

  }
