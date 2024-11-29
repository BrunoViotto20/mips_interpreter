#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_LENGTH 64
#define INSTRUCTION_ARGS 3

typedef struct CPU CPU;
typedef struct Registers Registers;

struct Registers
{
    /// @brief Temporary registers.
    int t[10];

    /// @brief Saved registers.
    int s[8];

    /// @brief Argument registers.
    int a[4];

    /// @brief Reserved for kernel.
    int k[2];

    /// @brief Return value registers.
    int v[2];

    /// @brief Register zero
    int zero;

    /// @brief Global pointer.
    int gp;

    /// @brief Stack pointer.
    int sp;

    /// @brief Frame pointer
    int fp;

    /// @brief Return address.
    int ra;

    /// @brief Reserved for assembler.
    int at;
};

struct CPU
{
    unsigned int program_counter;
    Registers registers;
};

void print_help();
void print_registers(Registers *registers);
void execute_instruction(char instruction[LINE_LENGTH], CPU *cpu);

void add(CPU *cpu, char **arguments, int length);
void addi(CPU *cpu, char **arguments, int length);
void addu(CPU *cpu, char **arguments, int length);
void sub(CPU *cpu, char **arguments, int length);
void subu(CPU *cpu, char **arguments, int length);
void j(CPU *cpu, char **arguments, int length);
void mult(CPU *cpu, char **arguments, int length);
void _and(CPU *cpu, char **arguments, int length);
void _or(CPU *cpu, char **arguments, int length);
void andi(CPU *cpu, char **arguments, int length);
void ori(CPU *cpu, char **arguments, int length);
void beq(CPU *cpu, char **arguments, int length);
void bne(CPU *cpu, char **arguments, int length);
void blez(CPU *cpu, char **arguments, int length);
void bgtz(CPU *cpu, char **arguments, int length);

void print_instruction_r(char *reg0, char *reg1, char *reg2, unsigned char funct);
void print_instruction_i(unsigned char opcode, char *reg0, char *reg1, short immediate);
void print_instruction_j(unsigned char opcode, int address);

int *get_register(Registers *registers, char *register_name);
char get_register_index(char *register_name);

char *trim(char *string);
bool is_whitespace(char character);

int main()
{
    CPU cpu;
    cpu.program_counter = 0;

    print_registers(&cpu.registers);

    while (true)
    {
        cpu.registers.zero = 0;

        // Gets the instruction from the user
        char instruction_buffer[LINE_LENGTH] = {0};
        printf("%11d > ", cpu.program_counter);
        (void)fgets(instruction_buffer, sizeof(instruction_buffer), stdin);

        char *instruction = trim(instruction_buffer);

        // If instruction is HELP, show supported tags
        if (strcmp(instruction, "HELP") == 0)
        {
            print_help();
            continue;
        }

        // If instruction is DEBUG, show register values
        if (strcmp(instruction, "DEBUG") == 0)
        {
            print_registers(&cpu.registers);
            continue;
        }

        // If instruction is EXIT, end program
        if (strcmp(instruction, "EXIT") == 0)
        {
            break;
        }

        execute_instruction(instruction, &cpu);

        cpu.program_counter += 4;
    }

    return 0;
}

void print_help()
{
    printf("\n");
    printf("HELP\n");
    printf("\n");

    printf("Instruções R\n");
    printf("\n");
    printf("ADD registrador0, registrador1, registrador2\n");
    printf("SUB registrador0, registrador1, registrador2\n");
    printf("ADDU registrador0, registrador1, registrador2\n");
    printf("SUBU registrador0, registrador1, registrador2\n");
    printf("MULT registrador0, registrador1, registrador2\n");
    printf("AND registrador0, registrador1, registrador2\n");
    printf("OR registrador0, registrador1, registrador2\n");
    printf("\n");

    printf("Instruções I\n");
    printf("\n");
    printf("ADDI registrador0, registrador1, imediato\n");
    printf("ANDI registrador0, registrador1, imediato\n");
    printf("ORI registrador0, registrador1, imediato\n");
    printf("BEQ registrador0, registrador1, endereço\n");
    printf("BNE registrador0, registrador1, endereço\n");
    printf("BLEZ registrador0, registrador1, endereço\n");
    printf("BGTZ registrador0, registrador1, endereço\n");
    printf("\n");

    printf("Instruções J\n");
    printf("\n");
    printf("J endereço\n");
    printf("\n");
}

