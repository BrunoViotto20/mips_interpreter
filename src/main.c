#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define LINE_LENGTH 64
#define INSTRUCTION_ARGS 3

typedef struct
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
} Registers;

typedef struct
{
    int program_counter;
    Registers registers;
} CPU;

void print_registers(Registers *registers);
void execute_instruction(char instruction[LINE_LENGTH], CPU *cpu);

void add(CPU *cpu, char **arguments, int length);

int *get_register(Registers *registers, char *register_name);

char *trim(char *string);
bool is_whitespace(char character);

int main()
{
    CPU cpu;
    cpu.program_counter = 0;

    print_registers(&cpu.registers);

    while (true)
    {
        // Gets the instruction from the user
        char instruction_buffer[LINE_LENGTH] = {0};
        printf("%4d > ", cpu.program_counter);
        (void)fgets(instruction_buffer, sizeof(instruction_buffer), stdin);

        char *instruction = trim(instruction_buffer);

        // If instruction is EXIT, end program
        if (strcmp(instruction, "SHOW") == 0)
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
    }

    return 0;
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

    cpu->registers.zero = 0;

    // Executes the instruction
    if (strcmp(tag, "ADD") == 0)
    {
        add(cpu, args, args_length);
    }
    else
    {
        printf("ERRO: \"%s\" não é uma tag válida\n", tag);
    }
}

void add(CPU *cpu, char **arguments, int length)
{
    if (length != 3)
    {
        printf("ERRO: Quantidade inesperada de argumetos, eram esperados 3 e foram recebidos %d", length);
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
