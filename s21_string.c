#include "./s21_string.h"

void *s21_memchr(const void *ptr, int c, s21_size_t count) {
    s21_size_t i = 0;
    unsigned char x = (unsigned char)c;
    char *ret = s21_NULL;
    char *buf = (char *)ptr;
    while (i < count && c >= 0) {
        if (x == *((char *)(buf + i))) {
            ret = (void *)(buf + i);
            break;
        }
        i++;
    }
    return (void *)(ret);
}

int s21_memcmp(const void *str1, const void *str2, s21_size_t n) {
    int ret = 0;
    if (str1 != s21_NULL && str2 != s21_NULL) {
        unsigned char *string1;
        unsigned char *string2;
        string1 = (unsigned char *)str1;
        string2 = (unsigned char *)str2;

        for (s21_size_t i = 0; i < n; i++) {
            if (string1[i] != string2[i]) {
                ret = string1[i] - string2[i];
                break;
            }
        }
    }
    return (ret);
}

void *s21_memcpy(void *dest, const void *src, s21_size_t n) {
    if ((char *)dest != s21_NULL && (char *)dest != s21_NULL) {
        s21_size_t i;
        i = 0;
        while (i < n) {
            *((unsigned char *)dest + i) = *((unsigned char *)src + i);
            i++;
        }
    }
    return (dest);
}

/*
Despite being specified "as if" a temporary buffer is used, actual
implementations of this function do not incur the overhead of double copying or
extra memory. For small count, it may load up and write out registers; for
larger blocks, a common approach (glibc and bsd libc) is to copy bytes forwards
from the beginning of the buffer if the destination starts before the source,
and backwards from the end otherwise, with a fall back to std::memcpy when there
is no overlap at all.

https://en.cppreference.com/w/cpp/string/byte/memmove
*/

void *s21_memmove(void *dest, const void *src, s21_size_t n) {
    char *pszDest = (char *)dest;
    const char *pszSrc = (const char *)src;
    if (pszDest != s21_NULL && pszSrc != s21_NULL) {
        while (n--)
            *pszDest++ = *pszSrc++;
    }
    return dest;
}

void *s21_memset(void *str, int c, s21_size_t n) {
    unsigned char x;
    s21_size_t i;
    x = (unsigned char)c;

    i = 0;
    while (i < n && n > 0) {
        *((char *)str + i) = x;
        i++;
    }
    return (str);
}

char *s21_strcat(char *dest, const char *src) {
    char *s = dest;
    while (*dest) {
        dest++;
    }
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return s;
}

char *s21_strncat(char *dest, const char *src, s21_size_t n) {
    s21_size_t i = 0;
    s21_size_t j = 0;
    while (dest[i] != '\0')
        i++;
    while (j < n && src[j] != '\0')
        dest[i++] = src[j++];
    dest[i] = '\0';
    return (dest);
}

int s21_strcmp(const char *str1, const char *str2) {
    s21_size_t i;
    int result;
    i = 0;
    result = 0;
    if (str1 != s21_NULL && str2 != s21_NULL) {
        while ((str1[i] != '\0') || (str2[i] != '\0')) {
            result = (str1[i] - str2[i]);
            if (result)
                break;
            i++;
        }
    }
    return (result);
}

int s21_strncmp(const char *str1, const char *str2, s21_size_t n) {
    s21_size_t i;
    int result;

    i = 0;
    result = 0;
    if (str1 != s21_NULL && str2 != s21_NULL && n != 0) {
        while (((str1[i] != '\0') || (str2[i] != '\0')) && (i < n)) {
            result = (str1[i] - str2[i]);

            if (result) break;

            i++;
        }
    }
    return (result);
}

char *s21_strcpy(char *dest, const char *src) {
    s21_size_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return (dest);
}

