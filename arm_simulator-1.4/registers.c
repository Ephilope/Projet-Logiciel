/*
Armator - simulateur de jeu d'instruction ARMv5T � but p�dagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique G�n�rale GNU publi�e par la Free Software
Foundation (version 2 ou bien toute autre version ult�rieure choisie par vous).

Ce programme est distribu� car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but sp�cifique. Reportez-vous � la
Licence Publique G�n�rale GNU pour plus de d�tails.

Vous devez avoir re�u une copie de la Licence Publique G�n�rale GNU en m�me
temps que ce programme ; si ce n'est pas le cas, �crivez � la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
�tats-Unis.

Contact: Guillaume.Huard@imag.fr
	 B�timent IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'H�res
*/
#include "registers.h"
#include "arm_constants.h"
#include <stdlib.h>
#include <stdio.h>






struct registers_data {
    uint32_t  regist_array[22];

};

registers registers_create() {
    registers r = malloc(sizeof(struct registers_data));
    r->mode = USR;
    return r;
}

void registers_destroy(registers r) {
    free(r);
}

uint8_t get_mode(registers r) {
    return r->mode;
} 

int current_mode_has_spsr(registers r) {
    return get_mode(r) == SVC || get_mode(r) == ABT || get_mode(r) == UND || 
            get_mode(r) == IRQ || get_mode(r) == FIQ;
}

int in_a_privileged_mode(registers r) {
    return get_mode(r) != USR;
}

uint32_t read_register(registers r, uint8_t reg) {
    uint32_t value=0;
    return value = read_usr_register(r,reg);
}

uint32_t read_usr_register(registers r, uint8_t reg) {
    uint32_t value=0;
    if(reg > 15){
        fprintf(stderr, "Lecture impossible du registre R%u", reg);
        exit(0);
    }
    value = r->regs[reg];
    return value;
}

uint32_t read_cpsr(registers r) {
    uint32_t value=0;
    return value = r->cpsr;
}

uint32_t read_spsr(registers r) {
    uint32_t value=0;
    if(!current_mode_has_spsr(r)){
        fprintf(stderr, "Impossible de lire le registre spsr. Mode : %s \n", arm_get_mode_name(r->mode));
        exit(1);
    }
    return value = r->spsr;
}

void write_register(registers r, uint8_t reg, uint32_t value) {
   
    write_usr_register(r,reg,value);
    
}

void write_usr_register(registers r, uint8_t reg, uint32_t value) {
    if (reg > 15)
    {
        fprintf(stderr, "Ecriture impossible dans le registre %d\n", reg);
        exit(0);
    }
    r->regs[reg] = value;

}

void write_cpsr(registers r, uint32_t value) {
    r->cpsr = value;
}

void write_spsr(registers r, uint32_t value) {
    if (!current_mode_has_spsr(r)){
        fprintf(stderr, "Impossible d'écfrire dans le registre spsr en mode : %s \n", arm_get_mode_name(r->mode));
        exit(1);
    }
    r->spsr = value;
    
}
