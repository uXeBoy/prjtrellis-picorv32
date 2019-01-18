#include <stdint.h>

#define NULL 0

#define LED (*(volatile uint32_t*)0x02000000)

#define reg_uart_clkdiv (*(volatile uint32_t*)0x02000004)
#define reg_uart_data (*(volatile uint32_t*)0x02000008)

int getchar_available()
{
  return reg_uart_data & 0x100;
}

char getchar()
{
  return reg_uart_data & 0xFF;
}

void putchar(char c)
{
    if (c == '\n')
      putchar('\r');
    reg_uart_data = c;
}

void main(void)
{
	int c, cnt, pos, val, len;
	char *cp;
	void *base_addr = NULL;

        reg_uart_clkdiv = 217; // sets baudrate 115200 @ 25 MHz

	/* Appease gcc's uninitialized variable warnings */
	val = 0;

	cp = NULL;	/* shut up uninitialized use warnings */

prompt:
	pos = 0;

	c = 0x76720A0D;			/* "\r\nrv" */
	do {
		putchar(c);
		c >>= 8;
		if (c == 0 && pos == 0) {
			pos = -1;
			c = 0x203E3233;	/* "32> " */
		}
	} while (c != 0);

next:
	pos = -1;
	len = 255;
	cnt = 2;

loop:
	/* Blink LEDs while waiting for serial input */
	do {
		#if 0
		if (pos < 0) {
			// RDTSC(val);
			val++;
			if (val & 0x08000000)
				c = 0xff;
			else
				c = 0;
			if ((val & 0xff) > ((val >> 19) & 0xff))
				LED = c ^ 0x0f;
			else
				LED = c ^ 0xf0;
		} else
			LED = (int) cp >> 8;
		#endif
		c = reg_uart_data;
	} while ( (c & 0x100) == 0);
	// c = getchar();
	c &= 0xFF;
	LED = c;

	if (pos < 0) {
		if (c == 'S')
			pos = 0;
		else {
			if (c == '\r') /* CR ? */
				goto prompt;
			/* Echo char */
			if (c >= 32)
				putchar(c);
		}
		val = 0;
		goto loop;
	}
	if (c >= 10 && c <= 13) /* CR / LF ? */
		goto next;

	val <<= 4;
	if (c >= 'a')
		c -= 32;
	if (c >= 'A')
		val |= c - 'A' + 10;
	else
		val |= c - '0';
	pos++;

	/* Address width */
	if (pos == 1) {
		if (val >= 7 && val <= 9) {
			#if 0
			// overwrite first few bytes
			#if 0
			for(c = 0; c < 32; c++)
			{
			  ((char *)base_addr)[c] = c;
			}
			#endif
			// binary blink leds reading first few 
			// uploaded bytes before jumping to this new code
			for(c = 0; c < 32; c++)
			{
			  LED = ((char *)base_addr)[c];
			  for(int k = 0; k < 100000; k++)
			    asm("nop");
			}
			#endif
			
			// ((void (*)(void))base_addr)();
			
			#if 1
			__asm __volatile__(
			"li s0, 0x00002000;"	/* stack address */
			"mv ra, zero;"
			"jr %0;"
			: 
			: "r" (base_addr)
			);
			#endif
		}
		if (val <= 3)
			len = (val << 1) + 5;
		val = 0;
		goto loop;
	}

	/* Byte count */
	if (pos == 3) {
		cnt += (val << 1);
		val = 0;
		goto loop;
	}

	/* Valid len? */
	if (len < 6)
		goto loop;

	/* End of address */
	if (pos == len) {
		cp = (char *) val;
		if (base_addr == NULL)
			base_addr = (void *) val;
		goto loop;
	}

	if (pos > len && (pos & 1) && pos < cnt)
		*cp++ = val;

	goto loop;
	/* Unreached */
}
