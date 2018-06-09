#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <elf.h>

#define BUFFER_SIZE 100
#define OPTIONS_NUM 3
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
        perror("Cannot open file");
        CLOSE_ELF;
        return;
    }

    if (0 != fstat(Currentfd, &fd_stat)) {
        perror("Cannot ferch stats");
        CLOSE_ELF;
        return;
    }

    if (MAP_FAILED == (map_start = mmap(NULL, fd_stat.st_size, PROT_READ ,MAP_SHARED, Currentfd, 0))) {
        perror("Cannot map file");
        CLOSE_ELF;
        return;
    }

    Elf64_Ehdr *header = (Elf64_Ehdr *) map_start;
    printf("\nMagic numbers: %c%c%c\n", header->e_ident[1], header->e_ident[2], header->e_ident[3]);
    if ('E' != header->e_ident[1] || 'L' != header->e_ident[2] || 'F' != header->e_ident[3]){
        printf("Not an ELF file!\n\n");
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
    if (-1 == Currentfd){
        printf("Invalid file\n\n");
        return;
    }
    Elf64_Ehdr *header = (Elf64_Ehdr *) map_start;
    Elf64_Shdr *sh_table = (Elf64_Shdr *) (map_start + header->e_shoff);
    char *shstrtab = (char *)header + sh_table[header->e_shstrndx].sh_offset;
    for (int i = 0; i < header->e_shnum; i++){
        Elf64_Shdr s = sh_table[i];
        printf("[%d] %s %p %lu %lu %d\n", i, shstrtab + s.sh_name, (void *)s.sh_addr, s.sh_offset, s.sh_size, s.sh_type);
    }
    printf("\n");
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
    Option menu[OPTIONS_NUM] = {{"1-Examine ELF File\n", &examine_elf_file}, {"2-Print Section Names\n", &print_sections}, {"3-Quit\n", &quit}};
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
