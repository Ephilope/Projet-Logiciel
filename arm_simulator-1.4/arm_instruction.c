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
#include "arm_instruction.h"
#include "arm_exception.h"
#include "arm_data_processing.h"
#include "arm_load_store.h"
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"


int condition_OK(uint8_t condition){
    return 1;
}

int arm_decode(arm_core p){
    uint32_t instruction;
    arm_fetch(p,&instruction);
    int (*fct_decode)(arm_core, uint32_t);
    int resultat = 0;
    /** Condition of execution of the instruction 
     * See ARM Manual reference A1-6 section A1.2 paragraphe 3 (p.32)
    **/
    uint8_t cond = (uint8_t) ((instruction >> 28) & 0xF);
    if (!condition_OK(cond)){
        return 1;
    }
    // ge bit fields in the interval [27-25]
    uint8_t code_instruction = (uint8_t)get_bits(instruction,27, 25);
    switch (code_instruction){
    // data processing instruction : code_instruction 000 or 001
    case 0x0:
        fct_decode = arm_data_processing_shift;
        resultat = fct_decode(p, instruction);
        break;
    case 0x1:
        fct_decode = arm_data_processing_immediate_msr;
        resultat = fct_decode(p, instruction);
        break;
    // Simple memory access, load/ strore : code instruction 010 or 011
    case 0x2:
    case 0x3:
        fct_decode = arm_load_store;
        resultat =  fct_decode(p, instruction);
        break;
    // Multiple memory access :  code_instruction 100
    case 0x4:
        // LDM / STM
        fct_decode = arm_load_store_multiple;
        resultat = fct_decode(p, instruction);
        break;
    // Branch or other : code_instruction 101
    case 0x5:
        // branch
        fct_decode = arm_branch;
        resultat = fct_decode(p, instruction);
        break;
    case 0x6:
    case 0x7:
        break;
    default:
        fprintf(stderr, "Instruction non prise en compte\n");
        resultat = UNDEFINED_INSTRUCTION;
        break;
    }
    return resultat;
}
static int arm_execute_instruction(arm_core p) {
   
    return 0;
}

int arm_step(arm_core p) {
    int result;

    result = arm_execute_instruction(p);
    if (result)
        arm_exception(p, result);
    return result;
}