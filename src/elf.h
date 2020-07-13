#pragma once



struct elf {
    uint64_t entry;         //Entry address of executable
    uint64_t text_base;     //Base address of code segment
    uint64_t data_base;     //Base address of rw data segment
    uint64_t bss_base;      //Base address of bss segment

    uint8_t *text_buf;      //Program .text buffer
    size_t text_n, text_c;  //text_buf size and capacity

    uint8_t *data_buf;      //Program .data buffer
    size_t data_n, data_c;  //data_buf size and capacity

    uint64_t bss_size;      //Requested size of zero'd memory
};

void elf_init(struct elf *elf);
char *elf_write(struct elf *elf, char *path);
int elf_append_text(struct elf *elf, void *data, size_t size);
int elf_append_data(struct elf *elf, void *data, size_t size);
