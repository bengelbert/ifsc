//******************************************************************************************************************
//											Programming Project Integrator III IFSC 2010
//******************************************************************************************************************
//  Nome do arquivo : Multimeter Ammeter AVR Microcontroller
//  Vers�o			: 1.1
//  Descri��o	 	: Meter Electric Magnitudes Microcontrolled Interface with printing Date and Time updated
//  Autores			: Geiser Pr� Vicenzi e Morgana Cristina L�
//  Destino			: AVR Atmega 168/88
//  Compiler		: AVR Studio 4.18.684 avr-libc (WinAVR 20090313)
//  Atualiza��o		: 2010
//******************************************************************************************************************
//------------------------------------------------------------------------------------------------------------------
//	Libraries
//------------------------------------------------------------------------------------------------------------------
#include <avr/io.h>
#include <util/delay.h>
#include <compat/twi.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>

#include "timer.h"

/******************************************************************************/
/*
 * Constants
 */
#define LED_PORT PORTD
#define LED1     PORTD4
#define LED2     PORTD5
#define LED3     PORTD6
#define LED4     PORTD7

//------------------------------------------------------------------------------------------------------------------
//	Definitions RTC DS1307 - I2C Slave
//------------------------------------------------------------------------------------------------------------------
#define BAUD_RATE 	 19200
#define MAX_TRIES 	 50
#define DS1307_ID    0xD0        //I2C DS1307 Device Identifier
#define DS1307_ADDR  0x00        //I2C DS1307 Device Address
//------------------------------------------------------------------------------------------------------------------
//	Definitions I2C
//------------------------------------------------------------------------------------------------------------------
#define I2C_START 	 0
#define I2C_DATA 	 1
#define I2C_DATA_ACK 2
#define I2C_STOP 	 3

#define ACK 		 1			//Acknowledge bit response = tells whether the 9bit was correctly received 
#define NACK 		 0			//No-Acknowledge bit response = tells you if it 9bit was not properly received 
//------------------------------------------------------------------------------------------------------------------
//	Definitions LCD 16x2 (Hitachi HD44780U)
//------------------------------------------------------------------------------------------------------------------
#define LCD_HOME 	  0x02
#define LCD_NEXT_LINE 0xC0
#define LCD_CLEAR 	  0x01
#define LCD_1CYCLE 	  0
#define LCD_2CYCLE 	  1
//------------------------------------------------------------------------------------------------------------------
//	Definitions General
//------------------------------------------------------------------------------------------------------------------
// DS1307 Register Address
// Second: ds1307_addr[0]
// Minute: ds1307_addr[1]
// Hour  : ds1307_addr[2]
// Day   : ds1307_addr[3]
// Date  : ds1307_addr[4]
// Month : ds1307_addr[5]
// Year  : ds1307_addr[6]

//Hour Format
#define HOUR_24		0			//Setup
#define HOUR_12 	1			//Setup
//------------------------------------------------------------------------------------------------------------------
//	Global Variables
//------------------------------------------------------------------------------------------------------------------

/******************************************************************************/
/*
 * Function prototypes
 */
void    main_setup_io   (void);

/******************************************************************************/
/*
 * Variables
 */
char ds1307_addr[7]; //
char sdigit[3] = {'0', '0', '\0'};
char *weekday[] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};
char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char hour_mode, ampm_mode;
int icount;

unsigned char mode, ichoice;

//------------------------------------------------------------------------------------------------------------------
//	Initializing subroutine USART (Universal Synchronous and Asynchronous serial Receiver and Transmitter)
//------------------------------------------------------------------------------------------------------------------

void uart_init(void) //Function is used to initialize the ATMega168 UART peripheral
{
    UBRR0H = (((F_CPU / BAUD_RATE) / 16) - 1) >> 8; //Set baud rate >>shifts 8 bits right (Asynchronous Normal mode)
    UBRR0L = (((F_CPU / BAUD_RATE) / 16) - 1);

    UCSR0B = (1 << RXEN0) | (1 << TXEN0); //(Receiver & Transmitter) Enable Rx & Tx
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); //(Character Size) Config USART 8N1 (8data 1stop bit)
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine sending frames with 5 a 8 bits (Write)
//------------------------------------------------------------------------------------------------------------------

