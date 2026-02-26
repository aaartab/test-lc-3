#include "vm.h"
#include <stdint.h>

uint16_t memory[MEMORY_MAX];

uint16_t reg[R_COUNT];

extern int running;

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

void update_flags(uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15)
    {
        reg[R_COND] = FL_NEG;
    }
    else 
    {
        reg[R_COND] = FL_POS;
    }
}

void add(uint16_t instr)
{
    uint16_t dr, sr1, sr2;
    dr = (instr >> 9) & 0x7;
    sr1 = (instr >> 6) & 0x7;
    
    if ((instr >> 5) & 0x1)
    {
       reg[dr] = reg[sr1] + sign_extend(instr & 0x1F, 5);
    }
    else 
    {
        sr2 = instr & 0x7;
        reg[dr] = reg[sr1] + reg[sr2];
    }

    update_flags(dr);
}

void ldi(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

    reg[dr] = mem_read(mem_read(reg[R_PC] + pc_offset));
    update_flags(dr);
}

void ld(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

    reg[dr] = mem_read(reg[R_PC] + pc_offset);
    update_flags(dr);
}

void st(uint16_t instr)
{
    uint16_t sr = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    mem_write(reg[R_PC] + pc_offset, reg[sr]);
}

void jsr(uint16_t instr)
{
    uint16_t mode = (instr >> 11) & 0x1; 
    reg[R_R7] = reg[R_PC];
    if (mode == 0) 
    {
        reg[R_PC] =  reg[(instr >> 6) & 0x7];
    }
    else 
    {
        reg[R_PC] += sign_extend(instr & 0x7FF, 11);
    }
}

void ando(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t sr1 = (instr >> 6) & 0x7;
    uint16_t mode = (instr >> 5) & 0x1;

    if (mode == 0)
    {
        uint16_t sr2 = instr & 0x7;
        reg[dr] = reg[sr1] & reg[sr2];
    }
    else 
    {
       reg[dr] = reg[sr1] & sign_extend(instr & 0x1F, 5); 
    }
    
    update_flags(dr);
}

void ldr(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t base_r = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F, 6);

    reg[dr] = mem_read(reg[base_r]+ offset); 

    update_flags(dr);
}

void str(uint16_t instr)
{
    uint16_t sr = (instr >> 9) & 0x7;
    uint16_t base_r = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F, 6);

    mem_write(reg[base_r] + offset, reg[sr]);
}

void rti(uint16_t instr)
{
    abort();
}

void res(uint16_t instr)
{
    abort();
}

void noto(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t sr = (instr >> 6) & 0x7;

    reg[dr] = ~reg[sr];
    update_flags(dr);
}

void sti(uint16_t instr)
{
    uint16_t sr = (instr >> 9) & 0x7;
    uint16_t pc_offset = instr & 0x1FF;

    mem_write(mem_read(reg[R_PC] + sign_extend(pc_offset, 9)), reg[sr]);
}

void jmp(uint16_t instr)
{
    uint16_t dr = (instr >> 6) & 0x7;
    reg[R_PC] = reg[dr];
}

void lea(uint16_t instr)
{
    uint16_t dr = (instr >> 9) & 0x7;
    uint16_t pc_offset = instr & 0x1FF;

    reg[dr] = reg[R_PC] + sign_extend(pc_offset, 9);

    update_flags(dr);
}

void br(uint16_t instr)
{
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    uint16_t cond_flag = (instr >> 9) & 0x7;
    if (cond_flag & reg[R_COND])
    {
        reg[R_PC] += pc_offset;
    }
}

void trap(uint16_t instr)
{
    reg[R_R7] = reg[R_PC];
    
    switch(instr & 0xFF)
    {
        case TRAP_GETC:
            trap_getc();
            break;
        case TRAP_OUT:
            trap_out();
            break;
        case TRAP_PUTS:
            trap_puts();
            break;
        case TRAP_IN:
            trap_in();
            break;
        case TRAP_PUTSP:
            trap_putsp();
            break;
        case TRAP_HALT:
            trap_halt();
            break;
    }
}


void trap_puts()
{
    uint16_t* c = memory + reg[R_R0];
    while(*c)
    {
        putc((char)*c, stdout);
        ++c;
    }
    fflush(stdout);
}

void trap_getc()
{
    reg[R_R0] = (uint16_t)getchar();
    update_flags(R_R0);
}

void trap_putsp()
{
    uint16_t* c = memory + reg[R_R0];
    while(*c)
    {
        char char1 = (*c) & 0xFF;
        putc(char1, stdout);
        char char2 = (*c) >> 8;
        if (char2) putc(char2, stdout);
        ++c;
    }
}

void trap_out()
{
    putc((char)reg[R_R0], stdout);
    fflush(stdout);
}

void trap_in()
{
    printf("Enter a character: ");
    char c = getchar();
    putc(c, stdout);
    fflush(stdout);
    reg[R_R0] = (uint16_t)c;
    update_flags(R_R0);
}

void trap_halt()
{
    puts("HALT");
    fflush(stdout);
    running = 0;
}

uint16_t swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

void read_image_file(FILE* file)
{
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    uint16_t max_read = MEMORY_MAX - origin;
    uint16_t* p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    while (read-- > 0) 
    {
        *p = swap16(*p);
        ++p;
    }
}

int read_image(const char* image_path)
{
    FILE* file = fopen(image_path, "rb");
    if (!file) { return 0; };
    read_image_file(file);
    fclose(file);
    return 1;
}


void mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

uint16_t mem_read(uint16_t address)
{
    if (address == MR_KBSR)
    {
        if (check_key())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
        else 
        {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[address];
}