char *s21_strncpy(char *dest, const char *src, s21_size_t n) {
    if (!src) {
        dest = s21_NULL;
    } else {
        s21_size_t size = 0;
        while (*src && n) {
            dest[size] = *src;
            src++;
            size++;
            dest[size] = '\0';
            n--;
        }
    }
    return dest;
}

s21_size_t s21_strcspn(const char *str1, const char *str2) {
    /* Returns the length of the maximum initial segment of the null-terminated
    byte string pointed to by dest, that consists of only the characters not found
    in the null-terminated byte string pointed to by src.

    The behavior is undefined if either dest or src is not a pointer to a
    null-terminated byte string.
    */
    s21_size_t i = 0;
    if (str1 != s21_NULL && str2 != s21_NULL) {
        int j = 0;
        while (str1[i] != '\0') {
            while (str2[j] != '\0') {
                if (str1[i] == str2[j])
                    break;
                ++j;
            }
            if (str1[i] == str2[j])
                break;
            j = 0;
            ++i;
        }
    }
    return (i);
}

char *s21_strerror(int errnum) {
    char *error = s21_NULL;
    static char result[42];
    char digit[12] = "";
    char *s21_error[] = s21_errors;
    if (errnum <= s21_N_error && errnum >= 0) {
        s21_strcpy(result, s21_error[errnum]);
        error = result;
    } else {
        #if defined(__APPLE__) && defined(__MACH__)
            s21_strcpy(result, "Unknown error: ");
        #elif defined(__linux__)
            s21_strcpy(result, "Unknown error ");
        #endif
        if (errnum < 0) {
            s21_strcat(digit, "-");
            errnum *= -1;
        }
        s21_itoa(errnum, &digit[s21_strlen(digit)]);
        s21_strcat(result, digit);
        error = result;
    }
    return error;
}

s21_size_t s21_strlen(const char *str) {
    s21_size_t i;

    i = 0;
    while (str != s21_NULL && str[i] != '\0') ++i;

    return (i);
}

char *s21_strpbrk(const char *str1, const char *str2) {
    s21_size_t i;
    s21_size_t j;

    i = 0;
    j = 0;
    char *ret = s21_NULL;
    while (str1[i] != '\0') {
        while (str2[j] != '\0') {
            if (str1[i] == str2[j]) {
                ret = (char *)str1 + i;
                break;
            }
            ++j;
        }
        if (str1[i] == str2[j])
            break;
        j = 0;
        ++i;
    }
    return (ret);
}

char *s21_strrchr(const char *str, int c) {
    s21_size_t i;
    char *ret = s21_NULL;
    i = (int)s21_strlen(str) + 1;

    while (i-- > 0) {
        if (str[i] == c) {
            ret = (char*)&str[i];
            break;
        }
    }
    return ret;
}

s21_size_t s21_strspn(const char *str1, const char *str2) {
    char *str = s21_NULL;
    s21_size_t i = 0;

    if (str1 != s21_NULL && str2 != s21_NULL) {
        str = (char *)str1;
        s21_size_t j = 0;
        while (str[i] != '\0') {
            while ((str2[j] != '\0') && (str2[j] != str[i])) ++j;

            if ('\0' == str2[j])
                break;
            else
                j = 0;

            ++i;
        }
    }
    return (i);
}

char *s21_strstr(const char *haystack, const char *needle) {
    char *ret;
    if ((haystack[0] == '\0') && (needle[0] == '\0')) {
        ret = ((char *)haystack);
    } else {
        ret = s21_NULL;
        s21_size_t length_n = s21_strlen(needle);
        s21_size_t i = 0;

        while (haystack[i] != '\0') {
            s21_size_t j = 0;
            while (j < length_n) {
                if (needle[j] != haystack[i + j])
                    break;
                j++;
            }
            if (j == length_n) {
                ret = ((char *)haystack + i);
                break;
            }
            i++;
        }
    }
    return (ret);
}

int del_check(char c, const char *delim) {
  int res = 0;
  while (*delim != '\0') {
    if (c == *delim) res++;
    delim++;
  }
  return res;
}