int uart_putch(char ch, FILE *stream) //Function used to put single character to the UART port
{
    if (ch == '\n')
        uart_putch('\r', stream);

    while (!(UCSR0A & (1 << UDRE0))); //Bit UDRE0 = 1 (enabled for writing)
    UDR0 = ch;

    return 0;
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine receiving frames	with 5 a 8 bits (Reads)
//------------------------------------------------------------------------------------------------------------------

int uart_getch(FILE *stream) //Function used to read single character from the UART port
{
    unsigned char ch;
    while (!(UCSR0A & (1 << RXC0))); //Bit RXC0 = 1 (enabled when received and clears the buffer when read)
    ch = UDR0;

    /*Echo the Output Back to terminal*/

    uart_putch(ch, stream);

    return ch;
}
//------------------------------------------------------------------------------------------------------------------
//	Handling communication with the PC
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//	Subroutine clear the screen of ANSI terminal emulation and display character
//------------------------------------------------------------------------------------------------------------------

void ansi_cl(void) //Function used to clear the ANSI emulation terminal screen
{
    //ANSI clear screen: cl=\E[H\E[J
    putchar(27);
    putchar('[');
    putchar('H');
    putchar(27);
    putchar('[');
    putchar('J');
    //printf("");
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine turn off all attribute on the ANSI terminal emulation and display character
//------------------------------------------------------------------------------------------------------------------

void ansi_me(void) //Function used to turn off all attribute on the ANSI terminal emulation
{
    //ANSI turn off all attribute: me=\E[0m
    putchar(27);
    putchar('[');
    putchar('0');
    putchar('m');
    //printf("");
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine move cursor on the ANSI terminal emulation and diplay character
//------------------------------------------------------------------------------------------------------------------

void ansi_cm(unsigned char row, unsigned char col) //Function used to move cursor on the ANSI terminal emulation
{
    //ANSI cursor movement: cl=\E%row;%colH
    putchar(27);
    putchar('[');
    printf("%d", row);
    putchar(';');
    printf("%d", col);
    putchar('H');
    //printf("");
}
//------------------------------------------------------------------------------------------------------------------
//	 Handling communication with the LCD
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//	Subroutine display single character on the LCD (Send Data)
//------------------------------------------------------------------------------------------------------------------

void LCD_putch(unsigned char data) //Function used to display single character on the LCD
{
    //LCD Upper 4 bits data (DB7,DB6,DB5,DB4)
    PORTD = (1 << PD3) | (1 << PD2) | (data & 0xF0); //RS = 1, E = 1

    // E=0; write data
    PORTD &= ~(1 << PD3);
    _delay_us(1);

    //LCD Lower 4 bits data (DB3,DB2,DB1,DB0)
    PORTD = (1 << PD3) | (1 << PD2) | ((data & 0x0F) << 4); //RS = 1, E = 1 (<< shifts 4 bits left)

    //E=0; write data
    PORTD &= ~(1 << PD3);
    _delay_ms(5); //Wait for busy flag (BF)
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine sending commands to the LCD (Send Instruction)
//------------------------------------------------------------------------------------------------------------------

void LCD_putcmd(unsigned char data, unsigned char cmdtype) //Function used to send LCD command
{
    //LCD Upper 4 bits data (DB7,DB6,DB5,DB4)
    PORTD = (1 << PD3) | (data & 0xF0); //RS = 0, E = 1

    //E=0; write data
    PORTD &= ~(1 << PD3);
    _delay_us(1);

    //cmdtype = 0; One cycle write, cmdtype = 1; Two cycle writes
    if (cmdtype) {
        //LCD Lower 4 bits data (DB3,DB2,DB1,DB0)
        PORTD = (1 << PD3) | ((data & 0x0F) << 4); //RS = 0, E = 1 (<< shifts 4 bits left)
        //E=0; write data
        PORTD &= ~(1 << PD3);
    }
    _delay_ms(5); //Wait for busy flag (BF)
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine initialized the LCD 16x2 mode 4bits
//------------------------------------------------------------------------------------------------------------------

void initlcd() //Function used to initialized the 16x2 LCD
{
    //Power ON - Stabilization
    //Wait for more than 15 ms after VCC rises to 4.5 V
    _delay_ms(30);

    //Send Command Instruction 0x30 (1 cycle)
    LCD_putcmd(0x30, LCD_1CYCLE);
    _delay_ms(8); //Wait for more than 4.1 ms

    //Send Command Instruction 0x30 (1 cycle) 
    LCD_putcmd(0x30, LCD_1CYCLE);
    _delay_us(200); //Wait for more than 100 us

    //Send Command Instruction 0x30 (1 cycle)
    LCD_putcmd(0x30, LCD_1CYCLE);

    //Function set: Set interface to be 4 bits data long (only 1 cycle write)
    LCD_putcmd(0x20, LCD_1CYCLE);

    //Function set: DL=0;Interface is 4 bits data, N=1; 2 Lines, F=0; 5x8 dots font (2 cycle)
    LCD_putcmd(0x28, LCD_2CYCLE);

    //Display Off: D=0; Display off, C=0; Cursor Off, B=0; Blinking Off (2 cycle)
    LCD_putcmd(0x08, LCD_2CYCLE);

    //Display Clear (2 cycle)
    LCD_putcmd(0x01, LCD_2CYCLE);

    //Entry Mode Set: I/D=1; Increment, S=0; No shift (2 cycle)
    LCD_putcmd(0x06, LCD_2CYCLE);
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine sends string writes LCD
//------------------------------------------------------------------------------------------------------------------

void LCD_puts(char *s) //Implementation of C puts() function for the LCD, which display
//a terminated null string to the LCD
{
    while (*s != 0) //While not Null
    {
        if (*s == '\n')
            LCD_putcmd(LCD_NEXT_LINE, LCD_2CYCLE); //Goto Second Line
        else
            LCD_putch(*s);
        s++;
    }
}
//------------------------------------------------------------------------------------------------------------------
//	START I2C Routine via Software
//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
//	Subroutine transmit data to the I2C devices 
//------------------------------------------------------------------------------------------------------------------

unsigned char i2c_transmit(unsigned char type) //Function used to transmit data to the I2C devices and wait for
//transmission done by monitoring the interrupt flag bit (TWINT) 
{
    switch (type) {
        //Send Start Condition
    case I2C_START:
        TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
        break;

        //Send Data with No-Acknowledge (bit response)
    case I2C_DATA:
        TWCR = (1 << TWINT) | (1 << TWEN);
        break;

        //Send Data with Acknowledge (bit response)
    case I2C_DATA_ACK:
        TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN);
        break;

        //Send Stop Condition
    case I2C_STOP:
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
        return 0;
    }
    //Wait for TWINT flag set on Register TWCR
    while (!(TWCR & (1 << TWINT)));
    //Return TWI Status Register, mask the prescaller bits (TWPS1,TWPS0)
    return (TWSR & 0xF8); //TWI STATUS TWS3,TWS4,TWS5,TWS6,TWS7 = 1
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine used to send for the device I2C start condition
//------------------------------------------------------------------------------------------------------------------

char i2c_start(unsigned int dev_id, unsigned int dev_addr, unsigned char rw_type) //Function used to send the I2C
//start condition
{
    unsigned char n = 0;
    unsigned char twi_status;
    char r_val = -1;
i2c_retry:
    if (n++ >= MAX_TRIES) return r_val;

    //Transmit Start Condition
    twi_status = i2c_transmit(I2C_START);

    //Check the TWI Status
    if (twi_status == TW_MT_ARB_LOST) goto i2c_retry;
    if ((twi_status != TW_START) && (twi_status != TW_REP_START))
        goto i2c_quit;

    //Send slave address (SLA_W)
    TWDR = (dev_id & 0xF0) | (dev_addr & 0x07) | rw_type;

    // Transmit I2C Data
    twi_status = i2c_transmit(I2C_DATA);

    //Check the TWSR status
    if ((twi_status == TW_MT_SLA_NACK) || (twi_status == TW_MT_ARB_LOST)) goto i2c_retry;
    if (twi_status != TW_MT_SLA_ACK) goto i2c_quit;

    r_val = 0;

i2c_quit:
    return r_val;
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine used to send for the device I2C stop condition
//------------------------------------------------------------------------------------------------------------------

void i2c_stop(void) //Function used to send the I2C stop condition
{
    unsigned char twi_status;
    //Transmit I2C Data
    twi_status = i2c_transmit(I2C_STOP);
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine used to write data to the I2C
//------------------------------------------------------------------------------------------------------------------

char i2c_write(char data) //Function write data to the I2C slave device register
{
    unsigned char twi_status;
    char r_val = -1;

    //Send the Data to I2C Bus
    TWDR = data;

    //Transmit I2C Data
    twi_status = i2c_transmit(I2C_DATA);

    //Check the TWSR status
    if (twi_status != TW_MT_DATA_ACK) goto i2c_quit;

    r_val = 0;

i2c_quit:
    return r_val;
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine of multiple readings on the device I2C
//------------------------------------------------------------------------------------------------------------------

char i2c_read(char *data, char ack_type) //Function is used to read data from the I2C slave device register;
//the I2C master read operation could be selected to response with 
//ACK (acknowledge) or NACK (no-acknowledge);
//this feature is used in the multiple data reading as in the DS1307 RTC
{
    unsigned char twi_status;
    char r_val = -1;

    if (ack_type) {
        //Read I2C Data and Send Acknowledge
        twi_status = i2c_transmit(I2C_DATA_ACK);
        if (twi_status != TW_MR_DATA_ACK) goto i2c_quit;
    } else {
        //Read I2C Data and Send No Acknowledge
        twi_status = i2c_transmit(I2C_DATA);
        if (twi_status != TW_MR_DATA_NACK) goto i2c_quit;
    }

    //Get the Data
    *data = TWDR;

    r_val = 0;

i2c_quit:
    return r_val;
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine Convert Decimal to Binary Coded Decimal (BCD)
//------------------------------------------------------------------------------------------------------------------

char dec2bcd(char num) //Function is used to convert decimal to the binary code decimal value
{
    return ((num / 10 * 16) + (num % 10));
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine Convert Binary Coded Decimal (BCD) to Decimal
//------------------------------------------------------------------------------------------------------------------

char bcd2dec(char num) //Function is used to convert binary code decimal to the decimal value
{
    return ((num / 16 * 10) + (num % 16));
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine reading RTC
//------------------------------------------------------------------------------------------------------------------

void Read_DS1307(void) //Function is used to read the DS1307 RTC timekeeper registers					
{
    char data;
    //First we initial the pointer register to address 0x00
    //Start the I2C Write Transmission
    i2c_start(DS1307_ID, DS1307_ADDR, TW_WRITE);

    //Start from Address 0x00
    i2c_write(0x00);

    //Stop I2C Transmission
    i2c_stop();

    //Start the I2C Read Transmission
    i2c_start(DS1307_ID, DS1307_ADDR, TW_READ);

    //Read the Second Register, Send Master Acknowledge
    i2c_read(&data, ACK);
    ds1307_addr[0] = bcd2dec(data & 0x7F); //*Use Special Masking*

    //Read the Minute Register, Send Master Acknowledge
    i2c_read(&data, ACK);
    ds1307_addr[1] = bcd2dec(data);

    //Read the Hour Register, Send Master Acknowledge
    i2c_read(&data, ACK);
    if ((data & 0x40) == 0x40) {
        hour_mode = HOUR_12;
        ampm_mode = (data & 0x20) >> 5; //ampm_mode: 0-AM, 1-PM
        ds1307_addr[2] = bcd2dec(data & 0x1F); //*Use Special Masking*
    } else {
        hour_mode = HOUR_24;
        ampm_mode = 0;
        ds1307_addr[2] = bcd2dec(data & 0x3F); //*Use Special Masking*
    }

    //Read the Day of Week Register, Send Master Acknowledge
    i2c_read(&data, ACK);
    ds1307_addr[3] = bcd2dec(data);

    //Read the Day of Month Register, Send Master Acknowledge
    i2c_read(&data, ACK);
    ds1307_addr[4] = bcd2dec(data);

    //Read the Month Register, Send Master Acknowledge
    i2c_read(&data, ACK);
    ds1307_addr[5] = bcd2dec(data);

    //Read the Year Register, Send Master No Acknowledge
    i2c_read(&data, NACK);
    ds1307_addr[6] = bcd2dec(data);

    //Stop I2C Transmission
    i2c_stop();
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine Write RTC
//------------------------------------------------------------------------------------------------------------------

void Write_DS1307(void) //Function is used to write the DS1307 RTC timekeeper registers
{
    unsigned char i, hour_format;

    //Make sure we enable the Oscillator control bit CH=0 on Register 0x00
    ds1307_addr[0] = ds1307_addr[0] & 0x7F;

    //Start the I2C Write Transmission
    i2c_start(DS1307_ID, DS1307_ADDR, TW_WRITE);

    //Start from Address 0x00
    i2c_write(0x00);
    //Write the data to the DS1307 address start at 0x00
    //DS1307 automatically will increase the Address.
    for (i = 0; i < 7; i++) {
        if (i == 2) {
            hour_format = dec2bcd(ds1307_addr[i]);
            if (hour_mode) {
                hour_format |= (1 << 6);
                if (ampm_mode)
                    hour_format |= (1 << 5);
                else
                    hour_format &= ~(1 << 5);
            }
            else {
                hour_format &= ~(1 << 6);
            }
            i2c_write(hour_format);
        } else {
            i2c_write(dec2bcd(ds1307_addr[i]));
        }
    }
    //Stop I2C Transmission
    i2c_stop();
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine implementing integer value from 00 to 99
//------------------------------------------------------------------------------------------------------------------

char *num2str(char number) //Function is used to convert a numeric value to on the LCD
{
    unsigned char digit;
    digit = '0'; //Start with ASCII '0'
    while (number >= 10) //Keep Looping for larger than 10
    {
        digit++; //Increase ASCII character
        number -= 10; //Subtract number with 10
    }
    sdigit[0] = '0'; //Default first Digit to '0'
    if (digit != '0')
        sdigit[0] = digit; //Put the Second digit
    sdigit[1] = '0' + number;
    return sdigit;
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine HiperTerminal vs Setup
//------------------------------------------------------------------------------------------------------------------

char getnumber(unsigned char min, unsigned char max) //Function is used to validate the user input HiperTerminal
{
    int inumber;
    scanf("%d", &inumber); //Function scanf() to read the user input from the HyperTerminal
    if (inumber < min || inumber > max) {
        printf("\n\nInvalid [%d to %d]!", min, max);
        _delay_ms(500);
        return -1;
    }
    return inumber;
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine Control Interrupt RTC DS1307/LM35 vs LCD
//------------------------------------------------------------------------------------------------------------------
#if 0
ISR(TIMER0_OVF_vect) //Function TIMER0 interrupt service register function will be called every 10ms; and every time
//its being called the function will increase its static internal variable by one variable(tenms)
{
    static unsigned char tenms = 1;
    int iTemp;
    tenms++; //Read DS1307 every 100 x 10ms = 1 sec

    if (tenms >= 100) {
        cli(); //Disable Interupt
        //Read DS1307
        Read_DS1307();
        //Display the Clock
        LCD_putcmd(LCD_HOME, LCD_2CYCLE); //LCD Home
        LCD_puts(weekday[ds1307_addr[3] - 1]);
        LCD_puts(", ");
        LCD_puts(num2str(ds1307_addr[4]));
        LCD_puts(" ");
        LCD_puts(month[ds1307_addr[5] - 1]);
        LCD_puts(" ");
        LCD_puts("20");
        LCD_puts(num2str(ds1307_addr[6]));
        LCD_putcmd(LCD_NEXT_LINE, LCD_2CYCLE); //Goto Second Line
        if (hour_mode) {
            LCD_puts(num2str(ds1307_addr[2]));
            LCD_puts(":");
            LCD_puts(num2str(ds1307_addr[1]));
            LCD_puts(":");
            LCD_puts(num2str(ds1307_addr[0]));
            if (ampm_mode)
                LCD_puts(" PM");
            else
                LCD_puts(" AM");
        } else {
            LCD_puts(num2str(ds1307_addr[2]));
            LCD_puts(":");
            LCD_puts(num2str(ds1307_addr[1]));
            LCD_puts(":");
            LCD_puts(num2str(ds1307_addr[0]));
            LCD_puts("   ");
        }
        //Set ADMUX Channel for LM35 Input
        ADMUX = 0x01;
        //Start conversion by setting ADSC on ADCSRA Register
        ADCSRA |= (1 << ADSC);
        //wait until convertion complete ADSC=0 -> Complete
        while (ADCSRA & (1 << ADSC));
        //Get the ADC Result
        iTemp = ADCW;
        //ADC = (Vin x 1024) / Vref, Vref = 1 Volt, LM35 Out = 10mv/C
        iTemp = (int) (iTemp) / 10.24;

        //Dislay Temperature
        LCD_puts(" ");
        LCD_puts(num2str((char) iTemp)); //Display Temperature
        LCD_putch(0xDF); //Degree Character
        LCD_putch('C'); //Centigrade

        tenms = 1;
        sei(); //Enable Interrupt
    }

    TCNT0 = 0x94; //Use Clock Frequency of 11059200 Hz
    //TIMER0 clock frequency with 1024 prescaler = 11.059.200 Hz / 1024 = 10.800 Hz
    //TCNT0 overflow period = (256 - 148) / 10.800 Hz = 0.01 Second = 10 ms
}
#endif
//Assign I/O stream to UART
FILE uart_str = FDEV_SETUP_STREAM(uart_putch, uart_getch, _FDEV_SETUP_RW);
//------------------------------------------------------------------------------------------------------------------
//	Subroutine Control Port Used
//------------------------------------------------------------------------------------------------------------------

void 
main_setup_io(void)
{
    DDRB  = 0xFE; /* Set PB0=Input, Others Output */
    PORTB = 0;
    DDRC  = 0;    /* Set PORTC as Input */
    PORTC = 0;
    DDRD  = 0xFF; /* Set PORTD as Output */
    PORTD = 0;
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine Counter2/PWM Control Backlight LCD
//------------------------------------------------------------------------------------------------------------------

void Timer2_lcd(void)
{
    //Control Backlight LCD TIP120 (PB3)
    //Initial ATMega168 PWM using Timer/Counter2 Peripheral
    TCCR2A = 0b10000011; // Fast PWM Mode, Clear on OCRA
    TCCR2B = 0b00000100; // Used fclk/64 prescaller
    OCR2A = 0xFF; // Initial the OC2A (PB3) Out to 0xFF
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine Control Interrupt
//------------------------------------------------------------------------------------------------------------------

void Timer0_Interrupt(void)
{
    //Control used to read the slave RTC DS1307
    //Initial ATMega168 Timer/Counter0 Peripheral
    TCCR0A = 0x00; // Normal Timer0 Operation
    TCCR0B = (1 << CS02) | (1 << CS00); // Use maximum prescaller: Clk/1024
    TCNT0 = 0x94; // Start counter from 0x94, overflow at 10 mSec
    TIMSK0 = (1 << TOIE0); // Enable Counter Overflow Interrupt
    sei(); // Enable Interrupt
}

/******************************************************************************/
/**
 * 
 * @desc Subroutine check button enter Setup Mode
 */
static void 
main_button_read(void)
{
    //Check if Button is pressed than enter to the Setup Configuration Mode
    if (bit_is_clear(PINB, PB0)) { //if button is pressed
        _delay_us(100); //Wait for debouching
        //Check if Button is pressed than enter to the Setup Configuration Mode
        if (bit_is_clear(PINB, PB0)) { //if button is pressed
            mode = 1;
            cli(); //Disable Interrupt
            LCD_putcmd(LCD_CLEAR, LCD_2CYCLE); //Clear LCD
            LCD_puts("   Setup Mode    ");
            _delay_ms(1e2);
            LCD_putcmd(LCD_NEXT_LINE, LCD_2CYCLE); //Goto Second Line
            LCD_puts("  and Read RMS   ");

            //Dimmming the LCD
            for (icount = 255; icount > 0; icount--) {
                OCR2A = icount;
                _delay_ms(3);
            }
        }
    }
}
//------------------------------------------------------------------------------------------------------------------
//	Subroutine Enter Setup Mode and Write
//------------------------------------------------------------------------------------------------------------------

void SetupMode_Write(void)
{
    while (mode) {
        //if(mode)
        //{
        ansi_me(); //Function used to turn off all attribute on the ANSI terminal emulation	
        ansi_cl(); //Function clear screen
        ansi_cm(1, 1); //Function used to move cursor on the ANSI terminal emulation
        printf(" ** AVR 168 Printing and Adjust Data ** \n\n");
        ansi_cm(3, 1); //Function used to move cursor on the ANSI terminal emulation
        printf("1. Time: %02d:%02d:%02d\n", ds1307_addr[2], ds1307_addr[1], ds1307_addr[0]);
        printf("2. Mode 24/12: %d, AM/PM: %d\n", hour_mode, ampm_mode);
        printf("3. Date: %02d-%02d-20%02d, Week Day: %d\n", ds1307_addr[4], ds1307_addr[5], ds1307_addr[6], ds1307_addr[3]);
        printf("4. Save\n");
        printf("5. Read RMS V, I and exit \n");
        printf("6. Exit\n");
        printf("\nEnter Choice: ");

        if ((ichoice = getnumber(1, 6)) < 0) continue;
        switch (ichoice) {
        case 1: //RTC DS1307 Time Setup
            printf("\n\nHour [0-24]: ");
            if ((ds1307_addr[2] = getnumber(0, 24)) < 0) continue;
            printf("\nMinute [0-59]: ");
            if ((ds1307_addr[1] = getnumber(0, 59)) < 0) continue;
            printf("\nSecond [0-59]: ");
            if ((ds1307_addr[0] = getnumber(0, 59)) < 0) continue;
            break;

        case 2: //RTC DS1307 Hour Mode Setup
            printf("\n\nMode 0> 24, 1> 12: ");
            if ((hour_mode = getnumber(0, 1)) < 0) continue;
            printf("\nAM/PM 0> AM, 1> PM: ");
            if ((ampm_mode = getnumber(0, 1)) < 0) continue;
            break;

        case 3: //RTC DS1307 Date Setup
            printf("\n\nWeekDay [1-7]: ");
            if ((ds1307_addr[3] = getnumber(1, 7)) < 0) continue;
            printf("\nDate [1-31]: ");
            if ((ds1307_addr[4] = getnumber(1, 31)) < 0) continue;
            printf("\nMonth [1-12]: ");
            if ((ds1307_addr[5] = getnumber(1, 12)) < 0) continue;
            printf("\nYear [0-99]: ");
            if ((ds1307_addr[6] = getnumber(0, 99)) < 0) continue;
            break;

        case 4: //Save to DS1307 Register
            Write_DS1307(); //Function is used to write the DS1307 RTC timekeeper registers
            printf("\n");
            break;

        case 5: //Exit Setup
            printf("\nRead V:  \n");
            printf("Read I:  \n\n");
            //ansi_cl();				//Function clear screen
            break;

        case 6: //Exit Setup
            mode = 0; //Initial configuration mode
            ansi_cl(); //Function clear screen

            //Illuminating the LCD
            for (icount = 0; icount < 255; icount++) {
                OCR2A = icount;
                _delay_ms(3);
            }
            TCNT0 = 0x94; //TCNT0 overflow period = (256 - 148) / 10.800 Hz = 0.01 Second = 10 ms
            sei(); //Enable Interrupt
            break;
        }
    }
}

/******************************************************************************/

static uint8_t
main_led_blink_stop(uint8_t id)
{
    timer_source_remove(id);

    return false;
}

/******************************************************************************/

static uint8_t
main_led_blink(uint8_t led)
{
    LED_PORT ^= 1 << led;

    return true;
}

/******************************************************************************/

int
main(void)
{
    main_setup_io();

    //Define Output/Input Stream
    stdout = stdin = &(uart_str);

    //Initial LCD using 4 bits data interface
    initlcd();
    LCD_putcmd(0x0C, LCD_2CYCLE); //Display On, Cursor Off
    LCD_putcmd(LCD_CLEAR, LCD_2CYCLE); //Clear LCD

    //Initial ATMega168 UART Peripheral
    uart_init();

    //Initial ATMega168 TWI/I2C Peripheral
    TWSR = 0x00; // Select Prescaler of 1

    //SCL frequency = 11059200 / (16 + 2 * 47 * 1) = 98.743 khz
    TWBR = 0x30; // 48 Decimal

    //Control of the temperature sensor LM35 (PC1)
    //Initial ATMega168 ADC Peripheral
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);

    //Free running ADC Mode
    //ADCSRB = 0x00;

    //Disable digital input on ADC0 and ADC1
    DIDR0 = 0x0E;

    //Control Backlight LCD TIP120 (PB3)
    //Initial ATMega168 PWM using Timer/Counter2 Peripheral
    Timer2_lcd();

    //Initial Configuration Mode PORTB (PB0)
    mode = 0;

    timer_init();
    
    timer_timeout_add(1000, TIMER_HANDLE(main_led_blink),   TIMER_FUNC_DATA(LED1));
    timer_timeout_add(500,  TIMER_HANDLE(main_led_blink),   TIMER_FUNC_DATA(LED2));
    timer_timeout_add(250,  TIMER_HANDLE(main_led_blink),   TIMER_FUNC_DATA(LED3));
    timer_timeout_add(125,  TIMER_HANDLE(main_led_blink),   TIMER_FUNC_DATA(LED4));
    timer_timeout_add(100,  TIMER_HANDLE(main_button_read), NULL);
    timer_timeout_add(100,  TIMER_HANDLE(SetupMode_Write),  NULL);

    timer_loop_run();
    
    return 0;
}

/******************************************************************************/

#if 0
int main(void)
{
    //Initial PORT Used
    main_setup_io();

    //Define Output/Input Stream
    stdout = stdin = &uart_str;

    //Initial LCD using 4 bits data interface
    initlcd();
    LCD_putcmd(0x0C, LCD_2CYCLE); //Display On, Cursor Off
    LCD_putcmd(LCD_CLEAR, LCD_2CYCLE); //Clear LCD

    //Initial ATMega168 UART Peripheral
    uart_init();

    //Initial ATMega168 TWI/I2C Peripheral
    TWSR = 0x00; // Select Prescaler of 1

    //SCL frequency = 11059200 / (16 + 2 * 47 * 1) = 98.743 khz
    TWBR = 0x30; // 48 Decimal

    //Control of the temperature sensor LM35 (PC1)
    //Initial ATMega168 ADC Peripheral
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);

    //Free running ADC Mode
    //ADCSRB = 0x00;

    //Disable digital input on ADC0 and ADC1
    DIDR0 = 0x0E;

    //Control Backlight LCD TIP120 (PB3)
    //Initial ATMega168 PWM using Timer/Counter2 Peripheral
    Timer2_lcd();

    //Control used to read the slave RTC DS1307
    //Initial ATMega168 Timer/Counter0 Peripheral
    Timer0_Interrupt();

    //Initial Configuration Mode PORTB (PB0)
    mode = 0;

    for (;;) {
        main_button_read();
        SetupMode_Write();
    }
    return 0;
}
#endif
/*
//TESTES AD:
//---------

    // Read command on ADC0
    ADMUX &= ~_BV (MUX1);
    ADMUX &= ~_BV (MUX0);
    // Start one conversion.
    ADCSRA |= _BV (ADSC);
    // Wait until conversion is completed.
    while (ADCSRA & _BV (ADSC)){}
    // Get ADC the Result
	

    // Read onboard potentiometer on ADC1. 
    if (bit_is_clear (PINC,LEITURA))
    {
        ADMUX |= _BV (MUX0);
        ADMUX &= ~_BV (MUX1);
        // Start one conversion.
        ADCSRA |= _BV (ADSC);
        // Wait until conversion is completed.
        while (ADCSRA & _BV (ADSC)){}
        //trimpot = ADCW;
    }
	
    //Read feedback on ADC2
    ADMUX &= ~_BV (MUX0);
    ADMUX |= _BV (MUX1);
    //Start one conversion
    ADCSRA |= _BV (ADSC);
    // wait until conversion is completed
    while (ADCSRA & _BV (ADSC)){}
    // get converted value
    //feedback = ADCW;
    return(ADC);
    
//------------------------------------------------------------------------------------------------------------------
//	Programm Diego
//------------------------------------------------------------------------------------------------------------------

#include 	<avr/io.h>
#include	<util/delay.h>
#include 	<math.h>
#include 	<avr/interrupt.h>

//------------------------------------------------------------------//
// 						DEFINI��ES DE PORTAS						//
//------------------------------------------------------------------//

// BIBLIOTECA DE LEITURAS

volatile double SOMA_TENSAO, SOMA_CORRENTE;
volatile unsigned int numero_amostras;
volatile unsigned int TENSAO, CORRENTE;
volatile unsigned int tensao_de_ajuste;

#define FlagAD !(ADCSRA & (1<<ADIF)) //FLAG DE FIM DE CONVERS�O

#define		n		500
#define		n_fma	1.0/n

float ajuste = 0.488758553;

unsigned char bit[4] ;
//int BIT_CONVERTIDO;

void Inicializa_AD (void)
{

//	ADMUX	=	(1<<REFS0);												//ad2 PAg 121-122
    ADCSRA	=	(1<< ADEN)  |	(1<<ADPS1)	|	(1<<ADPS0);// |	(1<<ADPS2);
}

void converte_display(unsigned int aux)
{
    bit[0]=(( aux/1000)+0x30);
    bit[1]=(( aux%1000/100)+0x30);
    bit[2]=(((aux%1000)%100/10)+0x30); 
    bit[3]=(((aux%1000)%100%10)+0x30);
}

int Le_AD(unsigned char AD)
{
    int lixo;
	
    if (AD)
    {
        ADMUX	&= ~(1<<MUX0);
        ADMUX   |=  (1<<MUX1);
    }
    else
    {
        ADMUX	&= ~(1<<MUX1);
        ADMUX	|=  (1<<MUX0);
    }
	
    ADCSRA |= 1<<ADSC;
    while (FlagAD){};
    lixo= ADC;

    ADCSRA |= 1<<ADSC;
    while (FlagAD){};
    return ADC;
}

void inicializa_timer0(void)
{
    TCCR0B = (1 << CS00) | (1 << CS02);
    TIMSK0 = (1 << TOIE0);
    TCNT0 = 256 - 20;
}

void AD_lido(void)
{
    SOMA_TENSAO += square (fma(ajuste, Le_AD(1), 0));	//ajuste = 0.524791484
    SOMA_CORRENTE += square (fma(2.5, (Le_AD(0)-512), 0)); //ensaio pratico; 2.819
}

void mostrar_leitura(void)
{
    TENSAO = sqrt (fma (n_fma, SOMA_TENSAO, 0));
    CORRENTE = sqrt (fma (n_fma, SOMA_CORRENTE, 0));
}

void mostrar_display (void)
{
    converte_display(TENSAO);
    LCDGotoXY(12,2);
    LCDstring(bit,4);
    LCDGotoXY(11,2);
    LCDstring(" ",1);
    converte_display(CORRENTE);
    LCDGotoXY(12,3);
    LCDstring(bit,4);
    LCDGotoXY(11,3);
    LCDstring(bit,2);
    LCDGotoXY(13,3);
    LCDstring(".",1);
    LCDGotoXY(11,3);
    LCDstring(" ",1);
}


ISR(TIMER0_OVF_vect)
{
    TCNT0 = 256 - 20; //atulizando Timer  1ms de overflow
    if(numero_amostras == n)
    {
        mostrar_leitura();
        mostrar_display();
        numero_amostras=0;
        SOMA_TENSAO=0;
        SOMA_CORRENTE=0;
    }
    AD_lido();
    numero_amostras++;
}
 */
/*
void leitura_tensao(){
    a2dSetChannel(ADC_CH_ADC0); 
    entrada_ad = fma(0.48828125,a2dConvert10bit(ADC_CH_ADC0),0);
    valor_leitura_anterior = fma(valor_leitura_anterior,peso_2,fma(square(entrada_ad),peso,0));
    valor_final_rms = sqrt(valor_leitura_anterior);	
    }
 */

