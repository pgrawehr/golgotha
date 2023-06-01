#pragma once
#include <cstdio>

int                gmod_arch_check(void);
int                gmod_read(FILE* filein);
float              gmod_read_float(FILE* filein);
unsigned short     gmod_read_w16(FILE* filein);
unsigned long      gmod_read_w32(FILE* filein);
int                gmod_write(FILE* fileout);
void               gmod_write_float(float Val, FILE* fileout);
void               gmod_write_w16(unsigned short Val, FILE* fileout);
void               gmod_write_w32(unsigned long Val, FILE* fileout);
int material_list_index(const char* name);