void print_registers(Registers *registers)
{
    printf("+-------------------------------------------------------------------------------------------------------------------------------------------------------+\n");

    printf("| $s0: %11d | $s1: %11d | $s2: %11d | $s3: %11d | $s4: %11d | $s5: %11d | $s6: %11d | $s7: %11d |\n",
           registers->s[0],
           registers->s[1],
           registers->s[2],
           registers->s[3],
           registers->s[4],
           registers->s[5],
           registers->s[6],
           registers->s[7]);

    printf("|------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|\n");

    printf("| $a0: %11d | $a1: %11d | $a2: %11d | $a3: %11d | $v0: %11d | $v1: %11d | $k0: %11d | $k1: %11d |\n",
           registers->a[0],
           registers->a[1],
           registers->a[2],
           registers->a[3],
           registers->v[0],
           registers->v[1],
           registers->k[0],
           registers->k[1]);

    printf("|------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|\n");

    printf("| $t0: %11d | $t1: %11d | $t2: %11d | $t3: %11d | $t4: %11d | $t5: %11d | $t6: %11d | $t7: %11d |\n",
           registers->t[0],
           registers->t[1],
           registers->t[2],
           registers->t[3],
           registers->t[4],
           registers->t[5],
           registers->t[6],
           registers->t[7]);

    printf("|------------------+------------------+------------------+------------------+------------------+------------------+------------------+------------------|\n");

    printf("| $t8: %11d | $t9: %11d | $zero: %9d | $gp: %11d | $sp: %11d | $fp: %11d | $ra: %11d | $at: %11d |\n",
           registers->t[8],
           registers->t[9],
           registers->zero,
           registers->gp,
           registers->sp,
           registers->fp,
           registers->ra,
           registers->at);

    printf("+-------------------------------------------------------------------------------------------------------------------------------------------------------+\n");
}

void execute_instruction(char instruction[LINE_LENGTH], CPU *cpu)
{
    // Gets the instruction tag
    char *tag = strtok(instruction, " ");
    if (tag == NULL)
    {
        printf("ERRO: Nenhuma tag fornecida\n");
        return;
    }

    // Gets the instruction arguments
    int args_length = 0;
    char *args[INSTRUCTION_ARGS] = {0};
    while (true)
    {
        char *argument = strtok(NULL, ",");
        if (argument == NULL)
        {
            break;
        }

        if (args_length >= INSTRUCTION_ARGS)
        {
            printf("ERRO: Foram providos mais argumentos do que os %d permitidos\n", INSTRUCTION_ARGS);
            return;
        }

        args[args_length] = trim(argument);
        args_length++;
    }

    // Executes the instruction
    if (strcmp(tag, "ADD") == 0)
    {
        add(cpu, args, args_length);
    }
    else if (strcmp(tag, "ADDU") == 0)
    {
        addu(cpu, args, args_length);
    }
    else if (strcmp(tag, "ADDI") == 0)
    {
        addi(cpu, args, args_length);
    }
    else if (strcmp(tag, "SUB") == 0)
    {
        sub(cpu, args, args_length);
    }
    else if (strcmp(tag, "SUBU") == 0)
    {
        subu(cpu, args, args_length);
    }
    else if (strcmp(tag, "J") == 0)
    {
        j(cpu, args, args_length);
    }
    else if (strcmp(tag, "MULT") == 0)
    {
        mult(cpu, args, args_length);
    }
    else if (strcmp(tag, "AND") == 0)
    {
        _and(cpu, args, args_length);
    }
    else if (strcmp(tag, "OR") == 0)
    {
        _or(cpu, args, args_length);
    }
    else if (strcmp(tag, "ANDI") == 0)
    {
        andi(cpu, args, args_length);
    }
    else if (strcmp(tag, "ORI") == 0)
    {
        ori(cpu, args, args_length);
    }
    else if (strcmp(tag, "BEQ") == 0)
    {
        beq(cpu, args, args_length);
    }
    else if (strcmp(tag, "BNE") == 0)
    {
        bne(cpu, args, args_length);
    }
    else if (strcmp(tag, "BLEZ") == 0)
    {
        blez(cpu, args, args_length);
    }
    else if (strcmp(tag, "BGTZ") == 0)
    {
        bgtz(cpu, args, args_length);
    }
    else
    {
        printf("ERRO: \"%s\" não é uma tag válida\n", tag);
        cpu->program_counter -= 4;
    }
}

