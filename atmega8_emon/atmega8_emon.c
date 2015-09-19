#ifndef F_CPU
/* prevent compiler error by supplying a default */
# warning "F_CPU not defined for main.c, defining now @ 16Mhz..."
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define discard_measurements	20 //discard first n measurements on power on until the capacitors are charged and voltage is stable
#define half_sine_waves			20 //measure x half sine waves
#define ADC_COUNTS				1024 //ADC resolution
#define MAINS_VOLTAGE			230 
#define MAINS_VOLTAGE_CORR		0.8534 //EDIT HERE
#define MAINS_CURRENT_CORR		57.7777 // EDIT HERE
#define timeout_samples			2000
#define phase_cal				1.7
#define Vcc						4990 // or 5000mV mine is 4.99 V EDIT HERE
#define V_RATIO					MAINS_VOLTAGE_CORR * MAINS_VOLTAGE * ((Vcc/1000.0) / 1024)
#define I_RATIO					MAINS_CURRENT_CORR * ((Vcc/1000.0) / 1024)

volatile uint8_t show = 0;
volatile long ms,s = 0;
volatile float Vrms, Irms = 0.0;
volatile float effV = 0.0, effI = 0.0;
float offsetV = ADC_COUNTS>>1;
float offsetI = ADC_COUNTS>>1;
float lastFilteredV;
float realPower = 0, apparentPower = 0, powerFactor;

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
volatile uint8_t uart_str_complete = 0;
volatile uint8_t uart_str_count = 0;
volatile char uart_string[6] = "";

static int uart_putc(unsigned char c);
static void uart_puts (char *s);


//USART RECIVE INTERRUPT
ISR(USART_RXC_vect)
{
	unsigned char nextChar;
	nextChar = UDR;
	if( uart_str_complete == 0 )
	{
		if( nextChar != '.')
		{
			uart_string[uart_str_count] = nextChar;
			uart_str_count++;
		}
		else
		{
			uart_string[uart_str_count] = '\0';
			uart_str_count = 0;
			uart_str_complete = 1;
		}
	}
}

//INIT USART
static void USART_Init(void)
{
	UBRRL = BAUD_PRESCALE;
	UBRRH = (BAUD_PRESCALE >> 8);
	UCSRB = (1<<TXEN)|(1<<RXEN)|(1<<RXCIE);//
}

static int uart_putc(unsigned char c)
{
	while (!(UCSRA & (1<<UDRE)))  {	}
	UDR = c;
	return 0;
}

void uart_puts (char *s)
{
	while (*s)
	{
		uart_putc(*s);
		s++;
	}
}

static void uart_putf(float n)
{
	uint16_t h,f;
	char buf[4];
	h = n;
	f = (n-h)*10;
	itoa(h,buf,10);
	uart_puts(buf);
	itoa(f,buf,10);
	uart_puts(".");
	uart_puts(buf);
}

//NO NEED TO INCLUDE <math.h>
uint8_t c_sqrt16 (uint16_t q)
{
	uint8_t r, mask;
	uint8_t i = 8*sizeof (r) -1;
	//uint8_t i = 8 -1;
	
	r = mask = 1 << i;
	
	for (; i > 0; i--)
	{
		mask >>= 1;
		
		if (q < (uint16_t) r*r)
		r -= mask;
		else
		r += mask;
	}
	
	if (q < (uint16_t) r*r)
	r -= 1;
	
	return r;
}
// INIT ADC
static void ACD_init() //atmega8
{
	ADMUX = (1<<REFS0);
	ADCSRA |=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //128 == 125KHz @ 16 Mhz
	ADCSRA|=(1<<ADSC); //warmup +dummy
	while (ADCSRA & (1<<ADSC) ) {}
	(void) ADCW;
}

//static void ACD_init() //attiny85
//{
	//ADMUX = PB3;
	//ADMUX &= ~(1 << ADLAR);  //clear for 10 bit ADC
	//ADCSRA |=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1); //64 == 125KHz @ 8MHz
	//ADCSRA|=(1<<ADSC); //warmup +dummy
	//while (ADCSRA & (1<<ADSC) ) {}
	//(void) ADCW;
//}

static uint16_t ADCsingleREAD(uint8_t ch)
{
	ADMUX = (ADMUX & ~(0x1F));
	ADMUX |= ch;
	_delay_us(60);  //120 us
	ADCSRA|=(1<<ADSC);
	while (ADCSRA & (1<<ADSC) ) {}
	return ADC;
}

//the copied / translated measuring function from openenergymonitor
void readLines()
{
	uint8_t zero_crossed = 0;
	uint8_t is_zero = 0;
	uint8_t lastVCross, checkVCross = 0;
	uint16_t raw_v =  0, raw_i = 0;
	uint16_t num_samples = 0;
	uint16_t adc_v = 0;
	float sumV = 0.0, sumI = 0.0, sumP = 0.0;
//	float phaseShiftedV;

	while (is_zero != 1) //wait for the sine wave to cross zero, then start measuring
	{
		adc_v = ADCsingleREAD(PC5);
		if (adc_v < 519 && adc_v > 507)
			is_zero = 1;
	}
	//measure
	while (zero_crossed < half_sine_waves && num_samples < timeout_samples)
	{
		num_samples++;
		lastFilteredV = effV;
		raw_v = ADCsingleREAD(PC5);
		raw_i = ADCsingleREAD(PC4);
		offsetV = offsetV + ((raw_v-offsetV)/1024); 
		offsetI = offsetI +  ((raw_i-offsetI)/1024);
		effV = raw_v - offsetV;
		effI = raw_i - offsetI;
		sumV += effV*effV; //sqV		
		sumI += effI*effI; //sqI

	//	phaseShiftedV = lastFilteredV + phase_cal * (effV - lastFilteredV);
	//	sumP += effI * phaseShiftedV;
		
		lastVCross = checkVCross;
		if (raw_v > adc_v)
		checkVCross = 1;
		else
		checkVCross = 0;
		if (num_samples == 1) lastVCross = checkVCross;
		if (lastVCross != checkVCross) zero_crossed++;
	}
	Vrms = V_RATIO * c_sqrt16(sumV / num_samples);
	Irms = I_RATIO * c_sqrt16(sumI / num_samples);
	realPower = V_RATIO * I_RATIO * sumP / num_samples;
	apparentPower = Vrms * Irms;
	powerFactor = realPower / apparentPower;
}

int main(void)
{
	char buf[4];
	uint8_t i = 0;
	//INIT
	ACD_init();
	USART_Init();
	//RF-modul-init
	DDRD |= 1 << PD2;
	PORTD |= 1<< PD2;
	//
	sei();
	uart_puts("starting...\r\n");
	
	while(1)
	{
		//discard first n measurements on power on until the capacitors are charged and voltage is stable
		if (i < discard_measurements) //discard_measurements
		{
			readLines();
			i++;
			itoa(i,buf,10);
			uart_puts(buf);
			uart_puts("dummy loop\r\n");
		}
		else
		{
			readLines();
			uart_putf(Vrms);
			uart_puts("|");
			uart_putf(Irms);
			uart_puts("|");
			uart_putf(apparentPower);
			uart_puts("\r\n");
		}
		_delay_ms(3000);
	}
}