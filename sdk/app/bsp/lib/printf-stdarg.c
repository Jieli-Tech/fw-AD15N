#if !defined(__USE_MASK_PRINTF)

#include "fcvt.h"
#include "typedef.h"
#include <stdarg.h>
/* #define __DEBUG */

static int (*printf_putchar)(int a) = NULL;

#define is_digit(c) ((c) >= '0' && (c) <= '9')

void register_handle_printf_putchar(int(*handle)(int a))
{
    printf_putchar = handle;
}

static int skip_atoi(const char **s)
{
    int i = 0;
    while (is_digit(** s)) {
        i = i * 10 + *((* s)++) - '0';
    }
    return i;
}

/* static char * flt( char ** str, double num, int size, int precision, char fmt, int flags ); */

static void printchar(char **str, int c)
{
    if (str) {
        **str = c;
        ++(*str);
    } else {
        //  putchar(c);
        if (printf_putchar) {
            printf_putchar(c);
        }
    }
}

void m_putchar(char c)
{
#ifdef __DEBUG
    if (printf_putchar) {
        printf_putchar(c);
    }
#endif
}

int m_puts(const char *p)
{
#ifdef __DEBUG
    while (*p) {
        m_putchar(*p++);
    }
#endif
    return 0;
}

void m_put_u4hex(u8 dat)
{
#ifdef __DEBUG
    dat = 0xf & dat;

    if (dat > 9) {
        m_putchar(dat - 10 + 'A');
    } else {
        m_putchar(dat + '0');
    }
#endif
}
void m_put_u32hex(u32 dat)
{
#ifdef __DEBUG
    m_putchar('0');
    m_putchar('x');
    m_put_u4hex(dat >> 28);
    m_put_u4hex(dat >> 24);

    m_put_u4hex(dat >> 20);
    m_put_u4hex(dat >> 16);

    m_put_u4hex(dat >> 12);
    m_put_u4hex(dat >> 8);

    m_put_u4hex(dat >> 4);
    m_put_u4hex(dat);
    m_putchar('\n');
#endif
}

void m_put_u16hex(u16 dat)
{
#ifdef __DEBUG
    m_put_u4hex(dat >> 12);
    m_put_u4hex(dat >> 8);

    m_put_u4hex(dat >> 4);
    m_put_u4hex(dat);
    m_putchar('\n');
#endif
}

void m_put_u8hex(u8 dat)
{
#ifdef __DEBUG
    m_put_u4hex(dat >> 4);
    m_put_u4hex(dat);
    m_putchar(' ');
#endif
}
void m_printf_buf(u8 *buf, u32 len)
{
#ifdef __DEBUG

    u32 i ;
    m_put_u32hex((u32)buf) ;
    m_putchar('\n') ;

    for (i = 0 ; i < len ; i++) {
        if ((i % 16) == 0) {
            m_putchar('\n') ;
        }

        m_put_u8hex(buf[i]) ;
    }
#endif
}

void m_put_buf(u8 *buf, u32 len)
{
    return m_printf_buf(buf, len);
}

#define PAD_RIGHT 1
#define PAD_ZERO 2
#define SIGN    4               // Unsigned/signed long
#define SPACE   8               // Space if plus
#define LEFT    16              // Left justified
#define SPECIAL 32              // 0x
#define LARGE   64              // Use 'ABCDEF' instead of 'abcdef'
#define PLUS    128               // Show plus

static int prints(char **out, const char *string, int width, int pad)
{
    register int pc = 0, padchar = ' ';

    if (width > 0) {
        register int len = 0;
        register const char *ptr;
        for (ptr = string; *ptr; ++ptr) {
            ++len;
        }
        if (len >= width) {
            width = 0;
        } else {
            width -= len;
        }
        if (pad & PAD_ZERO) {
            padchar = '0';
        }
    }
    if (!(pad & PAD_RIGHT)) {
        for (; width > 0; --width) {
            printchar(out, padchar);
            ++pc;
        }
    }
    for (; *string ; ++string) {
        printchar(out, *string);
        ++pc;
    }
    for (; width > 0; --width) {
        printchar(out, padchar);
        ++pc;
    }

    return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
    char print_buf[PRINT_BUF_LEN];
    register char *s;
    register int t, neg = 0, pc = 0;
    register unsigned int u = i;

    if (i == 0) {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return prints(out, print_buf, width, pad);
    }

    if (sg && b == 10 && i < 0) {
        neg = 1;
        u = -i;
    }

    s = print_buf + PRINT_BUF_LEN - 1;
    *s = '\0';

    while (u) {
        t = u % b;
        if (t >= 10) {
            t += letbase - '0' - 10;
        }
        *--s = t + '0';
        u /= b;
    }

    if (neg) {
        if (width && (pad & PAD_ZERO)) {
            printchar(out, '-');
            ++pc;
            --width;
        } else {
            *--s = '-';
        }
    }

    return pc + prints(out, s, width, pad);
}

