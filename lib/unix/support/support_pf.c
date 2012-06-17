#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "support.h"
#include "../os/os.h"

 
int printf_buffer(int i)
{
	int j;
	if ( i <= 0 ) return 0;
	for ( j = 0; j < i; j++) supcon_putc(' ');
	return 0;
}


#define STATE_OFF	0
#define STATE_PAD	1

int 	support_vfprintf(FILE* stream, const char* format, va_list ap)
{
	int d, i;
	int index;
	char c,  *s;
	int modifier;
	int state;

	int off_length;
	int pad_length;

	modifier = 0;
	state    = STATE_OFF;
	
	off_length = 0;
	pad_length = 0;

	while (*format)
	{
	  switch(*format++) 
	  {
	     case 's':       
			if (modifier == 0) { supcon_putc(*(format-1)); break; }
			s = va_arg(ap, char*);
			printf_buffer( off_length - strlen(s) );
			supcon_puts(s);
			modifier = 0;
			off_length = 0;
			pad_length = 0;
			state = STATE_OFF;
			break;
	     case 'd':        
	     case 'i':        
			if (modifier == 0) { supcon_putc(*(format-1)); break; }
			i = va_arg(ap, int);
			printf_buffer( off_length - supcon_leni(i) - pad_length );
			for ( index = 0; index < pad_length - supcon_leni(i); index++)
			  supcon_putc( '0' );
		  
			supcon_puti(i);
			modifier = 0;
			off_length = 0;
			pad_length = 0;
			state = STATE_OFF;
			break;
	     case 'u':        
			if (modifier == 0) { supcon_putc(*(format-1)); break; }
			i = va_arg(ap, int);
			printf_buffer( off_length - supcon_lenui(i) - pad_length );
			for ( index = 0; index < pad_length - supcon_lenui(i); index++)
			  supcon_putc( '0' );
			  
			supcon_putui(i);
			modifier = 0;
			off_length = 0;
			pad_length = 0;
			state = STATE_OFF;
			break;
	     case 'p':         
			if (modifier == 0) { supcon_putc(*(format-1)); break; }
			d = va_arg(ap, int);
			printf_buffer( off_length - supcon_lenp(d) - pad_length );
			for ( index = 0; index < pad_length - supcon_lenp(d); index++ )
			   supcon_putc( '0' );
			   
			supcon_putp(d,'A');
			modifier = 0;
			off_length = 0;
			pad_length = 0;
			state = STATE_OFF;
			break;
	     case 'x':         
			if (modifier == 0) { supcon_putc(*(format-1)); break; }
			d = va_arg(ap, int);
			
			printf_buffer( off_length - supcon_lenx(d) - 2 );
			supcon_puts("0x");
			for ( index = 0; index < pad_length-supcon_lenx(d)-2; index++ )
			   supcon_putc( '0' );
	
			supcon_putx(d);
			modifier = 0;
			off_length = 0;
			pad_length = 0;
			state = STATE_OFF;
			break;
	     case 'l':         
			if (modifier == 0) { supcon_putc(*(format-1)); break; }
			break;
	     case 'X':         
			if (modifier == 0) { supcon_putc(*(format-1)); break; }
			d = va_arg(ap, int);
			printf_buffer( off_length - supcon_lenx(d) - 2 );
			supcon_puts("0x");
			for ( index = 0; index < pad_length-supcon_lenx(d)-2; index++ )
			   supcon_putc( '0' );
			   
			supcon_putX(d);
			
			modifier = 0;
			off_length = 0;
			pad_length = 0;
			state = STATE_OFF;
			break;
	     case 'c':          
			if (modifier == 0) { supcon_putc(*(format-1)); break; }
			c = (char) va_arg(ap, int);
			printf_buffer( off_length - 1 );
			supcon_putc(c);
			modifier = 0;
			off_length = 0;
			pad_length = 0;
			state = STATE_OFF;
			break;
	     case '%':
			modifier = 1;
			off_length = 0;
			pad_length = 0;
			state = STATE_OFF;
			break;
	     case '.':
			if (modifier == 0) { supcon_putc(*(format-1)); break; }
			if ( state == STATE_PAD ) 
			{
				modifier = 0;
				off_length = 0;
				pad_length = 0;
				state = STATE_OFF;
				break; 
			}
	
			state = STATE_PAD;
			break;
	     
	     case '0':
	     case '1':
	     case '2':
	     case '3':
	     case '4':
	     case '5':
	     case '6':
	     case '7':
	     case '8':
	     case '9':
	   		if ( modifier != 1 )
			{
			  supcon_putc( *(format-1) );
	  	  	  modifier = 0;
			  off_length = 0;
			  pad_length = 0;
			  state = STATE_OFF;
			  break;
			}
			if ( state == STATE_OFF )
			  off_length = off_length * 10 + ((*(format-1)) - '0');
			if ( state == STATE_PAD )
			  pad_length = pad_length * 10 + ((*(format-1)) - '0');
	     		break;

	     default:     
			printf_buffer( off_length - 1 );
			supcon_putc( *(format-1) );
			modifier = 0;
			off_length = 0;
			pad_length = 0;
			state = STATE_OFF;
			break;
	 }
	}
   return 0;
}

// ---------------------------------