void add(CPU *cpu, char **arguments, int length)
{
    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d\n", length);
        return;
    }

    int *ret = get_register(&cpu->registers, arguments[0]);
    int *arg1 = get_register(&cpu->registers, arguments[1]);
    int *arg2 = get_register(&cpu->registers, arguments[2]);

    if (ret == NULL || arg1 == NULL || arg2 == NULL)
    {
        printf("ERRO: Instrução inválida, registrador não encontrado\n");
        return;
    }

    *ret = *arg1 + *arg2;

    print_instruction_r(arguments[0], arguments[1], arguments[2], 0x20);
}

void addi(CPU *cpu, char **arguments, int length)
{
    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d\n", length);
        return;
    }

    int *ret = get_register(&cpu->registers, arguments[0]);
    int *arg1 = get_register(&cpu->registers, arguments[1]);

    if (ret == NULL || arg1 == NULL)
    {
        printf("ERRO: Instrução inválida, registrador não encontrado\n");
        return;
    }

    errno = 0;
    char *endarg2;
    int arg2 = strtol(arguments[2], &endarg2, 10);

    if (errno != 0 || arguments[2] == endarg2 || *endarg2 != '\0')
    {
        printf("ERRO: Instrução inválida, número imediato inválido\n");
    }

    *ret = *arg1 + arg2;

    print_instruction_i(0x8, arguments[0], arguments[1], arg2);
}

void addu(CPU *cpu, char **arguments, int length)
{
    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d\n", length);
        return;
    }

    unsigned int *ret = (unsigned int *)get_register(&cpu->registers, arguments[0]);
    unsigned int *arg1 = (unsigned int *)get_register(&cpu->registers, arguments[1]);
    unsigned int *arg2 = (unsigned int *)get_register(&cpu->registers, arguments[2]);

    if (ret == NULL || arg1 == NULL || arg2 == NULL)
    {
        printf("ERRO: Instrução inválida, registrador não encontrado\n");
        return;
    }

    *ret = *arg1 + *arg2;

    print_instruction_r(arguments[0], arguments[1], arguments[2], 0x21);
}

void sub(CPU *cpu, char **arguments, int length)
{
    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d\n", length);
        return;
    }

    int *ret = get_register(&cpu->registers, arguments[0]);
    int *arg1 = get_register(&cpu->registers, arguments[1]);
    int *arg2 = get_register(&cpu->registers, arguments[2]);

    if (ret == NULL || arg1 == NULL || arg2 == NULL)
    {
        printf("ERRO: Instrução inválida, registrador não encontrado\n");
        return;
    }

    *ret = *arg1 - *arg2;

    print_instruction_r(arguments[0], arguments[1], arguments[2], 0x22);
}

