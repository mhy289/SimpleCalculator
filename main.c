#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>

// 错误类型枚举
typedef enum {
    ERROR_NONE,
    ERROR_INVALID_CHAR,
    ERROR_INVALID_NUMBER,
    ERROR_DIVISION_BY_ZERO,
    ERROR_INVALID_OPERATOR,
    ERROR_MISSING_OPERAND,
    ERROR_TRAILING_OPERATOR,
    ERROR_EMPTY_EXPRESSION
} ErrorType;

// 错误信息
const char* error_messages[] = {
    "无错误",
    "无效字符",
    "无效的数字格式",
    "除以零错误",
    "无效的运算符位置",
    "缺少操作数",
    "表达式末尾不能是运算符",
    "表达式为空"
};

// 解析数字并返回其值，同时更新索引位置
double parse_number(const char* expr, int* index, ErrorType* error) {
    int start = *index;
    bool is_hex = false;
    bool has_dot = false;
    double value = 0.0;

    // 检查是否是十六进制数
    if (expr[*index] == '0' && (expr[*index + 1] == 'x' || expr[*index + 1] == 'X')) {
        is_hex = true;
        *index += 2; // 跳过"0x"

        // 十六进制数不能有小数点
        if (expr[*index] == '.') {
            *error = ERROR_INVALID_NUMBER;
            return 0.0;
        }

        // 解析十六进制数
        while (expr[*index] != '\0' && expr[*index] != '=' &&
               (isxdigit((unsigned char)expr[*index]) || expr[*index] == ' ')) {
            if (expr[*index] == ' ') {
                (*index)++;
                continue;
            }

            int digit;
            if (isdigit((unsigned char)expr[*index])) {
                digit = expr[*index] - '0';
            } else if (expr[*index] >= 'a' && expr[*index] <= 'f') {
                digit = 10 + expr[*index] - 'a';
            } else { // 'A' to 'F'
                digit = 10 + expr[*index] - 'A';
            }

            value = value * 16 + digit;
            (*index)++;
        }

        // 检查是否解析到了有效的十六进制数字
        if (*index <= start + 2) {
            *error = ERROR_INVALID_NUMBER;
            return 0.0;
        }

        return value;
    }

    // 解析十进制数（可能有符号）
    if (expr[*index] == '+' || expr[*index] == '-') {
        (*index)++;
    }

    // 整数部分
    while (expr[*index] != '\0' && expr[*index] != '=' && expr[*index] != '+' &&
           expr[*index] != '-' && expr[*index] != '*' && expr[*index] != '/' &&
           expr[*index] != '.') {
        if (expr[*index] == ' ') {
            (*index)++;
            continue;
        }

        if (!isdigit((unsigned char)expr[*index])) {
            *error = ERROR_INVALID_NUMBER;
            return 0.0;
        }

        value = value * 10 + (expr[*index] - '0');
        (*index)++;
    }

    // 检查是否有小数部分
    if (expr[*index] == '.') {
        has_dot = true;
        (*index)++;

        double fraction = 0.1;
        while (expr[*index] != '\0' && expr[*index] != '=' && expr[*index] != '+' &&
               expr[*index] != '-' && expr[*index] != '*' && expr[*index] != '/') {
            if (expr[*index] == ' ') {
                (*index)++;
                continue;
            }

            if (!isdigit((unsigned char)expr[*index])) {
                *error = ERROR_INVALID_NUMBER;
                return 0.0;
            }

            value += (expr[*index] - '0') * fraction;
            fraction *= 0.1;
            (*index)++;
        }
    }

    // 检查是否解析到了有效的数字
    if (*index <= start || (*index == start + 1 && (expr[start] == '+' || expr[start] == '-'))) {
        *error = ERROR_INVALID_NUMBER;
        return 0.0;
    }

    // 应用符号
    if (expr[start] == '-') {
        value = -value;
    }

    return value;
}

// 检查表达式中的字符是否有效
ErrorType validate_expression(const char* expr) {
    int len = strlen(expr);
    if (len == 0) {
        return ERROR_EMPTY_EXPRESSION;
    }

    // 检查是否有等号，且等号是否在末尾
    int equals_pos = -1;
    for (int i = 0; i < len; i++) {
        if (expr[i] == '=') {
            if (equals_pos != -1) { // 多个等号
                return ERROR_INVALID_CHAR;
            }
            equals_pos = i;
        } else if (!isalnum((unsigned char)expr[i]) && expr[i] != '+' && expr[i] != '-' &&
                  expr[i] != '*' && expr[i] != '/' && expr[i] != '.' && expr[i] != ' ' &&
                  expr[i] != 'x' && expr[i] != 'X') {
            return ERROR_INVALID_CHAR;
        }
    }

    if (equals_pos == -1) { // 没有等号
        return ERROR_INVALID_CHAR;
    }

    if (equals_pos != len - 1) { // 等号不在末尾
        return ERROR_INVALID_CHAR;
    }

    return ERROR_NONE;
}