int sprintf_buffer(char *s, int i)
{
	int j;
	if ( i <= 0 ) return 0;
	for ( j = 0; j < i; j++) strcat(s," ");
	return 0;
}


 
int 	support_vsprintf(char* buffer, const char* format, va_list ap)
{
	int d, i;
	int index;
	char c,  *s;
	int modifier;
	int state;

	int off_length;
	int pad_length;

	modifier = 0;
	state    = STATE_OFF;
	
	off_length = 0;
	pad_length = 0;

	buffer[0] = 0;

	while (*format)
	{
	  switch(*format++) 
	  {
	     case 's':       
		if (modifier == 0) { bufcon_putc( buffer, *(format-1)); break; }
		s = va_arg(ap, char*);
		sprintf_buffer( buffer, off_length - strlen(s) );
		bufcon_puts( buffer, s);
		modifier = 0;
		off_length = 0;
		pad_length = 0;
		state = STATE_OFF;
		break;
	     case 'd':        
	     case 'i':        
		if (modifier == 0) { bufcon_putc( buffer, *(format-1)); break; }
		i = va_arg(ap, int);
		sprintf_buffer( buffer, off_length - supcon_leni(i) - pad_length );
		for ( index = 0; index < pad_length - supcon_leni(i); index++)
		  bufcon_putc( buffer, '0' );
		  
		bufcon_puti(buffer, i);
		modifier = 0;
		off_length = 0;
		pad_length = 0;
		state = STATE_OFF;
		break;
	     case 'u':        
		if (modifier == 0) { bufcon_putc( buffer, *(format-1)); break; }
		i = va_arg(ap, int);
		sprintf_buffer( buffer, off_length - supcon_lenui(i) - pad_length );
		for ( index = 0; index < pad_length - supcon_lenui(i); index++)
		  bufcon_putc( buffer, '0' );
		  
		bufcon_putui( buffer, i);
		modifier = 0;
		off_length = 0;
		pad_length = 0;
		state = STATE_OFF;
		break;
	     case 'p':         
		if (modifier == 0) { bufcon_putc( buffer, *(format-1)); break; }
		d = va_arg(ap, int);
		sprintf_buffer( buffer, off_length - supcon_lenp(d) - pad_length );
		for ( index = 0; index < pad_length - supcon_lenp(d); index++ )
		   bufcon_putc( buffer, '0' );
		   
		bufcon_putp( buffer, d);
		modifier = 0;
		off_length = 0;
		pad_length = 0;
		state = STATE_OFF;
		break;

	     case 'l':         
			if (modifier == 0) { supcon_putc(*(format-1)); break; }
			break;

	     case 'x':         
		if (modifier == 0) { bufcon_putc( buffer, *(format-1)); break; }
		d = va_arg(ap, int);
		
		sprintf_buffer( buffer, off_length - supcon_lenx(d) - 2 );
		/*bufcon_puts(buffer,"0x");
		for ( index = 0; index < pad_length-supcon_lenx(d)-2; index++ )
		   bufcon_putc( buffer, '0' );*/

		bufcon_putx( buffer, d);
		modifier = 0;
		off_length = 0;
		pad_length = 0;
		state = STATE_OFF;
		break;
	     case 'X':         
		if (modifier == 0) { bufcon_putc( buffer, *(format-1)); break; }
		d = va_arg(ap, int);
		sprintf_buffer( buffer, off_length - supcon_lenx(d) - 2 );
		/*bufcon_puts(buffer,"0x");
		for ( index = 0; index < pad_length-supcon_lenx(d)-2; index++ )
		   bufcon_putc( buffer, '0' );*/
		   
		bufcon_putX(buffer,d);
		
		modifier = 0;
		off_length = 0;
		pad_length = 0;
		state = STATE_OFF;
		break;
	     case 'c':          
		if (modifier == 0) { bufcon_putc( buffer, *(format-1)); break; }
		c = (char) va_arg(ap, int);
		sprintf_buffer( buffer, off_length - 1 );
		bufcon_putc(buffer, c);
		modifier = 0;
		off_length = 0;
		pad_length = 0;
		state = STATE_OFF;
		break;
	     case '%':
		modifier = 1;
		off_length = 0;
		pad_length = 0;
		state = STATE_OFF;
		break;
	     case '.':
		if (modifier == 0) { bufcon_putc( buffer, *(format-1)); break; }
		if ( state == STATE_PAD ) 
		{
			modifier = 0;
			off_length = 0;
			pad_length = 0;
			state = STATE_OFF;
			break; 
		}

		state = STATE_PAD;
		break;
	     
	     case '0':
	     case '1':
	     case '2':
	     case '3':
	     case '4':
	     case '5':
	     case '6':
	     case '7':
	     case '8':
	     case '9':
	     		if ( modifier != 1 )
			{
			  bufcon_putc( buffer, *(format-1) );
	  	  	  modifier = 0;
			  off_length = 0;
			  pad_length = 0;
			  state = STATE_OFF;
			  break;
			}
			if ( state == STATE_OFF )
			  off_length = off_length * 10 + ((*(format-1)) - '0');
			if ( state == STATE_PAD )
			  pad_length = pad_length * 10 + ((*(format-1)) - '0');
	     		break;
	     
	     default:     
		sprintf_buffer( buffer, off_length - 1 );
		bufcon_putc( buffer, *(format-1) );
		modifier = 0;
		off_length = 0;
		pad_length = 0;
		state = STATE_OFF;
		break;
	 }
	}
   return 0;
}




 