void subu(CPU *cpu, char **arguments, int length)
{
    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d\n", length);
        return;
    }

    unsigned int *ret = (unsigned int *)get_register(&cpu->registers, arguments[0]);
    unsigned int *arg1 = (unsigned int *)get_register(&cpu->registers, arguments[1]);
    unsigned int *arg2 = (unsigned int *)get_register(&cpu->registers, arguments[2]);

    if (ret == NULL || arg1 == NULL || arg2 == NULL)
    {
        printf("ERRO: Instrução inválida, registrador não encontrado\n");
        return;
    }

    *ret = *arg1 - *arg2;

    print_instruction_r(arguments[0], arguments[1], arguments[2], 0x23);
}

void j(CPU *cpu, char **arguments, int length)
{
    if (length != 1)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 1 e foram recebidos %d\n", length);
        return;
    }

    errno = 0;
    char *end;
    int arg = strtol(arguments[0], &end, 10);

    if (errno != 0 || arguments[0] == end || *end != '\0')
    {
        printf("ERRO: Instrução inválida, endereço inválido\n");
    }

    cpu->program_counter = arg - 4;

    print_instruction_j(0x2, arg);
}

void mult(CPU *cpu, char **arguments, int length)
{
    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d\n", length);
        return;
    }

    int *ret = get_register(&cpu->registers, arguments[0]);
    int *arg1 = get_register(&cpu->registers, arguments[1]);
    int *arg2 = get_register(&cpu->registers, arguments[2]);

    if (ret == NULL || arg1 == NULL || arg2 == NULL)
    {
        printf("ERRO: Instrução inválida, registrador não encontrado\n");
        return;
    }

    *ret = *arg1 * *arg2;

    print_instruction_r(arguments[0], arguments[1], arguments[2], 0x18);
}

void _and(CPU *cpu, char **arguments, int length)
{
    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d\n", length);
        return;
    }

    int *ret = get_register(&cpu->registers, arguments[0]);
    int *arg1 = get_register(&cpu->registers, arguments[1]);
    int *arg2 = get_register(&cpu->registers, arguments[2]);

    if (ret == NULL || arg1 == NULL || arg2 == NULL)
    {
        printf("ERRO: Instrução inválida, registrador não encontrado\n");
        return;
    }

    *ret = *arg1 & *arg2;

    print_instruction_r(arguments[0], arguments[1], arguments[2], 0x24);
}

void _or(CPU *cpu, char **arguments, int length)
{
    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d\n", length);
        return;
    }

    int *ret = get_register(&cpu->registers, arguments[0]);
    int *arg1 = get_register(&cpu->registers, arguments[1]);
    int *arg2 = get_register(&cpu->registers, arguments[2]);

    if (ret == NULL || arg1 == NULL || arg2 == NULL)
    {
        printf("ERRO: Instrução inválida, registrador não encontrado\n");
        return;
    }

    *ret = *arg1 | *arg2;

    print_instruction_r(arguments[0], arguments[1], arguments[2], 0x25);
}

void andi(CPU *cpu, char **arguments, int length)
{
    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d\n", length);
        return;
    }

    int *ret = get_register(&cpu->registers, arguments[0]);
    int *arg1 = get_register(&cpu->registers, arguments[1]);

    if (ret == NULL || arg1 == NULL)
    {
        printf("ERRO: Instrução inválida, registrador não encontrado\n");
        return;
    }

    errno = 0;
    char *endarg2;
    int arg2 = strtol(arguments[2], &endarg2, 10);

    if (errno != 0 || arguments[2] == endarg2 || *endarg2 != '\0')
    {
        printf("ERRO: Instrução inválida, número imediato inválido\n");
    }

    *ret = *arg1 & arg2;

    print_instruction_i(0x0C, arguments[0], arguments[1], arg2);
}

void ori(CPU *cpu, char **arguments, int length)
{

    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d\n", length);
        return;
    }

    int *ret = get_register(&cpu->registers, arguments[0]);
    int *arg1 = get_register(&cpu->registers, arguments[1]);

    if (ret == NULL || arg1 == NULL)
    {
        printf("ERRO: Instrução inválida, registrador não encontrado\n");
        return;
    }

    errno = 0;
    char *endarg2;
    int arg2 = strtol(arguments[2], &endarg2, 10);

    if (errno != 0 || arguments[2] == endarg2 || *endarg2 != '\0')
    {
        printf("ERRO: Instrução inválida, número imediato inválido\n");
    }

    *ret = *arg1 | arg2;

    print_instruction_i(0x0D, arguments[0], arguments[1], arg2);
}