char *s21_strtok(char *str, const char *delim) {
    static char *ptr;
    char *ret = s21_NULL;
    if (!str) str = ptr;
    if (str) {
        int flag = 0;
        while (flag == 0) {
            if (del_check(*str, delim)) {
                str++;
                continue;
            }
            if (*str == '\0') flag = 1;
            break;
        }
        char *inter = str;
        while (1 && flag == 0) {
            if (*str == '\0') {
                ptr = str;
                ret = inter;
                flag = 1;
            }
            if (del_check(*str, delim)) {
                *str = '\0';
                ptr = str + 1;
                ret = inter;
                flag = 1;
            }
            str++;
        }
    }
    return ret;
}

const char *s21_strchr(const char *str, int c) {
    char *ch = 0;
    int len = s21_strlen(str) + 1;
    for (int i = 0; i < len; i++) {
        if (str[i] == (char)c) {
            ch = (char*)&str[i];
            break;
        }
    }
    return ch;
}

void *s21_to_upper(const char *str) {
    char *str_up = s21_NULL;
    if (str != s21_NULL) {
        int size = s21_strlen(str) + 1;
        str_up = malloc(size * sizeof(char));
        int i;
        for (i = 0; str[i]; i++) {
            if (str[i] >= 97 && str[i] <= 122) {
                str_up[i] = str[i] - 32;
            } else {
                str_up[i] = str[i];
            }
        }
        str_up[i] = '\0';
    }
    return str_up;
}

void *s21_to_lower(const char *str) {
    char *str_low = s21_NULL;
    if (str != s21_NULL) {
        int size = s21_strlen(str) + 1;
        int i;
        str_low = malloc(size * sizeof(char));
        for (i = 0; str[i]; i++) {
            if (str[i] <= 90 && str[i] >= 65) {
                str_low[i] = str[i] + 32;
            } else {
                str_low[i] = str[i];
            }
        }
        str_low[i] = '\0';
    }
    return str_low;
}

void *s21_insert(const char *src, const char *str, s21_size_t start_index) {
    char *new_str = s21_NULL;
    if (src && str && s21_strlen(src) >= start_index) {
        new_str = (char *)malloc((s21_strlen(src) + s21_strlen(str) + 1) *
                                 sizeof(char));
        if (new_str) {
            s21_strncpy(new_str, src, start_index);
            *(new_str + start_index) = '\0';
            s21_strcat(new_str, str);
            s21_strcat(new_str, src + start_index);
        }
    }
    return (void *)new_str;
}

void *s21_trim(const char *src, const char *trim_chars) {
    char *src_new = s21_NULL;
    if (src != s21_NULL && trim_chars != s21_NULL) {
        int size = s21_strlen(src) + 1;
        src_new = malloc(size * sizeof(char));
        int i = 0, k = size - 2, flag = 1, l;
        while (src[i] && flag) {
            flag = 0;
            for (int j = 0; trim_chars[j]; j++) {
                if (src[i] == trim_chars[j]) {
                    flag = 1;
                    i++;
                    break;
                }
            }
        }
        flag = 1;
        while (k >=0 && flag) {
            flag = 0;
            for (int j = 0; trim_chars[j]; j++) {
                if (src[k] == trim_chars[j]) {
                    flag = 1;
                    k--;
                    break;
                }
            }
        }
        k = k - i;
        for (l = 0; l <= k; i++, l++)
            src_new[l] = src[i];
        src_new[l] = '\0';
    }
    return src_new;
}

void s21_itoa(unsigned long long int m, char *str) {
    if (m == 0) {
        *str++ = '0';
        *str = '\0';
        return;
    }
    unsigned long long int power = 0, j = 0;
    j = m;
    for (power = 1; j >= 10; j /= 10)
        power = power * 10;
    for (; power > 0; power /= 10) {
        *str++ = '0' + m / power;
        m %= power;
    }
    *str = '\0';
}
