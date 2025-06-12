// https://github.com/FlowerBlackG/YurongOS/blob/i386-archive-v0.0.1/src/lib/stdio.cpp
/*
 * 标准输入输出实现〄1�7
 * 创建亄1�7 2022幄1�77朄1�713日��1�7
 * 
 * 参��：
 *   https://pubs.opengroup.org/onlinepubs/9699919799/
 *   https://cplusplus.com/reference/cstdio/sprintf/
 *   https://cplusplus.com/reference/cstdio/vsprintf/
 *   https://cplusplus.com/reference/cstdio/printf/
 */

#include <libyrosstd/stdio.h>
#include <libyrosstd/string.h>

int sprintf(char* buffer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int res = vsprintf(buffer, format, args);
    va_end(args);
    return res;
}

int vsprintf(char* buffer, const char* format, va_list args) {
    
    // %[flags][width][.precision][length]specifier

    // flag 定义〄1�7
#define __VPF_FLAG_NULL 0
#define __VPF_FLAG_ZERO_BIT 1
#define __VPF_FLAG_SPACE_BIT 2
#define __VPF_FLAG_PLUS_BIT 4
#define __VPF_FLAG_MINUS_BIT 8
#define __VPF_FLAG_NUMBER_SIGN_BIT 16

    // 长度限定符定义��1�7
#define __VPF_LENGTH_NULL 0
#define __VPF_LENGTH_CHAR 1
#define __VPF_LENGTH_SHORT 2
#define __VPF_LENGTH_LONG 3
#define __VPF_LENGTH_LONG_LONG 4
#define __VPF_LENGTH_INT_MAX 5
#define __VPF_LENGTH_SIZE_T 6
#define __VPF_LENGTH_PTRDIFF_T 7
#define __VPF_LENGTH_LONG_DOUBLE 8

    // 精度〄1�7
#define __VPF_PRECISION_NULL -1

    // 宽度〄1�7
#define __VPF_WIDTH_NULL -1


    char* pBuf = buffer;
    const char* pFmt = format;

    const char* pPrecisionSign;

    uint8_t lengthSpecifier;
    uint8_t flag;

    int width;
    int precision;

    // 工具闭包函数〄1�7
    // 拼接丢�丄1�716进制数��自动补充对齐等〄1�7
    auto catHex = [&] (uint64_t hexVal, size_t nbytes, bool upper) {

        char tmpStr[32];
        char* pTmpStr = tmpStr;
        size_t nHalfBytes = 2 * nbytes;

        if (hexVal == 0) {
            *(pTmpStr++) = '0';
        } else while (hexVal > 0 && (nHalfBytes--) > 0) {
            uint8_t digit = hexVal & 0xf;
            hexVal >>= 4;

            if (digit <= 9) {
                *(pTmpStr++) = digit + '0';
            } else {
                *(pTmpStr++) = digit - 10 + 'a';
            }
        }

        int32_t len = pTmpStr - tmpStr;
        int32_t paddingLen = precision > width ? precision : width;
        if (paddingLen < 0) {
            paddingLen = 0;
        }

        paddingLen -= len;
        if (paddingLen < 0) {
            paddingLen = 0;
        }

        if (paddingLen > 0 && !(flag & __VPF_FLAG_MINUS_BIT)) {
            memset(
                pBuf,
                ((flag & __VPF_FLAG_ZERO_BIT) ? '0' : ' '),
                paddingLen
            );
            pBuf += paddingLen;
        }

        if (flag & __VPF_FLAG_NUMBER_SIGN_BIT) {
            *(pBuf++) = '0';
            *(pBuf++) = upper ? 'X' : 'x';
        }
        
        while (pTmpStr > tmpStr) {
            *(pBuf++) = *(--pTmpStr);
        }

        if (paddingLen > 0 && (flag & __VPF_FLAG_MINUS_BIT)) {
            memset(pBuf, ' ', paddingLen);
            pBuf += paddingLen;
        }
    };

    while (*pFmt != NULL) {

        /* ------------ 百分叄1�7 %. ------------ */

        if (*pFmt == '%') {
            pPrecisionSign = pFmt++;

            lengthSpecifier = __VPF_LENGTH_NULL;
            flag = __VPF_FLAG_NULL;
            precision = __VPF_PRECISION_NULL;
            width = __VPF_WIDTH_NULL;
        } else {
            *(pBuf++) = *(pFmt++);
            continue;;
        }


        /* ------------ flag. ------------ */

        while (true) {
            if (*pFmt == '-') {
                flag |= __VPF_FLAG_MINUS_BIT;
                ++pFmt;
            } else if (*pFmt == '+') {
                flag |= __VPF_FLAG_PLUS_BIT;
                ++pFmt;
            } else if (*pFmt == ' ') {
                flag |= __VPF_FLAG_SPACE_BIT;
                ++pFmt;
            } else if (*pFmt == '#') {
                flag |= __VPF_FLAG_NUMBER_SIGN_BIT;
                ++pFmt;
            } else if (*pFmt == '0') {
                flag |= __VPF_FLAG_ZERO_BIT;
                ++pFmt;
            } else {
                break;
            }
        }


        /* ------------ width. ------------ */

        if (*pFmt == '*') {
            width = va_arg(args, int);
            ++pFmt;
        } else if (*pFmt >= '0' && *pFmt <= '9') {
            width = *(pFmt++) - '0';
            while (*pFmt >= '0' && *pFmt <= '9') {
                width *= 10;
                width += *(pFmt++) - '0';
            }

        }


        /* ------------ precision. ------------ */

        if (*pFmt == '.') {
            ++pFmt;

            if (*pFmt == '*') {
                precision = va_arg(args, int);
                ++pFmt;
            } else if (*pFmt >= '0' && *pFmt <= '9') {
                precision = *(pFmt++) - '0';
                while (*pFmt >= '0' && *pFmt <= '9') {
                    precision *= 10;
                    precision += *(pFmt++) - '0';
                }
            } else {
                // 格式解析异常〄1�7
                return -1;
            }
        }


        /* ------------ 长度限定符��1�7 ------------ */

        if (*pFmt == 'h' && *(pFmt + 1) == 'h') {
            pFmt += 2;
            lengthSpecifier = __VPF_LENGTH_CHAR;
        } else if (*pFmt == 'h') {
            pFmt += 1;
            lengthSpecifier = __VPF_LENGTH_SHORT;
        } else if (*pFmt == 'l') {
            pFmt += 1;
            lengthSpecifier = __VPF_LENGTH_LONG;
        } else if (*pFmt == 'l' && *(pFmt + 1) == 'l') {
            pFmt += 2;
            lengthSpecifier = __VPF_LENGTH_LONG_LONG;
        } else if (*pFmt == 'j') {
            pFmt += 1;
            lengthSpecifier = __VPF_LENGTH_INT_MAX;
        } else if (*pFmt == 'z') {
            pFmt += 1;
            lengthSpecifier = __VPF_LENGTH_SIZE_T;
        } else if (*pFmt == 't') {
            pFmt += 1;
            lengthSpecifier = __VPF_LENGTH_PTRDIFF_T;
        } else if (*pFmt == 'L') {
            pFmt += 1;
            lengthSpecifier = __VPF_LENGTH_LONG_DOUBLE;
        }


        /* ------------ 类型限定符��1�7 ------------ */

        char specifier = *(pFmt++);
        
        switch (specifier) {
            case '%':
                *(pBuf++) = '%';
                continue;

            // 整数系列〄1�7

            case 'd':
            case 'i': 
            case 'u':
            case 'o': {
                char tmp[64];
                char* pTmp = tmp;

                // todo: 无法处理 long long 筄1�7 64 位数据类型��1�7
                uint8_t base = (specifier == 'o' ? 8 : 10);
                uint32_t mask;
                uint32_t val;
                char sign = NULL;

                // �Ȼ�ȡԭʼֵ���жϷ���
                if (lengthSpecifier == __VPF_LENGTH_CHAR) {
                    char originalVal = (char)va_arg(args, int);
                    if ((specifier == 'd' || specifier == 'i') && originalVal < 0) {
                        sign = '-';
                        val = (uint32_t)(-originalVal);
                    } else {
                        mask = (1UL << (sizeof(char) * 8)) - 1;
                        val = (uint32_t)originalVal & mask;
                    }
                } else if (lengthSpecifier == __VPF_LENGTH_SHORT) {
                    short originalVal = (short)va_arg(args, int);
                    if ((specifier == 'd' || specifier == 'i') && originalVal < 0) {
                        sign = '-';
                        val = (uint32_t)(-originalVal);
                    } else {
                        mask = (1UL << (sizeof(short) * 8)) - 1;
                        val = (uint32_t)originalVal & mask;
                    }
                } else if (lengthSpecifier == __VPF_LENGTH_LONG) {
                    long originalVal = va_arg(args, long);
                    if ((specifier == 'd' || specifier == 'i') && originalVal < 0) {
                        sign = '-';
                        val = (uint32_t)(-originalVal);
                    } else {
                        mask = (1UL << (sizeof(long) * 8)) - 1;
                        val = (uint32_t)originalVal & mask;
                    }
                } else if (lengthSpecifier == __VPF_LENGTH_LONG_LONG) {
                    long long originalVal = va_arg(args, long long);
                    if ((specifier == 'd' || specifier == 'i') && originalVal < 0) {
                        sign = '-';
                        val = (uint32_t)(-originalVal);
                    } else {
                        mask = (1UL << (sizeof(long long) * 8)) - 1;
                        val = (uint32_t)originalVal & mask;
                    }
                } else if (lengthSpecifier == __VPF_LENGTH_INT_MAX) {
                    intmax_t originalVal = va_arg(args, intmax_t);
                    if ((specifier == 'd' || specifier == 'i') && originalVal < 0) {
                        sign = '-';
                        val = (uint32_t)(-originalVal);
                    } else {
                        mask = (1UL << (sizeof(intmax_t) * 8)) - 1;
                        val = (uint32_t)originalVal & mask;
                    }
                } else if (lengthSpecifier == __VPF_LENGTH_SIZE_T) {
                    size_t originalVal = va_arg(args, size_t);
                    // size_t ���޷������ͣ�����Ҫ�����ж�
                    mask = (1UL << (sizeof(size_t) * 8)) - 1;
                    val = (uint32_t)originalVal & mask;
                } else {
                    mask = (1ULL << (sizeof(int) * 8)) - 1;
                    val = va_arg(args, int) & mask;
                }

                char sign = NULL;

                // 判断是否为负数��1�7
                if (specifier == 'd' || specifier == 'i') {
                    if (
                        (lengthSpecifier == __VPF_LENGTH_CHAR && (char) val < 0)
                        || (lengthSpecifier == __VPF_LENGTH_SHORT && (short) val < 0)
                        || (lengthSpecifier == __VPF_LENGTH_LONG && (long) val < 0)
                        || (lengthSpecifier == __VPF_LENGTH_LONG_LONG && (long long) val < 0)
                        || (lengthSpecifier == __VPF_LENGTH_INT_MAX && (intmax_t) val < 0)
                        || (lengthSpecifier == __VPF_LENGTH_SIZE_T && (size_t) val < 0)
                        || (lengthSpecifier == __VPF_LENGTH_NULL && (int) val < 0)
                    ) {
                        sign = '-';
                        val = (uint32_t)(-originalVal);
                    } else {
                        mask = (1UL << (sizeof(int) * 8)) - 1;
                        val = (uint32_t)originalVal & mask;
                    }
                }
                
                // 非负情况，决定是否要强制设置符号〄1�7
                if (sign == NULL) {
                    if (flag & __VPF_FLAG_SPACE_BIT) {
                        sign = ' ';
                    } else if (flag & __VPF_FLAG_PLUS_BIT) {
                        sign = '+';
                    }
                }

                // 提取各位数字〄1�7
                if (val == 0) {
                    *(pTmp++) = '0';
                } else while (val > 0) {
                    *(pTmp++) = (val % base) + '0';
                    val /= base;
                }

                // 8进制补充前缀0〄1�7
                if ((flag & __VPF_FLAG_NUMBER_SIGN_BIT) && specifier == 'o') {
                    *(pTmp++) = '0';
                }

                // 数字串长度��1�7
                int32_t len = pTmp - tmp;

                // 填充串长度��符号算在填充串内��1�7
                int32_t paddingLen = (precision > width ? precision : width);
                paddingLen = (paddingLen > 0 ? paddingLen : 0);

                paddingLen -= len;

                paddingLen = (paddingLen > 0 ? paddingLen : 0);

                // 输出前缀填充〄1�7
                if (paddingLen && (flag & __VPF_FLAG_ZERO_BIT || !(flag & __VPF_FLAG_MINUS_BIT)))
                {

                    memset(
                        pBuf,
                        (flag & __VPF_FLAG_ZERO_BIT ? '0' : ' '),
                        paddingLen
                    );

                    if (sign) {
                        if (flag & __VPF_FLAG_ZERO_BIT) {
                            *(pBuf++) = sign;
                        } else {
                            *(pBuf + paddingLen - 1) = sign;
                        }
                    }

                    pBuf += paddingLen;
                } else if (sign) {
                    *(pBuf++) = sign;
                }

                // 输出数字〄1�7
                while (pTmp > tmp) {
                    *(pBuf++) = *(--pTmp);
                }

                // 后缀填充空格〄1�7
                if (paddingLen && !(flag & __VPF_FLAG_ZERO_BIT) && flag & __VPF_FLAG_MINUS_BIT)
                {
                    int32_t len = paddingLen - !!(sign != NULL);
                    memset(pBuf, ' ', len);
                    pBuf += len;
                }

                break;
            }
        
            // 16 进制无符号整数��1�7
            case 'x':
            case 'X': {
                if (lengthSpecifier == __VPF_LENGTH_CHAR) {
                    catHex(va_arg(args, int), sizeof(char), (specifier == 'X'));
                } else if (lengthSpecifier == __VPF_LENGTH_SHORT) {
                    catHex(va_arg(args, int), sizeof(short), (specifier == 'X'));
                } else if (lengthSpecifier == __VPF_LENGTH_LONG) {
                    catHex(va_arg(args, long), sizeof(long), (specifier == 'X'));
                } else if (lengthSpecifier == __VPF_LENGTH_LONG_LONG) {
                    catHex(va_arg(args, long long), sizeof(long long), (specifier == 'X'));
                } else if (lengthSpecifier == __VPF_LENGTH_INT_MAX) {
                    catHex(va_arg(args, intmax_t), sizeof(intmax_t), (specifier == 'X'));
                } else if (lengthSpecifier == __VPF_LENGTH_SIZE_T) {
                    catHex(va_arg(args, size_t), sizeof(size_t), (specifier == 'X'));
                } else {
                    catHex(va_arg(args, int), sizeof(int), (specifier == 'X'));
                }

                break;
            }


            // 浮点系列〄1�7
            
            // 浮点系列未实现��1�7 todo.

            // char〄1�7
            // lc 形式未实现��todo.
            case 'c': {
                int ch = va_arg(args, int);

                if (!(precision & __VPF_FLAG_MINUS_BIT) && width > 1) {
                    memset(pBuf, ' ', width - 1);
                    pBuf += width - 1;
                }

                *(pBuf++) = (char) ch;

                if ((precision & __VPF_FLAG_MINUS_BIT) && width > 1) {
                    memset(pBuf, ' ', width - 1);
                    pBuf += width - 1;
                }

                break;
            }

            // 字符串��1�7
            case 's': {
                char* spStr = va_arg(args, char*);
                int32_t sLen = strlen(spStr);

                int32_t sRealLen;
                if (precision == __VPF_PRECISION_NULL || sLen <= precision) {
                    sRealLen = sLen;
                } else {
                    sRealLen = precision;
                }

                int32_t sPaddingLen;
                if (width == __VPF_WIDTH_NULL || width <= sRealLen) {
                    sPaddingLen = 0;
                } else {
                    sPaddingLen = width - sRealLen;
                }

                if (sPaddingLen == 0 || (flag & __VPF_FLAG_MINUS_BIT)) {
                    memcpy(pBuf, spStr, sRealLen);
                    pBuf += sRealLen;
                }

                memset(pBuf, ' ', sPaddingLen);
                pBuf += sPaddingLen;

                if (sPaddingLen > 0 && !(flag & __VPF_FLAG_MINUS_BIT)) {
                    memcpy(pBuf, spStr, sRealLen);
                    pBuf += sRealLen;
                }

                break;
            }

            // 指针〄1�7
            case 'p': {
                void* pointer = va_arg(args, void*);
                uint64_t ui64pointer = (uint64_t) pointer;
                catHex(ui64pointer, sizeof(ui64pointer), false);  
                break;
            } 

            // 空��1�7
            case 'n': //todo
                break;

            default:
                // 格式解析异常〄1�7
                return -1;

        } // switch (specifier)


    } // while (*pFmt != NULL)

    *pBuf = '\0';
    return pBuf - buffer;

    // 取消 flag 定义，防止干扰后续操作��1�7
#undef __VPF_FLAG_NULL
#undef __VPF_FLAG_ZERO_BIT
#undef __VPF_FLAG_SPACE_BIT
#undef __VPF_FLAG_PLUS_BIT
#undef __VPF_FLAG_MINUS_BIT
#undef __VPF_FLAG_NUMBER_SIGN_BIT

    // 取消长度限定符定义��1�7
#undef __VPF_LENGTH_NULL
#undef __VPF_LENGTH_CHAR
#undef __VPF_LENGTH_SHORT
#undef __VPF_LENGTH_LONG
#undef __VPF_LENGTH_LONG_LONG
#undef __VPF_LENGTH_INT_MAX
#undef __VPF_LENGTH_SIZE_T
#undef __VPF_LENGTH_PTRDIFF_T
#undef __VPF_LENGTH_LONG_DOUBLE

    // 取消精度定义〄1�7
#undef __VPF_PRECISION_NULL

    // 取消宽度定义〄1�7
#undef __VPF_WIDTH_NULL

} // int vsprintf(char* buffer, const char* format, va_list args)