// 计算表达式的值
double calculate(const char* expr, ErrorType* error, int* error_pos) {
    *error = validate_expression(expr);
    if (*error != ERROR_NONE) {
        *error_pos = 0;
        return 0.0;
    }

    int len = strlen(expr);
    int index = 0;
    double result = 0.0;
    char current_op = '+'; // 初始操作为加，处理第一个数
    bool has_operand = false;

    while (index < len - 1) { // 最后一个字符是'='，不需要处理
        // 跳过空格
        while (expr[index] == ' ') {
            index++;
        }

        if (expr[index] == '+' || expr[index] == '-' || expr[index] == '*' || expr[index] == '/') {
            // 如果是第一个字符且是减号，允许作为负数处理
            if (index == 0 && expr[index] == '-') {
                // 继续解析数字，会处理负号
            } else {
                *error = ERROR_INVALID_OPERATOR;
                *error_pos = index;
                return 0.0;
            }
        }

        // 解析数字
        *error_pos = index;
        double num = parse_number(expr, &index, error);
        if (*error != ERROR_NONE) {
            return 0.0;
        }

        has_operand = true;

        // 执行当前操作
        switch (current_op) {
            case '+':
                result += num;
                break;
            case '-':
                result -= num;
                break;
            case '*':
                result *= num;
                break;
            case '/':
                if (fabs(num) < 1e-10) { // 防止除以零
                    *error = ERROR_DIVISION_BY_ZERO;
                    *error_pos = index;
                    return 0.0;
                }
                result /= num;
                break;
            default:
                *error = ERROR_INVALID_OPERATOR;
                *error_pos = index;
                return 0.0;
        }

        // 跳过空格
        while (expr[index] == ' ') {
            index++;
        }

        // 获取下一个运算符
        if (index < len - 1) { // 不是等号
            if (expr[index] == '+' || expr[index] == '-' || expr[index] == '*' || expr[index] == '/') {
                current_op = expr[index];
                index++;
            } else {
                *error = ERROR_INVALID_OPERATOR;
                *error_pos = index;
                return 0.0;
            }
        }
    }

    if (!has_operand) {
        *error = ERROR_EMPTY_EXPRESSION;
        *error_pos = 0;
        return 0.0;
    }

    // 检查最后一个字符是否是运算符（等号前）
    int last_op_pos = len - 2;
    while (last_op_pos >= 0 && expr[last_op_pos] == ' ') {
        last_op_pos--;
    }

    if (last_op_pos >= 0 && (expr[last_op_pos] == '+' || expr[last_op_pos] == '-' ||
                             expr[last_op_pos] == '*' || expr[last_op_pos] == '/')) {
        *error = ERROR_TRAILING_OPERATOR;
        *error_pos = last_op_pos;
        return 0.0;
    }

    *error = ERROR_NONE;
    return result;
}

// 打印错误信息和指示
void print_error(const char* expr, ErrorType error, int error_pos) {
    printf("错误: %s\n", error_messages[error]);
    printf("%s\n", expr);

    // 打印指示错误位置的箭头
    for (int i = 0; i < error_pos; i++) {
        putchar(' ');
    }
    printf("^\n");
}

int main() {
    system("chcp 65001"); // 需要额外设置控制台为UTF-8
    char expression[1024];
    printf("=== 十六进制四则运算计算器 ===\n");
    printf("支持十进制(整数和小数)和十六进制(0x开头)数字\n");
    printf("支持 +, -, *, / 运算\n");


    while (1) {
        printf("输入表达式以 = 结束，输入 q 退出\n");
        printf("请输入表达式: ");
        if (fgets(expression, sizeof(expression), stdin) == NULL) {
            break;
        }

        // 去除换行符
        size_t len = strlen(expression);
        if (len > 0 && expression[len - 1] == '\n') {
            expression[len - 1] = '\0';
        }

        // 检查是否退出
        if (strcmp(expression, "q") == 0 || strcmp(expression, "Q") == 0) {
            break;
        }

        ErrorType error;
        int error_pos;
        double result = calculate(expression, &error, &error_pos);

        if (error == ERROR_NONE) {
            printf("结果: %.6f (十进制)\n", result);
            printf("      0x%lx (十六进制整数部分)\n", (long)result);
        } else {
            print_error(expression, error, error_pos);
        }
    }

    printf("感谢使用，再见！\n");
    return 0;
}
