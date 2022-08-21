#include "./s21_string.h"

static char specif[16] = "cdifsuxXgGeEopn%";

int isspecif(char c) {
    if (s21_strchr(specif, c) != s21_NULL)
        return 1;
    else
        return 0;
}

void add_char(char *str, char c) {
    int i = 0;
    while (str[i])
        i++;
    str[i] = c;
    str[i + 1] = '\0';
}

int islength(char c) {
    return (c == 'l' || c == 'h');
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

void init_struct(struct flags *flags) {
    flags->is_precision = 0;
    flags->precision = 0;
    flags->width = 0;
    flags->length = 0;
    flags->flagm = 0;
    flags->flagp = 0;
    flags->flags = 0;
    flags->flagz = 0;
    flags->flagsharp = 0;
    flags->ldouble = 0;
}

int getlen(long int dig) {
    int size = 0;
    if (dig == 0)
        size = 1;
    if (dig < 0)
        dig *= -1;
    while (dig > 0) {
        size++;
        dig /= 10;
    }
    return size;
}

// prefixes "-+ 0#"
void get_flags(char *str, struct flags *flags, int *i) {
    *i += 1;
    while (s21_strchr("-+ 0#", str[*i])) {
        if (str[*i] == '-')
            flags->flagm = 1;
        if (str[*i] == '+')
            flags->flagp = 1;
        if (str[*i] == ' ')
            flags->flags = 1;
        if (str[*i] == '0')
            flags->flagz = 1;
        if (str[*i] == '#')
            flags->flagsharp = 1;
        *i += 1;
    }
}

// getting flags, precision, etc
int parse_flags(char *str, int i, va_list vlist, struct flags *flags) {
    if (i + 1 < (int)s21_strlen(str) && !isspecif(str[i + 1])) {
        get_flags(str, flags, &i);
        while (!isspecif(str[i]) && i < (int)s21_strlen(str)) {
            if ((is_digit(str[i]) || str[i] == '*') && !flags->is_precision) {
                if (str[i] == '*') {
                    flags->width = va_arg(vlist, int);
                    if (flags->width < 0) {
                        flags->width *= -1;
                        flags->flagm = 1;
                    }
                } else {
                    while (!isspecif(str[i]) && str[i] && str[i] != '.' && !islength(str[i])
                    && is_digit(str[i])) {
                        flags->width = flags->width * 10 + str[i] - 48;
                        i++;
                    }
                }
            }
            if (str[i] == '.') {
                i++;
                flags->is_precision = 1;
                if (str[i] == '*') {
                    flags->precision = va_arg(vlist, int);
                    if (flags->precision < 0) {
                        flags->precision = 0;
                        flags->is_precision = 0;
                    }
                } else {
                    while (!isspecif(str[i]) && i < (int)s21_strlen(str) && !islength(str[i])
                    && is_digit(str[i])) {
                        flags->precision = flags->precision * 10 + str[i] - 48;
                        i++;
                    }
                }
            }
            if (str[i] == 'h' || str[i] == 'l')
                flags->length = str[i];
            if (str[i] == 'L')
                flags->ldouble = 1;
            if (!isspecif(str[i]) && i < (int)s21_strlen(str))
                i++;
        }
    }
    return i;
}

int len_of_int(struct flags *flags, int dig) {
    int len = getlen(dig);
    if (flags->precision > len)
        len = flags->precision;
    if (dig < 0)
        len++;
    if (flags->flagp && dig > 0) {
        flags->flags = 0;
        len++;
    }
    if (flags->flags && dig > 0 && !flags->flagp)
        len++;
    return len;
}

void terminate_str(char *str, struct flags *flags) {
    for (int i = 0; i < flags->index; i++)
        str[i] = '\0';
}

int s21_sprintf(char *restrict str, const char *restrict format, ...) {
    va_list vlist;
    va_start(vlist, format);
    struct flags *flags = (struct flags *)malloc(sizeof(struct flags));
    flags->index = 0;
    int ret = s21_parse(str, (char *)format, vlist, flags);
    va_end(vlist);
    if (ret != -1) {
        if (str != s21_NULL)
            ret = (int)s21_strlen(str);
    } else {
        if (str != s21_NULL)
            terminate_str(str, flags);
    }
    free(flags);
    return ret;
}

// cppcheck complains about (is_negative || (!is_negative && flags->flagp))
int dodge_cppcheck(int is_negative, struct flags *flags) {
    int ret = is_negative;
    if (!is_negative && flags->flagp)
        ret = !is_negative && flags->flagp;
    if (!is_negative && flags->flags)
        ret = !is_negative && flags->flags;
    return ret;
}

int close_p(struct flags *flags) {
    return (flags->error || flags->exit_sprintf);
}

// parser
int s21_parse(char *str, char *format, va_list vlist, struct flags *flags) {
    int flaglen;
    flags->error = 0;
    flags->exit_sprintf = 0;
    init_struct(flags);
    for (int i = 0; i < (int)s21_strlen(format) && !close_p(flags); i++) {
        if (i < (int)s21_strlen(format) && format[i] == '%') {
            init_struct(flags);
            flaglen = parse_flags(format, i, vlist, flags);  // flags data
            i = i < flaglen ? flaglen : i + 1;
            if (format[i] == 'd' || format[i] == 'i') s21_parse_d(str, vlist, flags);
            if (format[i] == 'c') s21_parse_c(str, vlist, flags);
            if (format[i] == 's') s21_parse_s(str, vlist, flags);
            if (format[i] == '%') s21_parse_pr(str, flags);
            if (format[i] == 'u') s21_parse_u(str, vlist, flags);
            if (format[i] == 'f') s21_parse_f(str, vlist, flags);
            if (format[i] == 'x' || format[i] == 'X') s21_parse_xo(str, vlist, flags, format[i], 16);
            if (format[i] == 'o') s21_parse_xo(str, vlist, flags, format[i], 8);
            if (format[i] == 'p') s21_parse_p(str, vlist, flags, format[i], 16);
            if (format[i] == 'n') s21_parse_n(vlist, flags);
            if (format[i] == 'e' || format[i] == 'E') s21_parse_e(str, vlist, flags, format[i]);
        } else {
            str[flags->index++] = format[i];
        }
    }
    str[flags->index++] = '\0';
    int ret = flags->error;
    return ret;
}

void s21_parse_pr(char *str, struct flags *flags) {
    str[flags->index++] = '%';
    str[flags->index] = '\0';
}

void s21_wcharcat(char *destination, const wchar_t *source, int size, struct flags *flags) {
    int i = 0;
    while (i < size && source[i] != '\0') {
        destination[flags->index++] = source[i];
        i++;
    }
    destination[flags->index] = '\0';
}

void s21_parse_c(char *str, va_list vlist, struct flags *flags) {
    int i = 0;
    char c = '\n';
    wchar_t tmp = 12;
    if (!flags->length)
        c = va_arg(vlist, int);
    else
        tmp = va_arg(vlist, wchar_t);
    if (flags->width && !flags->flagm && (!flags->length || tmp <= 255)) {
        while (i < flags->width - 1) {
            str[flags->index++] = ' ';
            i++;
        }
    }
    if (!flags->length) {
        str[flags->index++] = c;
    } else {
        wchar_t s[2] = {0};
        if ((int)tmp > 255) {
            tmp = 0;
            flags->error = 1;
        }
        s[0] = tmp;
        s21_wcharcat(str, s, 2, flags);
    }
    if (c == '\0' || tmp == '\0') {
        flags->exit_sprintf = -1;
    } else if (flags->width && flags->flagm && (!flags->length || tmp <= 255)) {
        while (i < flags->width - 1) {
            str[flags->index++] = ' ';
            i++;
        }
    }
    str[flags->index] = '\0';
}

void parse_hl(char *str, va_list vlist, struct flags *flags, char *digit, char spec) {
    long int dig, is_negative = 0;
    if (spec == 'h')
        dig = (short int)va_arg(vlist, int);
    else
        dig = va_arg(vlist, long int);
    int len = len_of_int(flags, dig);
    if (!flags->flagm && !flags->flagz)
        add_spaces(str, flags, len);
    if (flags->flagp && dig > 0)
        str[flags->index++] = '+';
    if (dig < 0) {
        str[flags->index++] = '-';
        dig *= -1;
        is_negative = 1;
    }
    s21_itoa(dig, digit);
    if (flags->flagz && flags->flags && (int)s21_strlen(digit) > 0 && !is_negative)
        str[flags->index++] = ' ';
    if (!flags->flagm && flags->flagz)
        add_spaces(str, flags, len);
    if (flags->flags && (int)s21_strlen(digit) > 0 && !is_negative && !flags->flagz)
        str[flags->index++] = ' ';
    int size = (int)s21_strlen(digit);
    while (size < flags->precision) {
        size++;
        str[flags->index++] = '0';
    }
    for (s21_size_t i = 0; i < s21_strlen(digit); i++)
        str[flags->index++] = digit[i];
    if (flags->flagm)
        add_spaces(str, flags, len);
    str[flags->index] = '\0';
}

void s21_parse_d(char *str, va_list vlist, struct flags *flags) {
    char digit[20] = "";
    if (flags->length == 'h') {
        parse_hl(str, vlist, flags, digit, 'h');
    } else if (flags->length == 'l') {
        parse_hl(str, vlist, flags, digit, 'l');
    } else {
        actual_d(str, vlist, flags, digit);
    }
}

void actual_d(char *str, va_list vlist, struct flags *flags, char *digit) {
    int dig = va_arg(vlist, int), is_negative = 0;
    int len = len_of_int(flags, dig);
    if (!flags->flagm && !flags->flagz)
        add_spaces(str, flags, len);
    if (dig < 0 && dig != -2147483648) {
        dig *= -1;
        is_negative = 1;
    }
    if (dig == -2147483648) {
        is_negative = 1;
        s21_strcat(digit, "2147483648");
    } else {
        s21_itoa(dig, digit);
    }
    if (flags->flags && flags->flagz && (int)s21_strlen(digit) > 0 && !is_negative)
        str[flags->index++] = ' ';
    if (flags->flagz && len < flags->width && is_negative)
        str[flags->index++] = '-';
    if (flags->flagp && dig >= 0 && flags->flagz && len < flags->width && !is_negative)
        str[flags->index++] = '+';
    if (!flags->flagm && flags->flagz)
        add_spaces(str, flags, len);
    if (flags->flags && (int)s21_strlen(digit) > 0 && !is_negative && !flags->flagz)
        str[flags->index++] = ' ';
    int size = (int)s21_strlen(digit);
    if (flags->precision >= size && is_negative)
        str[flags->index++] = '-';
    if (flags->precision >= size && flags->flagp && dig >= 0 && !is_negative)
        str[flags->index++] = '+';
    while (size < flags->precision) {
        size++;
        str[flags->index++] = '0';
    }
    if (flags->precision < size && !flags->flagz && is_negative)
        str[flags->index++] = '-';
    if (flags->precision < size && !flags->flagz && flags->flagp && dig >= 0 && !is_negative)
        str[flags->index++] = '+';
    if (!flags->is_precision && !flags->flagm && (flags->flagz && flags->width <= (int)s21_strlen(digit)) &&
    ((int)s21_strlen(digit) >= flags->precision && flags->flagp && !is_negative))
        str[flags->index++] = '+';
    if (!flags->is_precision && !flags->flagm && (flags->flagz && flags->width <= (int)s21_strlen(digit)) &&
    ((int)s21_strlen(digit) >= flags->precision && is_negative))
        str[flags->index++] = '-';
    for (s21_size_t i = 0; i < s21_strlen(digit); i++)
        str[flags->index++] = digit[i];
    if (flags->flagm)
        add_spaces(str, flags, len);
    str[flags->index] = '\0';
}

void ls_case(char *str, va_list vlist, struct flags *flags) {
    wchar_t *res = va_arg(vlist, wchar_t*);
    for (int i = 0; res != s21_NULL && res[i] != '\0'; i++) {
        if (res[i] < 0 || res[i] > 255) {
            flags->error = -1;
            return;
        }
    }
    int size = 0, tmp_size;
    while (res != s21_NULL && res[size] != '\0')
        size++;
    if (res == s21_NULL)
        size = 6;
    if (flags->is_precision && flags->precision < size)
        size = flags->precision;
    tmp_size = size;
    if (!flags->flagm)
        add_spaces(str, flags, tmp_size);
    if (res != s21_NULL) {
        s21_wcharcat(str, res, size, flags);
    } else {
        char null_s[7] = "";
        s21_strncpy(null_s, "(null)", size + 1);
        for (s21_size_t i = 0; i < s21_strlen(null_s); i++)
            str[flags->index++] = null_s[i];
    }
    if (flags->flagm)
        add_spaces(str, flags, tmp_size);
    str[flags->index] = '\0';
}

void s21_parse_s(char *str, va_list vlist, struct flags *flags) {
    if (flags->length) {
        ls_case(str, vlist, flags);
        return;
    }
    char *new;
    new = va_arg(vlist, char *);
    int len;
    if (flags->is_precision && new != s21_NULL) {
        len = flags->precision <= (int)s21_strlen(new) ? flags->precision : (int)s21_strlen(new);
    } else if (new != s21_NULL) {
        len = s21_strlen(new);
    } else {
        len = flags->precision <= 6 ? flags->precision : 6;
    }
    char *result_string = (char *)malloc((len * sizeof(char)) + 1);
    for (int i = 0; new != s21_NULL && i < len; i++)
        result_string[i] = new[i];
    if (new == s21_NULL)
        s21_strncpy(result_string, "(null)", len + 1);
    result_string[len] = '\0';
    if (flags->flagm) {
        for (s21_size_t i = 0; i < s21_strlen(result_string); i++)
            str[flags->index++] = result_string[i];
        add_spaces(str, flags, len);
    } else {
        add_spaces(str, flags, len);
        for (s21_size_t i = 0; i < s21_strlen(result_string); i++)
            str[flags->index++] = result_string[i];
    }
    free(result_string);
    str[flags->index] = '\0';
}

void s21_utoa(unsigned long int m, char *str) {
    unsigned long int power = 0, j = 0, i = 0;
    j = m;
    for (power = 1; j >= 10; j /= 10)
        power *= 10;
    for (; power > 0; power /= 10) {
        str[i++] = '0' + m / power;
        m %= power;
    }
    str[i] = '\0';
}

void s21_parse_u(char *str, va_list vlist, struct flags *flags) {
    char digit[30] = "";
    unsigned long int dig;
    if (flags->length == 'h') {
        dig = (unsigned short int)va_arg(vlist, int);
    } else if (flags->length == 'l') {
        dig = va_arg(vlist, unsigned long int);
    } else {
        dig = va_arg(vlist, unsigned int);
    }
    s21_utoa(dig, digit);
    int diff;
    int size = (int)s21_strlen(digit);
    diff = flags->precision > size ? flags->precision : size;
    if (!flags->flagm)
        add_spaces(str, flags, diff);
    while (flags->precision > size) {
        str[flags->index++] = '0';
        size++;
    }
    for (s21_size_t i = 0; i < s21_strlen(digit); i++)
        str[flags->index++] = digit[i];
    if (flags->flagm)
        add_spaces(str, flags, diff);
    str[flags->index] = '\0';
}

int check_sign(double *f, int *str_len) {
    int is_negative = 0;
    if (*f < 0) {
        is_negative = 1;
        *f *= -1;
        *str_len += 1;
    }
    return is_negative;
}

int rounding(char *fl_s, int size, int last_number, struct flags *flags) {
    int first = 1;
    if (flags->is_precision && size > flags->precision + 1)
        size = flags->precision;
    if (!flags->is_precision && size > 6)
        size = 6;
    while (last_number > 4 && size >= 0 && (int)s21_strlen(fl_s) > flags->precision) {
        if (size < (int)s21_strlen(fl_s) && first && fl_s[size] <= '4') {
            size++;
            break;
        }
        fl_s[size] += 1;
        if (size < (int)s21_strlen(fl_s) &&
        ((flags->is_precision && size <= flags->precision) || (!flags->is_precision && size <= 6))
        && fl_s[size] < '5') {
            break;
        } else {
            size--;
            if (size < (int)s21_strlen(fl_s) && fl_s[size + 1] == ':' && size + 1 != 0)
                fl_s[size + 1] = '0';
            else if ((flags->is_precision && size + 1 < flags->precision) ||
            (!flags->is_precision && size + 1 < 6))
                break;
        }
        first = 0;
    }
    return size;
}

unsigned long long int get_num_of_digits(struct flags *flags, char *fl_s, unsigned long long int integer,
unsigned long long int *in_fl, double fl) {
    double pow = flags->precision > 7 ? flags->precision : 7.0;
    unsigned long int mn = 1, zeros = 0;
    while (pow-- > 0)
        mn *= 1E1;
    *in_fl = round(fl * mn), integer = *in_fl;
    for (int i = 0; i < 6 - flags->precision && flags->is_precision && *in_fl > 0; i++)
        *in_fl /= 10;
    s21_itoa(*in_fl, fl_s);
    while (integer * 10 / mn < 1 && integer != 0) {
        integer *= 10;
        zeros++;
    }
    return zeros;
}

void fill_zeroarr(int zeros, char *zero_arr, char *fl_s) {
    for (int i = 0; i < (int)zeros; i++)
        add_char(zero_arr, '0');
    int i = 0;
    while (zero_arr[i])
        i++;
    for (int j = 0; fl_s[j]; j++) {
        if (fl_s[j] == ':')
            zero_arr[i] = '0';
        else
            zero_arr[i] = fl_s[j];
        i++;
    }
    s21_strcat(zero_arr, fl_s);
}

int check_last_num(char *fl_s, struct flags *flags, int is_negative) {
    int first_num = 0;
    if (fl_s[0] > '4' && flags->is_precision && flags->precision == 0 && !is_negative) {
        first_num++;
    } else if (fl_s[0] == ':') {
        fl_s[0] = '0';
        first_num++;
    }
    return first_num;
}

int fill_result_arr1(struct flags *flags, unsigned long long int first_num, char *in_s,
char *result) {
    s21_itoa(first_num, in_s);
    s21_strcat(result, in_s);
    if (!flags->is_precision || (flags->precision == 0 && flags->flagsharp) || flags->precision > 0)
        add_char(result, '.');
    int len = !flags->is_precision ? 6 : flags->precision;
    return len;
}

void zero_f(char *str, struct flags *flags) {
    if (!flags->is_precision && flags->precision == 0)
        flags->precision = 6;
    int size_of_spaces = flags->precision + 1;
    if (!(!flags->flagsharp && flags->is_precision && flags->precision == 0))
        size_of_spaces++;
    if (flags->flagp && !(flags->flagz && size_of_spaces + 1 < flags->width))
        size_of_spaces++;
    if (flags->flagp && flags->flagz)
        str[flags->index++] = '+';
    if ((size_of_spaces < flags->width || flags->flagm) && flags->flags && !flags->flagp
    && flags->flagz)
        str[flags->index++] = ' ';
    if (!flags->flagm)
        add_spaces(str, flags, size_of_spaces);
    if (flags->flagp && !flags->flagz)
        str[flags->index++] = '+';
    if ((size_of_spaces < flags->width || flags->flagm) && flags->flags && !flags->flagp
    && !flags->flagz)
        str[flags->index++] = ' ';
    if (flags->width == 0 && flags->flags)
        str[flags->index++] = ' ';
    if (!flags->flagz)
        str[flags->index++] = '0';
    if (!(flags->is_precision && flags->precision == 0 && !flags->flagsharp))
        str[flags->index++] = '.';
    for (int i = 0; i < flags->precision; i++)
        str[flags->index++] = '0';
    if (flags->flags)
        size_of_spaces++;
    if (flags->flagm)
        add_spaces(str, flags, size_of_spaces);
    str[flags->index] = '\0';
}

int check_signl(long double *f, int *str_len) {
    int is_negative = 0;
    if (*f < 0) {
        is_negative = 1;
        *f *= -1;
        *str_len += 1;
    }
    return is_negative;
}

unsigned long long int get_num_of_digitsl(struct flags *flags, char *fl_s, unsigned long long int integer,
unsigned long long int *in_fl, long double fl) {
    long double pow = flags->precision > 7 ? flags->precision : 7.0;
    unsigned long long int mn = 1, zeros = 0;
    while (pow-- > 0)
        mn *= 1E1;
    *in_fl = round(fl * mn), integer = *in_fl;
    for (int i = 0; i < 6 - flags->precision && flags->is_precision && *in_fl > 0; i++)
        *in_fl /= 10;
    s21_itoa(*in_fl, fl_s);
    while (integer * 10 / mn < 1 && integer != 0) {
        integer *= 10;
        zeros++;
    }
    return zeros;
}

void lf_case(char *str, va_list vlist, struct flags *flags) {
    long double f = va_arg(vlist, long double);
    if (f == 0.0) {
        zero_f(str, flags);
    } else {
        int str_len = 0, is_negative = check_signl(&f, &str_len);
        unsigned long long int integer = (unsigned long long int)f, in_fl = 0, first_num = integer, zeros = 0;
        long double fl = f - integer;
        char in_s[12] = "", fl_s[20] = "", zero_arr[20] = "", result[100] = "";
        zeros = get_num_of_digitsl(flags, fl_s, integer, &in_fl, fl);
        fill_zeroarr(zeros, zero_arr, fl_s);
        rounding(zero_arr, s21_strlen(zero_arr) - 1, in_fl % 10, flags);
        first_num += check_last_num(zero_arr, flags, is_negative);
        int len = fill_result_arr1(flags, first_num, in_s, result);
        for (int size = 1; s21_strlen(zero_arr) == 1 && *zero_arr == '0' && size < len; size++)
            add_char(result, '0');
        if (!flags->is_precision || flags->precision != 0)
            s21_strcat(result, zero_arr);
        if (!flags->is_precision)
            flags->precision = 6;
        for (int i = 0; flags->precision > 0 && zeros-- > 0; i++)
            add_char(result, zero_arr[i]);
        result[flags->precision + s21_strlen(in_s) + 1] = '\0';
        if (flags->flagz && is_negative)
            str[flags->index++] = '-';
        if (flags->flagz && flags->flagp && !is_negative)
            str[flags->index++] = '+';
        if (flags->flagz && flags->flags && !is_negative)
            str[flags->index++] = ' ';
        if (!flags->flagm)
            add_spaces(str, flags, s21_strlen(result) + dodge_cppcheck(is_negative, flags));
        if (!flags->flagz && is_negative)
            str[flags->index++] = '-';
        if (!flags->flagz && flags->flagp && !is_negative)
            str[flags->index++] = '+';
        if (!flags->flagz && flags->flags && !is_negative)
            str[flags->index++] = ' ';
        if (flags->flagm) {
            for (s21_size_t i = 0; i < s21_strlen(result); i++)
                str[flags->index++] = result[i];
            add_spaces(str, flags, s21_strlen(result) + dodge_cppcheck(is_negative, flags));
        } else {
            for (s21_size_t i = 0; i < s21_strlen(result); i++)
                str[flags->index++] = result[i];
        }
    }
    str[flags->index] = '\0';
}

void s21_parse_f(char *str, va_list vlist, struct flags *flags) {
    if (flags->ldouble) {
        lf_case(str, vlist, flags);
        return;
    }
    double f = va_arg(vlist, double);
    if (f == 0.0) {
        zero_f(str, flags);
    } else {
        int str_len = 0, is_negative = check_sign(&f, &str_len);
        unsigned long long int integer = (unsigned long long int)f, in_fl = 0, first_num = integer, zeros = 0;
        double fl = f - integer;
        char in_s[12] = "", fl_s[20] = "", zero_arr[20] = "", result[100] = "";
        zeros = get_num_of_digits(flags, fl_s, integer, &in_fl, fl);
        fill_zeroarr(zeros, zero_arr, fl_s);
        rounding(zero_arr, s21_strlen(zero_arr) - 1, in_fl % 10, flags);
        first_num += check_last_num(zero_arr, flags, is_negative);
        int len = fill_result_arr1(flags, first_num, in_s, result);
        for (int size = 1; s21_strlen(zero_arr) == 1 && *zero_arr == '0' && size < len; size++)
            add_char(result, '0');
        if (!flags->is_precision || flags->precision != 0)
            s21_strcat(result, zero_arr);
        if (!flags->is_precision)
            flags->precision = 6;
        int i = 0;
        for (; flags->precision > 0 && zeros-- > 0; i++) {
            add_char(result, zero_arr[i]);
        }
        for (; i < flags->precision; i++)
            add_char(result, '0');
        result[flags->precision + s21_strlen(in_s) + 1] = '\0';
        if (flags->flagz && is_negative)
            str[flags->index++] = '-';
        if (flags->flagz && flags->flagp && !is_negative)
            str[flags->index++] = '+';
        if (flags->flagz && flags->flags && !is_negative)
            str[flags->index++] = ' ';
        if (!flags->flagm)
            add_spaces(str, flags, s21_strlen(result) + dodge_cppcheck(is_negative, flags));
        if (!flags->flagz && is_negative)
            str[flags->index++] = '-';
        if (!flags->flagz && flags->flagp && !is_negative)
            str[flags->index++] = '+';
        if (!flags->flagz && flags->flags && !is_negative)
            str[flags->index++] = ' ';
        if (flags->flagm) {
            for (s21_size_t in = 0; in < s21_strlen(result); in++)
                str[flags->index++] = result[in];
            add_spaces(str, flags, s21_strlen(result) + dodge_cppcheck(is_negative, flags));
        } else {
            for (s21_size_t in = 0; in < s21_strlen(result); in++)
                str[flags->index++] = result[in];
        }
    }
    str[flags->index] = '\0';
}

int radtoa(char *digit, unsigned long int dig, char format, int radix, int i) {
    if (dig == 0) {
        digit[i] = '0';
        i += 1;
    }
    while (dig > 0 && radix != 0) {
        if (dig % (unsigned)radix >= 10) {
            if (format == 'x' || format == 'p')
                digit[i] = dig % (unsigned)radix + 87;
            else
                digit[i] = dig % (unsigned)radix + 55;
        } else {
            digit[i] = dig % (unsigned)radix + 48;
        }
        dig /= (unsigned)radix;
        i += 1;
    }
    return i;
}

unsigned long int get_type(struct flags *flags, va_list vlist, char format) {
    unsigned long int digit;
    if (flags->length == 'h') {
        digit = (unsigned short int)va_arg(vlist, int);
    } else if (flags->length == 'l') {
        digit = va_arg(vlist, unsigned long int);
    } else if (format == 'p') {
        digit = (unsigned long int)va_arg(vlist, void *);
    } else {
        digit = (long unsigned int)va_arg(vlist, unsigned int);
    }
    return digit;
}

int check_flags(struct flags *flags, char format, char *str, unsigned long int dig, int *size) {
    int id = 0;
    int diff = flags->precision > *size ? flags->precision : *size;
    if (format != 'o' && flags->flagsharp)
        diff += 1;
    if (!flags->flagm && !flags->flagz) {
        for (int i = 0; i < flags->width - diff - flags->flagsharp; i++) {
            str[id++] = ' ';
        }
    }
    int put_x = 0;
    if (!flags->flagm && flags->flagz) {
        for (int i = 0; i < flags->width - diff - flags->flagsharp; i++) {
            if (put_x == 1 && format != 'o')
                str[id++] = format;
            str[id++] = '0';
            put_x++;
        }
    }
    if (flags->flagsharp && !flags->is_precision && dig != 0) {
        put_x++;
        str[id++] = '0';
    }
    if (flags->flagsharp && flags->is_precision && dig != 0) {
        str[id++] = '0';
        *size += 1;
        put_x++;
    }
    if (put_x == 1 && format != 'o')
        str[id++] = format;
    if ((format == 'x' || format == 'X') && flags->flagsharp && dig != 0) {
        *size -= 1;
    }
    str[id] = '\0';
    return diff;
}

void x_equals_0_case(char *str, struct flags *flags) {
    if (!flags->flagm)
        add_spaces(str, flags, 1);
    if (!(flags->is_precision && flags->precision == 0)) {
        str[flags->index++] = '0';
    }
    if (flags->flagm)
        add_spaces(str, flags, 1);
    str[flags->index] = '\0';
}

void s21_parse_xo(char *str, va_list vlist, struct flags *flags, char format, int radix) {
    char digit[100] = "";
    char reverse[100] = "";
    unsigned long int dig = get_type(flags, vlist, format);
    if (dig == 0) {
        x_equals_0_case(str, flags);
        return;
    }
    char buf[300] = "";
    s21_utoa(dig, digit);
    int i = 0, j = 0;
    i = radtoa(digit, dig, format, radix, i);
    int diff, size = i;
    diff = check_flags(flags, format, buf, dig, &size);
    int put_x = 0;
    while (flags->is_precision && flags->precision > size) {
        if (!flags->flagz && put_x == 1 && format != 'o' && flags->flagsharp && s21_strlen(buf) == 1)
            add_char(buf, format);
        add_char(buf, '0');
        size++;
        put_x++;
    }
    if (i == 0 && digit[0] == '0')
        reverse[j] = '0';
    while (--i >= 0)
        reverse[j++] = digit[i];
    if (reverse[0] == '0' && flags->is_precision && flags->precision == 0)
        reverse[0] = '\0';
    s21_strcat(buf, reverse);
    if (flags->flagm) {
        for (int in = 0; in < flags->width - diff - flags->flagsharp; in++) {
            if (flags->flagz == 0 || flags->flagm)
                add_char(buf, ' ');
            else
                add_char(buf, '0');
        }
    }
    for (s21_size_t in = 0; in < s21_strlen(buf); in++)
        str[flags->index++] = buf[in];
    str[flags->index] = '\0';
}

void s21_parse_p(char *str, va_list vlist, struct flags *flags, char format, int radix) {
    char digit[35] = "";
    char reverse[35] = "";
    unsigned long int dig = get_type(flags, vlist, format);
    s21_utoa(dig, digit);
    int i = 0, j = 2;
    i = radtoa(digit, dig, format, radix, i);
    s21_strcat(reverse, "0x");
    while (--i >= 0)
        reverse[j++] = digit[i];
    reverse[j] = '\0';
    if (!flags->flagm)
        add_spaces(str, flags, s21_strlen(reverse));
    for (s21_size_t in = 0; in < s21_strlen(reverse); in++)
            str[flags->index++] = reverse[in];
    if (flags->flagm)
        add_spaces(str, flags, s21_strlen(reverse));
}

void s21_parse_n(va_list vlist, struct flags *flags) {
    int *i = va_arg(vlist, int *);
    *i = flags->index;
}

int normalize(double *val) {
    int exponent = 0;
    double value = *val;
    while (value >= 1.0) {
        value /= 10.0;
        ++exponent;
    }
    while (value < 0.1) {
        value *= 10.0;
        --exponent;
    }
    *val = value;
    return exponent;
}

void zero_e(char *str, struct flags *flags, char format) {
    if (!flags->is_precision)
        flags->precision = 6;
    char result[250] = "";
    add_char(result, '0');
    if (!(flags->is_precision && flags->precision == 0 && !flags->flagsharp))
        add_char(result, '.');
    while (flags->precision-- > 0)
        add_char(result, '0');
    add_char(result, format);
    s21_strcat(result, "+00");
    if (flags->flagp && flags->flagz)
        str[flags->index++] = '+';
    if ((flags->width > (int)s21_strlen(result) || flags->flagm) && !flags->flagp && flags->flagz)
        str[flags->index++] = ' ';
    if ((flags->flagm || !flags->width) && !flags->flagp && flags->flags)
        str[flags->index++] = ' ';
    while (flags->width > (int)s21_strlen(result) + ((flags->flagz && flags->flags)
    || (flags->flagz && flags->flagp) || (!flags->flagz && flags->flagp)) && !flags->flagm) {
        if (!flags->flagz)
            str[flags->index++] = ' ';
        else
            str[flags->index++] = '0';
        flags->width--;
    }
    if (flags->flagp && !flags->flagz)
        str[flags->index++] = '+';
    for (s21_size_t i = 0; i < s21_strlen(result); i++)
        str[flags->index++] = result[i];
    if (flags->flags || flags->flagp)
        flags->width--;
    while (flags->width > (int)s21_strlen(result) && flags->flagm) {
        str[flags->index++] = ' ';
        flags->width--;
    }
    str[flags->index] = '\0';
}

void fill_result_arr2(int exponent, char *result, char *in_s, char format) {
    add_char(result, format);
    if (exponent < 0) {
        add_char(result, '-');
        exponent *= -1;
    } else {
        add_char(result, '+');
    }
    if (exponent < 10)
        add_char(result, '0');
    s21_itoa(exponent, in_s);
    s21_strcat(result, in_s);
}

int normalizel(long double *val) {
    int exponent = 0;
    long double value = *val;
    while (value >= 1.0) {
        value /= 10.0;
        ++exponent;
    }
    while (value < 0.1) {
        value *= 10.0;
        --exponent;
    }
    *val = value;
    return exponent;
}

void check_zero_arr(char *zero_arr, struct flags *flags) {
    int size = flags->is_precision ? flags->precision : 6;
    for (int i = 0; i < size; i++) {
        if (!zero_arr[i])
            zero_arr[i] = '0';
    }
}

void le_case(char *str, va_list vlist, struct flags *flags, char format) {
    long double f = va_arg(vlist, long double);
    int str_len = 0, is_negative = check_signl(&f, &str_len);
    if (f == 0.0) {
        zero_e(str, flags, format);
    } else {
        char in_s[10] = "", fl_s[20] = "", zero_arr[20] = "", result[100] = "";
        int exponent = normalizel(&f) - 1;
        f *= 1E1;
        unsigned long long int integer = f, in_fl = 0, first_num = integer, zeros = 0;
        long double fl = f - integer;
        zeros = get_num_of_digits(flags, fl_s, integer, &in_fl, fl);
        fill_zeroarr(zeros, zero_arr, fl_s);
        rounding(zero_arr, s21_strlen(zero_arr) - 1, in_fl % 10, flags);
        first_num += check_last_num(fl_s, flags, is_negative);
        fill_result_arr1(flags, first_num, in_s, result);
        if (flags->precision != 0)
            zero_arr[flags->precision] = '\0';
        else if (!flags->is_precision)
            zero_arr[6] = '\0';
        check_zero_arr(zero_arr, flags);
        if (!flags->is_precision || flags->precision != 0)
            s21_strcat(result, zero_arr);
        if (!flags->is_precision)
            flags->precision = 6;
        fill_result_arr2(exponent, result, in_s, format);
        if (is_negative && flags->flagz)
            str[flags->index++] = '-';
        if (flags->flagz && flags->flagp && !is_negative)
            str[flags->index++] = '+';
        if (flags->flagz && flags->flags && !is_negative)
            str[flags->index++] = ' ';
        if (!flags->flagm)
            add_spaces(str, flags, s21_strlen(result) + dodge_cppcheck(is_negative, flags));
        if (is_negative && !flags->flagz)
            str[flags->index++] = '-';
        if (!flags->flagz && flags->flagp && !is_negative)
            str[flags->index++] = '+';
        if (!flags->flagz && flags->flags && !is_negative)
            str[flags->index++] = ' ';
        for (s21_size_t i = 0; i < s21_strlen(result); i++)
            str[flags->index++] = result[i];
        if (flags->flagm)
            add_spaces(str, flags, s21_strlen(result) + dodge_cppcheck(is_negative, flags));
    }
    str[flags->index] = '\0';
}

void s21_parse_e(char *str, va_list vlist, struct flags *flags, char format) {
    if (flags->ldouble) {
        le_case(str, vlist, flags, format);
        return;
    }
    double f = va_arg(vlist, double);
    int str_len = 0, is_negative = check_sign(&f, &str_len);
    if (f == 0.0) {
        zero_e(str, flags, format);
    } else {
        char in_s[10] = "", fl_s[20] = "", zero_arr[20] = "", result[100] = "";
        int exponent = normalize(&f) - 1;
        f *= 1E1;
        unsigned long long int integer = (int)f, in_fl = 0, first_num = integer, zeros = 0;
        double fl = f - integer;
        zeros = get_num_of_digits(flags, fl_s, integer, &in_fl, fl);
        fill_zeroarr(zeros, zero_arr, fl_s);
        rounding(zero_arr, s21_strlen(zero_arr) - 1, in_fl % 10, flags);
        first_num += check_last_num(fl_s, flags, is_negative);
        fill_result_arr1(flags, first_num, in_s, result);
        if (flags->precision != 0)
            zero_arr[flags->precision] = '\0';
        else if (!flags->is_precision)
            zero_arr[6] = '\0';
        check_zero_arr(zero_arr, flags);
        if (!flags->is_precision || flags->precision != 0)
            s21_strcat(result, zero_arr);
        if (!flags->is_precision)
            flags->precision = 6;
        fill_result_arr2(exponent, result, in_s, format);
        if (is_negative && flags->flagz)
            str[flags->index++] = '-';
        if (flags->flagz && flags->flagp && !is_negative)
            str[flags->index++] = '+';
        if (flags->flagz && flags->flags && !is_negative)
            str[flags->index++] = ' ';
        if (!flags->flagm)
            add_spaces(str, flags, s21_strlen(result) + dodge_cppcheck(is_negative, flags));
        if (is_negative && !flags->flagz)
            str[flags->index++] = '-';
        if (!flags->flagz && flags->flagp && !is_negative)
            str[flags->index++] = '+';
        if (!flags->flagz && flags->flags && !is_negative)
            str[flags->index++] = ' ';
        for (s21_size_t i = 0; i < s21_strlen(result); i++)
            str[flags->index++] = result[i];
        if (flags->flagm)
            add_spaces(str, flags, s21_strlen(result) + dodge_cppcheck(is_negative, flags));
    }
    str[flags->index] = '\0';
}

void add_spaces(char *str, struct flags *flags, int str_len) {
    for (int i = 0; i < flags->width - str_len; i++) {
        if (flags->flagz == 0 || flags->flagm)
            str[flags->index++] = ' ';
        else
            str[flags->index++] = '0';
    }
}
