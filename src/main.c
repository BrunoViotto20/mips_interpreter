#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define LINE_LENGTH 64

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

bool execute_instruction(char instruction[LINE_LENGTH], CPU *cpu);

int main()
{
    CPU cpu;
    cpu.program_counter = 0;

    while (true)
    {
        // Gets the instruction from the user
        char instruction_buffer[LINE_LENGTH] = {0};
        printf("%4d > ", cpu.program_counter);
        (void)fgets(instruction_buffer, sizeof(instruction_buffer), stdin);

        // Removes '\n' at the end of instruction buffer
        int length = strlen(instruction_buffer);
        if (instruction_buffer[length - 1] == '\n')
        {
            instruction_buffer[length - 1] = '\0';
        }

        bool exit = execute_instruction(instruction_buffer, &cpu);

        if (exit)
        {
            break;
        }
    }

    return 0;
}

bool execute_instruction(char instruction[LINE_LENGTH], CPU *cpu)
{
    if (strcmp(instruction, "EXIT") == 0)
    {
        return true;
    }

    return false;
}