void beq(CPU *cpu, char **arguments, int length)
{
    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d\n", length);
        return;
    }

    int *arg1 = get_register(&cpu->registers, arguments[0]);
    int *arg2 = get_register(&cpu->registers, arguments[1]);

    if (arg1 == NULL || arg2 == NULL)
    {
        printf("ERRO: Instrução inválida, registrador não encontrado\n");
        return;
    }

    errno = 0;
    char *endlabel;
    int label = strtol(arguments[2], &endlabel, 10);

    if (errno != 0 || arguments[2] == endlabel || *endlabel != '\0')
    {
        printf("ERRO: Instrução inválida, número imediato inválido\n");
    }

    if (*arg1 == *arg2)
    {
        cpu->program_counter = label - 4;
    }

    print_instruction_j(0x4, label);
}

void bne(CPU *cpu, char **arguments, int length)
{
    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d\n", length);
        return;
    }

    int *arg1 = get_register(&cpu->registers, arguments[0]);
    int *arg2 = get_register(&cpu->registers, arguments[1]);

    if (arg1 == NULL || arg2 == NULL)
    {
        printf("ERRO: Instrução inválida, registrador não encontrado\n");
        return;
    }

    errno = 0;
    char *endlabel;
    int label = strtol(arguments[2], &endlabel, 10);

    if (errno != 0 || arguments[2] == endlabel || *endlabel != '\0')
    {
        printf("ERRO: Instrução inválida, número imediato inválido\n");
    }

    if (*arg1 != *arg2)
    {
        cpu->program_counter = label - 4;
    }

    print_instruction_j(0x5, label);
}

void blez(CPU *cpu, char **arguments, int length)
{

    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d\n", length);
        return;
    }

    int *arg1 = get_register(&cpu->registers, arguments[0]);
    int *arg2 = get_register(&cpu->registers, arguments[1]);

    if (arg1 == NULL || arg2 == NULL)
    {
        printf("ERRO: Instrução inválida, registrador não encontrado\n");
        return;
    }

    errno = 0;
    char *endlabel;
    int label = strtol(arguments[2], &endlabel, 10);

    if (errno != 0 || arguments[2] == endlabel || *endlabel != '\0')
    {
        printf("ERRO: Instrução inválida, número imediato inválido\n");
    }

    if (*arg1 <= *arg2)
    {
        cpu->program_counter = label - 4;
    }

    print_instruction_j(0x6, label);
}

void bgtz(CPU *cpu, char **arguments, int length)
{

    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d\n", length);
        return;
    }

    int *arg1 = get_register(&cpu->registers, arguments[0]);
    int *arg2 = get_register(&cpu->registers, arguments[1]);

    if (arg1 == NULL || arg2 == NULL)
    {
        printf("ERRO: Instrução inválida, registrador não encontrado\n");
        return;
    }

    errno = 0;
    char *endlabel;
    int label = strtol(arguments[2], &endlabel, 10);

    if (errno != 0 || arguments[2] == endlabel || *endlabel != '\0')
    {
        printf("ERRO: Instrução inválida, número imediato inválido\n");
    }

    if (*arg1 > *arg2)
    {
        cpu->program_counter = label - 4;
    }

    print_instruction_j(0x7, label);
}

