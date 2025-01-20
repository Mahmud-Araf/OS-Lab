#include <ustdio.h>
#include<kstring.h>
void uprintf(char *format,...)
{
    char *tr;
	uint32_t i;
	uint8_t *str;
	uint8_t *cur;  // Added declaration for cur
	va_list list;
	double dval;
    static char buffer[256];  // Static buffer to avoid stack issues
    int ptr = 0;
	//uint32_t *intval;
	va_start(list,format);
	for(tr = format; *tr != '\0' && ptr < 255; tr++) {
        if(*tr != '%') {
            buffer[ptr++] = *tr;
            continue;
        }
        
        tr++;
        switch (*tr)
        {
            case 'c':
                //Cast character into int
                i = va_arg(list,int);
                //Assign it to our built string
                buffer[ptr++] = (char)i;
                break;
            case 'd': {
                i = va_arg(list,int);
                if(i<0)
                {
                    //negative value
                    buffer[ptr++] = '-';
                    i=-i;				
                }
                //Convert to base 10
                //cast it into character array
                cur =  convert(i,10);
                for(int j = 0; cur[j] != '\0' && ptr < 255; j++) {
                    buffer[ptr++] = cur[j];
                }
                break;
            }
            case 'o': 
                i = va_arg(list,int);
                //Same code copy-paste
                if(i<0)
                {
                    //negative value
                    buffer[ptr++] = '-';
                    i=-i;				
                }
                //Convert to base 8
                //cast it into character array
                cur =  convert(i,8);
                for(int j = 0; cur[j] != '\0' && ptr < 255; j++) {
                    buffer[ptr++] = cur[j];
                }
                break;
            case 'x': 
                i = va_arg(list,int);
                if(i<0)
                {
                    //negative value
                    buffer[ptr++] = '-';
                    i=-i;				
                }
                //Convert to base 16
                //cast it into character array
                cur =  convert(i,16);
                for(int j = 0; cur[j] != '\0' && ptr < 255; j++) {
                    buffer[ptr++] = cur[j];
                }
                break;
            case 's': 
                //Direct assigning for string
                str = va_arg(list,uint8_t*);
                for(int j = 0; str[j] != '\0' && ptr < 255; j++) {
                    buffer[ptr++] = str[j];
                }
                break;
            case 'f': 
                dval = va_arg(list,double);
                cur = (uint8_t*)float2str(dval);
                for(int j = 0; cur[j] != '\0' && ptr < 255; j++) {
                    buffer[ptr++] = cur[j];
                }
                break;	
            default:
                break;
		}
	}
    buffer[ptr]=0;
    

	va_end(list);
    // Write the buffer
    uwrite(1, buffer);
}

void uscanf(char *format, ...) {
    va_list ap;
    va_start(ap, format);
    
    char buffer[256];
    char *input_ptr = buffer;
    
    // Read input from STDIN (fd = 0)
    uread(0, &input_ptr, 1);  // Read just one character
    
    // Process input based on format
    if (*format == '%') {
        format++;  // Skip the %
        switch (*format) {
            case 'd': {  // Integer
                int *d = va_arg(ap, int*);
                *d = *input_ptr - '0';  // Convert ASCII to integer
                break;
            }
            case 's': {  // String
                char *s = va_arg(ap, char*);
                *s = *input_ptr;
                *(s + 1) = '\0';
                break;
            }
            case 'c': {  // Character
                char *c = va_arg(ap, char*);
                *c = *input_ptr;
                break;
            }
        }
    }
    
    va_end(ap);
}
