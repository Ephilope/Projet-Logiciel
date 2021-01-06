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

#define R0 0 
#define R1 1 
#define R2 2 
#define R3 3 
#define R4 4 
#define R5 5 
#define R6 6 
#define R7 7 
#define R8 8
#define R9 9
#define R10 10
#define R11 11
#define R12 12
#define R13 13
#define R14 14
#define PC 15
#define CPSR 16
#define R13_svc 17
#define R14_svc 18
#define SPSR_svc 19
#define R13_abt 20
#define R14_abt 21
#define SPSR_abt 22
#define R13_und 23
#define R14_und 24
#define SPSR_und 25
#define R13_irq 26
#define R14_irq 27
#define SPSR_irq 28
#define R8_fiq 29
#define R9_fiq 30
#define R10_fiq 31
#define R11_fiq 32
#define R12_fiq 33
#define R13_fiq 34
#define R14_fiq 35
#define SPSR_fiq 36





struct registers_data {
    uint32_t regs[37];
    uint8_t mode;

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
    switch (r->mode){
    case USR:
    case SYS:
        value = read_usr_register(r,reg);
        break;
    case SVC:
        if (reg >= R0 && reg <= SPSR_svc){
            value = r->regs[reg];
        }
        break;
    case UND:
        if ((reg >= R0 && reg <= CPSR) || (reg == R13_und || 
            reg == R14_und || reg == SPSR_und )){
            value = r->regs[reg];
        }
        break;
    case ABT:
        if ((reg >= R0 && reg <= CPSR) || (reg == R13_abt || 
            reg == R14_abt || reg == SPSR_abt )){
            value = r->regs[reg];
        }
        break;
    case IRQ:
        if ((reg >= R0 && reg <= CPSR) || (reg == R13_irq || 
            reg == R14_irq || reg == SPSR_irq )){
            value = r->regs[reg];
        }
        break;
    case FIQ:
        if ((reg >= R0 && reg <= CPSR) || (reg >= R8_fiq && reg <= SPSR_irq)){
            value = r->regs[reg];
        }
        break;
    default:
        fprintf(stderr,"Erreur: %s est un mode inconnu\n", arm_get_mode_name(r->mode));
        break;
    }
    return value;
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
    return value = r->regs[CPSR];
}

uint32_t read_spsr(registers r) {
    uint32_t value=0;
    if(!current_mode_has_spsr(r)){
        fprintf(stderr, "Impossible de lire le registre spsr. Mode : %s \n", arm_get_mode_name(r->mode));
        exit(1);
    }else{
        switch (r->mode){
        case SVC:
            value = r->regs[SPSR_svc];
            break;
        case UND:
            value = r->regs[SPSR_und];
            break;
        case ABT:
            value = r->regs[SPSR_abt];
            break;
        case IRQ:
            value = r->regs[SPSR_irq];
            break;
        case FIQ:
            value = r->regs[SPSR_fiq];
            break;
        default:
            fprintf(stderr,"Erreur: %s est un mode inconnu\n", arm_get_mode_name(r->mode));
            break;
        }
    }

    return value;
}

void write_register(registers r, uint8_t reg, uint32_t value) {
    switch (r->mode){
    case USR:
    case SYS:
        write_usr_register(r,reg,value);
        break;
    case SVC:
        if (reg >= R0 && reg <= SPSR_svc){
            r->regs[reg] = value;
        }
        break;
    case UND:
        if ((reg >= R0 && reg <= CPSR) || (reg == R13_und || 
            reg == R14_und || reg == SPSR_und )){
            r->regs[reg] = value;
        }
        break;
    case ABT:
        if ((reg >= R0 && reg <= CPSR) || (reg == R13_abt || 
            reg == R14_abt || reg == SPSR_abt )){
            r->regs[reg] = value;
        }
        break;
    case IRQ:
        if ((reg >= R0 && reg <= CPSR) || (reg == R13_irq || 
            reg == R14_irq || reg == SPSR_irq )){
            r->regs[reg] = value;
        }
        break;
    case FIQ:
        if ((reg >= R0 && reg <= CPSR) || (reg >= R8_fiq && reg <= SPSR_irq)){
            r->regs[reg] = value;
        }
        break;
    default:
        fprintf(stderr,"Erreur: %s est un mode inconnu\n", arm_get_mode_name(r->mode));
        break;
    }
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
    r->regs[CPSR] = value;
}

void write_spsr(registers r, uint32_t value) {
    if (!current_mode_has_spsr(r)){
        fprintf(stderr, "Impossible d'écfrire dans le registre spsr en mode : %s \n", arm_get_mode_name(r->mode));
        exit(1);
    }else{
        switch (r->mode){
        case SVC:
            r->regs[SPSR_svc] = value;
            break;
        case UND:
            r->regs[SPSR_und] = value;
            break;
        case ABT:
            r->regs[SPSR_abt] = value;
            break; 
        case IRQ:
            r->regs[SPSR_irq] = value;
            break;
        case FIQ:
            r->regs[SPSR_fiq] = value;
            break;
        default:
            fprintf(stderr,"Erreur: %s est un mode inconnu\n", arm_get_mode_name(r->mode));
            break;
        }
    }
}
