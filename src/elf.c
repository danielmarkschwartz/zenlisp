#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "elf.h"

#define ELF_MAGIC       "\x7F""ELF"
#define ELF_MAGIC_SIZE  4
#define ELF_CLASS64     2
#define ELF_DATA_LSB    1
#define ELF_PAD         0,0,0,0,0,0,0,0
#define ELF_PAD_SIZE    8
#define ELF_EXECUTABLE  2
#define ELF_ARM64       183
#define ELF_VER         1
#define ELF_FLAGS_NONE  0

#define ELF_LOAD        1
#define ELF_EXECUTE     0x1
#define ELF_WRITE       0x2
#define ELF_READ        0x4

#define ELF_HEADER_SIZE     0x40
#define ELF_PROGHEAD_SIZE   0x38
#define ELF_SECTHEAD_SIZE   0x40

#define ARM_ALIGN           4
#define DEFAULT_TEXT_BASE   0x800000
#define DEFAULT_DATA_BASE   0x400000
#define DEFAULT_BSS_BASE    0x200000

#define BUF_DEFAULT         1024

struct elf_header {
    uint8_t  magic[ELF_MAGIC_SIZE];
    uint8_t  class;
    uint8_t  data;
    uint8_t  version;
    uint8_t  pad[ELF_PAD_SIZE];
    uint16_t type;
    uint16_t machine;
    uint32_t version1;
    uint64_t entry;
    uint64_t phoff;
    uint64_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
};


struct elf_prog_header {
    uint32_t type;
	uint32_t flags;
	uint64_t offset;
	uint64_t vaddr;
	uint64_t paddr;
	uint64_t filesz;
	uint64_t memsz;
	uint64_t align;
};

struct elf_sect_header {
    uint32_t name;      // Index of name string in .shstrtab
    uint32_t type;
    uint64_t flags;
    uint64_t addr;      // Virtual address of section in memory
    uint64_t offset;    // Offset to location in file
    uint64_t size;      // Size of data in file
    uint32_t link;      // Depends on section type
    uint32_t info;      // Depends on section type
    uint64_t align;     // Alignment of section
    uint64_t entsize;   // Size of entry if applicable to section
};


/*
Elf64_Addr	8	8	Unsigned program address
Elf64_Off	8	8	Unsigned file offset
Elf64_Half	2	2	Unsigned medium integer
Elf64_Word	4	4	Unsigned integer
Elf64_Sword	4	4	Signed integer
Elf64_Xword	8	8	Unsigned long integer
Elf64_Sxword	8	8	Signed long integer
unsigned char	1	1	Unsigned small integer
*/

void elf_init(struct elf *elf) {

    //Make sure the compiler generated correctly padded structures
    assert(sizeof (struct elf_header) == ELF_HEADER_SIZE);
    assert(sizeof (struct elf_prog_header) == ELF_PROGHEAD_SIZE);
    assert(sizeof (struct elf_sect_header) == ELF_SECTHEAD_SIZE);

    memset(elf, 0, sizeof *elf);
    *elf = (struct elf) {
        .entry     = DEFAULT_TEXT_BASE,
        .text_base = DEFAULT_TEXT_BASE,
        .data_base = DEFAULT_DATA_BASE,
        .bss_base  = DEFAULT_BSS_BASE
    };
}

char *elf_write(struct elf *elf, char *path) {
    assert(elf); assert(path);

    FILE *f = fopen(path, "wb");
    if(!f) return "Could not open file";

    uint32_t phoff = ELF_HEADER_SIZE;
    uint32_t phnum = 3;     // (text, data, bss)
    uint32_t shoff = phoff + phnum * ELF_PROGHEAD_SIZE;
    uint32_t shnum = 0;
    uint32_t shstrndx = 0;  // section index of section name strings
    uint32_t data_start = shoff + shnum * ELF_SECTHEAD_SIZE;
    shoff = 0;

    struct elf_header hdr = {
        ELF_MAGIC,
        ELF_CLASS64,        //arm64 must use 64 bit ELF version
        ELF_DATA_LSB,       //little endian data
        ELF_VER,
        ELF_PAD,
        ELF_EXECUTABLE,     //executable file
        ELF_ARM64,          //arm64 code
        ELF_VER,
        elf->entry,         //program entry location in virtual memory
        phoff,
        shoff,
        ELF_FLAGS_NONE,     //no flags defined for arm64
        ELF_HEADER_SIZE,
        ELF_PROGHEAD_SIZE,
        phnum,
        ELF_SECTHEAD_SIZE,
        shnum,
        shstrndx
    };

    if(fwrite(&hdr, sizeof hdr, 1, f) != 1)
        return "Could not write header";

    // Generate text section
    struct elf_prog_header phdr = {
        ELF_LOAD,
        ELF_READ | ELF_EXECUTE,
        data_start,     //Location in file
        elf->text_base, //Virtual address in ram
        0,              //Physical address, don't care
        elf->text_n,    //Size on disk
        elf->text_n,    //Size in memory
        ARM_ALIGN       //Arm requires 4 byte alignment
    };

    data_start += elf->text_n;

    if(fwrite(&phdr, sizeof phdr, 1, f) != 1)
        return "Could not write .text header";

    // Generate .text section
    phdr = (struct elf_prog_header){
        ELF_LOAD,
        ELF_READ | ELF_WRITE,
        data_start,     //Location in file
        elf->data_base, //Virtual address in ram
        0,              //Physical address, don't care
        elf->data_n,    //Size on disk
        elf->data_n,    //Size in memory
        ARM_ALIGN       //Arm requires 4 byte alignment
    };

    if(fwrite(&phdr, sizeof phdr, 1, f) != 1)
        return "Could not write .data header";

    // Generate .bss section
    phdr = (struct elf_prog_header){
        ELF_LOAD,
        ELF_READ | ELF_WRITE,
        0,              //No data in file for bss
        elf->bss_base,  //Virtual address in ram
        0,              //Physical address, don't care
        0,              //Size on disk - none for bss
        elf->bss_size,  //Size in memory
        ARM_ALIGN       //Arm requires 4 byte alignment
    };

    if(fwrite(&phdr, sizeof phdr, 1, f) != 1)
        return "Could not write .bss header";

    if(fwrite(elf->text_buf, elf->text_n, 1, f) != 1)
        return "Could not write .text buffer";

    if(fwrite(elf->data_buf, elf->data_n, 1, f) != 1)
        return "Could not write .data buffer";

    fclose(f);

    return NULL;
}

int elf_append_text(struct elf *elf, void *data, size_t size) {
    assert(elf); assert(data);

    if(size + elf->text_n > elf->text_c) {
        while(elf->text_c < size + elf->text_n)
            elf->text_c = (elf->text_c == 0)? BUF_DEFAULT : elf->text_c * 2;
        elf->text_buf = reallocf(elf->text_buf, elf->text_c);
    }

    memcpy(&elf->text_buf[elf->text_n], data, size);

    uint64_t ret = elf->text_n;
    elf->text_n += size;
    return ret;
}

int elf_append_data(struct elf *elf, void *data, size_t size) {
    assert(elf); assert(data);

    if(size + elf->data_n > elf->data_c) {
        while(elf->data_c < size + elf->data_n)
            elf->data_c = (elf->data_c == 0)? BUF_DEFAULT : elf->data_c * 2;
        elf->data_buf = reallocf(elf->data_buf, elf->data_c);
    }

    memcpy(&elf->data_buf[elf->data_n], data, size);

    uint64_t ret = elf->data_n;
    elf->data_n += size;
    return ret;
}
