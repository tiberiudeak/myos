#ifndef _ELF_H
#define _ELF_H 1

#include <stdint.h>

typedef uint16_t Elf32_Half;

typedef uint32_t Elf32_Word;
typedef	int32_t  Elf32_Sword;

typedef uint64_t Elf32_Xword;
typedef	int64_t  Elf32_Sxword;

typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint16_t Elf32_Section;
typedef Elf32_Half Elf32_Versym;

typedef enum {
    ET_NONE             = 0x0,
    ET_REL              = 0x1,
    ET_EXEC             = 0x2,
    ET_DYN              = 0x3,
    ET_CORE             = 0x4
} ELF_TYPES;

typedef enum {
    EM_NONE             = 0x0,
    EM_386              = 0x3
} ELF_MACHINES;

typedef enum {
    PT_NULL             = 0x0,
    PT_LOAD             = 0x1,
    PT_DYNAMIC          = 0x2
} PHDR_TYPES;

#define EI_NIDENT   (16)

// ELF header
typedef struct {
  unsigned char	e_ident[EI_NIDENT];	/* Magic number and other info */
  Elf32_Half	e_type;			/* Object file type */
  Elf32_Half	e_machine;		/* Architecture */
  Elf32_Word	e_version;		/* Object file version */
  Elf32_Addr	e_entry;		/* Entry point virtual address */
  Elf32_Off	    e_phoff;		/* Program header table file offset */
  Elf32_Off	    e_shoff;		/* Section header table file offset */
  Elf32_Word	e_flags;		/* Processor-specific flags */
  Elf32_Half	e_ehsize;		/* ELF header size in bytes */
  Elf32_Half	e_phentsize;	/* Program header table entry size */
  Elf32_Half	e_phnum;		/* Program header table entry count */
  Elf32_Half	e_shentsize;	/* Section header table entry size */
  Elf32_Half	e_shnum;		/* Section header table entry count */
  Elf32_Half	e_shstrndx;		/* Section header string table index */
} Elf32_Ehdr;

// Program header
typedef struct {
  Elf32_Word	p_type;			/* Segment type */
  Elf32_Off	    p_offset;		/* Segment file offset */
  Elf32_Addr	p_vaddr;		/* Segment virtual address */
  Elf32_Addr	p_paddr;		/* Segment physical address */
  Elf32_Word	p_filesz;		/* Segment size in file */
  Elf32_Word	p_memsz;		/* Segment size in memory */
  Elf32_Word	p_flags;		/* Segment flags */
  Elf32_Word	p_align;		/* Segment alignment */
} Elf32_Phdr;

typedef struct {
    void *physical_address;     // the physical address used for the segments
    void *virtual_address;      // the corresponding virtual address
    uint32_t num_blocks;        // number of contiguous blocks used
    struct elf_phys_mem_info *next;
} elf_phys_mem_info;

//void *load_elf(uint32_t *);
int32_t execute_elf(int, char**);
void elf_after_program_execution(int);

#endif /* !_ELF_H */