int *get_register(Registers *registers, char *register_name)
{
    if (register_name[0] != '$')
    {
        return NULL;
    }

    char first = register_name[1];
    char second = register_name[2];

    switch (first)
    {
    case 't':
    case 's':
    case 'a':
    case 'k':
    case 'v':
        if (second < '0' || second > '9')
        {
            break;
        }
        int index = second - '0';

        if (first == 't')
        {
            return &registers->t[index];
        }
        if (first == 's' && index < 8)
        {
            return &registers->s[index];
        }
        if (first == 'a' && index < 4)
        {
            return &registers->a[index];
        }
        if (first == 'k' && index < 2)
        {
            return &registers->k[index];
        }
        if (first == 'v' && index < 2)
        {
            return &registers->v[index];
        }
        return NULL;
    }

    if (strcmp(register_name, "$zero") == 0)
    {
        return &registers->zero;
    }
    if (strcmp(register_name, "$gp") == 0)
    {
        return &registers->gp;
    }
    if (strcmp(register_name, "$sp") == 0)
    {
        return &registers->sp;
    }
    if (strcmp(register_name, "$fp") == 0)
    {
        return &registers->fp;
    }
    if (strcmp(register_name, "$ra") == 0)
    {
        return &registers->ra;
    }
    if (strcmp(register_name, "$at") == 0)
    {
        return &registers->at;
    }

    return NULL;
}

char get_register_index(char *register_name)
{
    if (register_name[0] != '$')
    {
        return -1;
    }

    char first = register_name[1];
    char second = register_name[2];

    if (strcmp(register_name, "$zero") == 0)
    {
        return 0;
    }

    if (strcmp(register_name, "$at") == 0)
    {
        return 1;
    }

    if (first == 'v' && second - '0' < 2)
    {
        return 2 + second - '0';
    }

    if (first == 'a' && second - '0' < 4)
    {
        return 4 + second - '0';
    }

    if (first == 't' && second - '0' < 8)
    {
        return 8 + second - '0';
    }

    if (first == 's' && second - '0' < 8)
    {
        return 16 + second - '0';
    }

    if (first == 't' && second - '8' < 2)
    {
        return 24 + second - '8';
    }

    if (first == 'k' && second - '0' < 8)
    {
        return 26 + second - '0';
    }

    if (strcmp(register_name, "$gp") == 0)
    {
        return 28;
    }

    if (strcmp(register_name, "$sp") == 0)
    {
        return 29;
    }

    if (strcmp(register_name, "$fp") == 0)
    {
        return 30;
    }

    if (strcmp(register_name, "$ra") == 0)
    {
        return 31;
    }

    printf("ERRO: Registrador não encontrado");
    return -1;
}

char *trim(char *string)
{
    while (true)
    {
        if (!is_whitespace(string[0]))
        {
            break;
        }
        string = string + 1;
    }

    int length = strlen(string);
    while (length > 0)
    {
        if (!is_whitespace(string[length - 1]))
        {
            break;
        }
        string[length - 1] = '\0';
        length--;
    }

    return string;
}

bool is_whitespace(char character)
{
    switch (character)
    {
    case ' ':
    case '\n':
        return true;
    default:
        return false;
    }
}

void print_instruction_r(char *reg0, char *reg1, char *reg2, unsigned char funct)
{
    char reg0_index = get_register_index(reg0);
    char reg1_index = get_register_index(reg1);
    char reg2_index = get_register_index(reg2);

    if (reg0_index == -1 || reg1_index == -1 || reg2_index == -1)
    {
        printf("ERRO: Registrador não encontrado\n");
        return;
    }

    printf("EXECUTE -> 0 %d %d %d 0 %d\n", reg0_index, reg1_index, reg2_index, funct);
}

void print_instruction_i(unsigned char opcode, char *reg0, char *reg1, short immediate)
{
    char reg0_index = get_register_index(reg0);
    char reg1_index = get_register_index(reg1);

    if (reg0_index == -1 || reg1_index == -1)
    {
        printf("ERRO: Registrador não encontrado\n");
        return;
    }

    printf("EXECUTE -> %d %d %d %d\n", opcode, reg0_index, reg1_index, immediate);
}

void print_instruction_j(unsigned char opcode, int address)
{
    printf("EXECUTE -> %d %d\n", opcode, address);
}