static int print(char **out, const char *format, va_list args)
{
    register int width, pad, precision;
    register int pc = 0;
    char scr[2];

    for (; *format != 0; ++format) {
        if (*format == '%') {
            ++format;
            precision = width = pad = 0;
            if (*format == '\0') {
                break;
            }
            if (*format == '%') {
                goto _out;
            }
            if (*format == '-') {
                ++format;
                pad = PAD_RIGHT;
            }
            while (*format == '0') {
                ++format;
                pad |= PAD_ZERO;
            }
            // Get field width
            width = - 1;
            if (is_digit(* format)) {
                width = skip_atoi(& format);
            }

            precision = - 1;
            if (* format == '.') {
                ++ format;
                if (is_digit(* format)) {
                    precision = skip_atoi(& format);
                } else if (* format == '*') {
                    ++ format;
                    precision = va_arg(args, int);
                }
                if (precision < 0) {
                    precision = 0;
                }
            }




            if (*format == 's') {
                register char *s = (char *)va_arg(args, int);
                pc += prints(out, s ? s : "(null)", width, pad);
                continue;
            }
            if (*format == 'd') {
                pc += printi(out, va_arg(args, int), 10, 1, width, pad, 'a');
                continue;
            }
            if (*format == 'x') {
                pc += printi(out, va_arg(args, int), 16, 0, width, pad, 'a');
                continue;
            }
            if (*format == 'X') {
                pc += printi(out, va_arg(args, int), 16, 0, width, pad, 'A');
                continue;
            }
            if (*format == 'u') {
                pc += printi(out, va_arg(args, int), 10, 0, width, pad, 'a');
                continue;
            }
            if (*format == 'c') {
                /* char are converted to int then pushed on the stack */
                scr[0] = (char)va_arg(args, int);
                scr[1] = '\0';
                pc += prints(out, scr, width, pad);
                continue;
            }

#ifndef NOFLOAT
            if ((*format == 'E') || (*format == 'G') || (*format == 'e') || (*format == 'f') || (*format == 'g')) {
                pc += flt(out, va_arg(args, double), width, precision, *format, pad | SIGN);
                continue;


            }

#endif


        } else {
_out:
            printchar(out, *format);
            ++pc;
        }
    }
    if (out) {
        **out = '\0';
    }
    va_end(args);
    return pc;
}

int libprintf(const char *format, va_list args)
{
    return print(0, format, args);
}
int xprintf(const char *format, ...)
{
    if (printf_putchar == NULL) {
        return 0;
    }

    va_list args;

    va_start(args, format);
    return print(0, format, args);
}

int sprintf(char *out, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    return print(&out, format, args);
}



