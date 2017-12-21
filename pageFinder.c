#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BASE_ADDR 0xc1000000
#define PAGE_SIZE 4096

#define MIN(a,b) (((a)<(b)) ? (a):(b))
#define MAX(a,b) (((a)>(b)) ? (a):(b))

struct _page_sig{
    uint32_t offset;
    uint32_t value;
};

int sig_cnt = 0;

int main(int argc, char **argv)
{
    char buf[PAGE_SIZE];

    if (argc<4){
        printf("Usage: ./pageFinder <page offset file> <text section of elf> <memory dump file>\n");
        exit(0);
    }   

    FILE* sig_f = fopen(argv[1], "r");
    if (sig_f == NULL){
        printf("Can't open file %s\n", argv[1]);
        exit(0);
    }
    FILE* elf_f = fopen(argv[2], "rb");
    if (elf_f == NULL){
        printf("Can't open file %s\n", argv[2]);
        exit(0);
    }

    FILE* dump_f = fopen(argv[3], "rb");
    if (dump_f == NULL){
        printf("Can't open file %s\n", argv[3]);
        exit(0);
    }
    
    fread(buf, 1, PAGE_SIZE, elf_f);    

    struct _page_sig page_sig[500];
    char offset[10];
    while(fscanf(sig_f, "%s", offset) != EOF){
        page_sig[sig_cnt].offset = (uint32_t)strtol(offset, NULL, 16) - BASE_ADDR;

        page_sig[sig_cnt].value = *(uint32_t*)(buf+page_sig[sig_cnt].offset);
//        printf("%x, %08x\n", page_sig[sig_cnt].offset, page_sig[sig_cnt].value);
        sig_cnt++;
    }
    printf("signature count: %d\n", sig_cnt);
//    getchar();

    int page_idx = 0;

    while(PAGE_SIZE == fread(buf, 1, PAGE_SIZE, dump_f)){
/*
        if(page_idx != 0x13001){
            page_idx++;
            continue;
        }
*/
        
        uint32_t diff_list[sig_cnt][2];
        memset(diff_list, 0, sig_cnt*2*sizeof(uint32_t));
        int diff_cnt=0;
        
        for(int i=0; i<sig_cnt; i++){
            uint32_t diff = MAX(*(uint32_t*)(buf+page_sig[i].offset),(uint32_t)page_sig[i].value) - MIN(*(uint32_t*)(buf+page_sig[i].offset),(uint32_t)page_sig[i].value); 
/*
            if (page_idx == 0x13001){
                printf("%x, %x\n", page_sig[i].offset, diff);
            }
*/
            int j=0;
            for(; j<diff_cnt; j++){
                if (diff == diff_list[j][0]){
                    diff_list[j][1]++;
                    break;
                } 
            }
            if(j==diff_cnt){
                diff_list[j][0] = diff;
                diff_list[j][1] = 1;
                diff_cnt++;
            }
        }
        int most=0;
        int most_idx=0;

        for(int i=0; i<diff_cnt; i++){
        //    printf("%x %d\n", diff_list[i][0], diff_list[i][1]);
            if (diff_list[i][1] > most){
                most = diff_list[i][1];
                most_idx = i;
            }
        }

        if (most > 3){
            printf("page_idx: %x, # of matched (mostly): %d, diff: %x\n", page_idx, diff_list[most_idx][1], diff_list[most_idx][0]);
        }
        page_idx++;
    }
    return 0;
}
