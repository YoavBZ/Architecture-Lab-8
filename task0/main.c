#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <elf.h>

#define BUFFER_SIZE 100
#define OPTIONS_NUM 2
#define CLOSE_ELF                        \
    close(Currentfd);                    \
    munmap(map_start, fd_stat.st_size);  \
    Currentfd = -1;

int Currentfd = -1;
char filename[BUFFER_SIZE];
void *map_start;
struct stat fd_stat;
int map_size = 0;

void examine_elf_file(){
    if (-1 != Currentfd){
        CLOSE_ELF;
    }

    printf("Please enter a filename:\n");
    scanf("%s", filename);
    Currentfd = open(filename, O_RDONLY);
    if (0 >= Currentfd){
        printf("Cannot open file: %s\n", filename);
        CLOSE_ELF;
        return;
    }

    if (0 != fstat(Currentfd, &fd_stat)) {
        CLOSE_ELF;
        return;
    }

    if (MAP_FAILED == (map_start = mmap(NULL, fd_stat.st_size, PROT_READ ,MAP_SHARED, Currentfd, 0))) {
        CLOSE_ELF;
        return;
    }

    Elf64_Ehdr *header = (Elf64_Ehdr *) map_start;
    printf("\nMagic numbers: %c%c%c\n", header->e_ident[1], header->e_ident[2], header->e_ident[3]);
    if ('E' != header->e_ident[1] || 'L' != header->e_ident[2] || 'F' != header->e_ident[3]){
        CLOSE_ELF;
        return;
    }
    printf("Data encoding: %s\n", header->e_ident[5] == 1 ? "Little Endian" : "Big Endian");
    printf("Entry point: %p\n", (void *)header->e_entry);
    printf("Section header offset: %lu\n", header->e_shoff);
    printf("Number of section header entries: %u\n\n", header->e_shnum);
    printf("Section Headers:\n");
    for (int i = 0; i < header->e_shnum; i++){
        printf("[%d] size: %lu\n", i, ((Elf64_Shdr *) (map_start + header->e_shoff))[i].sh_size);
    }
    printf("\nProgram header offset: %lu\n\n", header->e_phoff);
    printf("Program Headers:\n");
    Elf64_Phdr *ph_table = (Elf64_Phdr *) (map_start + header->e_phoff);
    for (int i = 0; i < header->e_phnum; i++){
        printf("[%d] size: %lu\n", i, ph_table[i].p_filesz);
    }
    printf("\n");
}

void print_sections(){
    Elf64_Ehdr *header = (Elf64_Ehdr *) map_start;
    Elf64_Shdr *sh_table = (Elf64_Shdr *) (map_start + header->e_shoff);
    Elf64_Shdr *shstrtab = sh_table + header->e_shstrndx * sizeof(Elf64_Shdr);
}

void quit(){
    CLOSE_ELF;
    exit(0);
}

typedef struct Option {
    char *name;
    void (*function)();
} Option;

int main(int argc, char **argv){
    Option menu[OPTIONS_NUM] = {{"1-Examine ELF File\n", &examine_elf_file}, {"2-Quit\n", &quit}};
    while (1){
        for (int i = 0; i < OPTIONS_NUM; i++){
            printf("%s", menu[i].name);
        }
        int selected;
        scanf("%d", &selected);
        if (0 < selected && OPTIONS_NUM >= selected)
            menu[selected-1].function();
    }
    return 0;
}