#ifndef NOFLOAT
static void cfltcvt(double value, char *buffer, char fmt, int precision)
{
    int decpt, sign, exp, pos;
    char *digits = NULL;
    char cvtbuf[CVTBUFSIZE + 1];
    int capexp = 0;
    int magnitude;

    if (fmt == 'G' || fmt == 'E') {
        capexp = 1;
        fmt += 'a' - 'A';
    }

    if (fmt == 'g') {
        digits = ecvtbuf(value, precision, & decpt, & sign, cvtbuf);
        magnitude = decpt - 1;
        if (magnitude < - 4  ||  magnitude > precision - 1) {
            fmt = 'e';
            precision -= 1;
        } else {
            fmt = 'f';
            precision -= decpt;
        }
    }

    if (fmt == 'e') {
        digits = ecvtbuf(value, precision + 1, & decpt, & sign, cvtbuf);

        if (sign) {
            * buffer ++ = '-';
        }
        * buffer ++ = * digits;
        if (precision > 0) {
            * buffer ++ = '.';
        }
        memcpy(buffer, digits + 1, precision);
        buffer += precision;
        * buffer ++ = capexp ? 'E' : 'e';

        if (decpt == 0) {
            if (value == 0.0) {
                exp = 0;
            } else {
                exp = - 1;
            }
        } else {
            exp = decpt - 1;
        }

        if (exp < 0) {
            * buffer ++ = '-';
            exp = - exp;
        } else {
            * buffer ++ = '+';
        }

        buffer[2] = (exp % 10) + '0';
        exp = exp / 10;
        buffer[1] = (exp % 10) + '0';
        exp = exp / 10;
        buffer[0] = (exp % 10) + '0';
        buffer += 3;
    } else if (fmt == 'f') {
        digits = fcvtbuf(value, precision, & decpt, & sign, cvtbuf);
        if (sign) {
            * buffer ++ = '-';
        }
        if (* digits) {
            if (decpt <= 0) {
                * buffer ++ = '0';
                * buffer ++ = '.';
                for (pos = 0; pos < - decpt; pos ++) {
                    * buffer ++ = '0';
                }
                while (* digits) {
                    * buffer ++ = * digits ++;
                }
            } else {
                pos = 0;
                while (* digits) {
                    if (pos ++ == decpt) {
                        * buffer ++ = '.';
                    }
                    * buffer ++ = * digits ++;
                }
            }
        } else {
            * buffer ++ = '0';
            if (precision > 0) {
                * buffer ++ = '.';
                for (pos = 0; pos < precision; pos ++) {
                    * buffer ++ = '0';
                }
            }
        }
    }

    * buffer = '\0';
}

static void forcdecpt(char *buffer)
{
    while (* buffer) {
        if (* buffer == '.') {
            return;
        }
        if (* buffer == 'e' || * buffer == 'E') {
            break;
        }
        buffer ++;
    }

    if (* buffer) {
        int n = strlen(buffer);
        while (n > 0) {
            buffer[n + 1] = buffer[n];
            n --;
        }

        * buffer = '.';
    } else {
        * buffer ++ = '.';
        * buffer = '\0';
    }
}

static void cropzeros(char *buffer)
{
    char *stop;

    while (* buffer && * buffer != '.') {
        buffer ++;
    }
    if (* buffer ++) {
        while (* buffer && * buffer != 'e' && * buffer != 'E') {
            buffer ++;
        }
        stop = buffer --;
        while (* buffer == '0') {
            buffer --;
        }
        if (* buffer == '.') {
            buffer --;
        }
        while (*++ buffer = * stop ++);
    }
}

static char *flt(char **str, double num, int size, int precision, char fmt, int flags)
{
    char tmp[80];
    char c, sign;
    int n, i;
    //printf("size = %d presisiion = %d \n",size ,precision) ;
    // Left align means no zero padding
    if (flags & LEFT) {
        flags &= ~ PAD_ZERO;
    }

    // Determine padding and sign char
    c = (flags & PAD_ZERO) ? '0' : ' ';
    sign = 0;
    if (flags & SIGN) {
        if (num < 0.0) {
            sign = '-';
            num = - num;
            size --;
        } else if (flags & PLUS) {
            sign = '+';
            size --;
        } else if (flags & SPACE) {
            sign = ' ';
            size --;
        }
    }

    // Compute the precision value
    if (precision < 0) {
        precision = 6;    // Default precision: 6
    } else if (precision == 0 && fmt == 'g') {
        precision = 1;    // ANSI specified
    }

    // Convert floating point number to text
    cfltcvt(num, tmp, fmt, precision);


    // '#' and precision == 0 means force a decimal point
    if ((flags & SPECIAL) && precision == 0) {
        forcdecpt(tmp);
    }

    // 'g' format means crop zero unless '#' given
    if (fmt == 'g' && !(flags & SPECIAL)) {
        cropzeros(tmp);
    }

    n = strlen(tmp);

    // Output number with alignment and padding
    size -= n;
    if (!(flags & (PAD_ZERO | LEFT))) while (size -- > 0) {
            printchar(str, ' ');    //* str ++ = ' ';
        }
    if (sign) {
        printchar(str, sign);    //* str ++ = sign;
    }
    if (!(flags & LEFT)) while (size -- > 0) {
            printchar(str, c);    //* str ++ = c;
        }
    for (i = 0; i < n; i ++) {
        printchar(str, tmp[i]);    //* str ++ = tmp[i];
    }
    while (size -- > 0) {
        printchar(str, ' ');    //* str ++ = ' ';
    }

    return str;
}

#endif
#endif

