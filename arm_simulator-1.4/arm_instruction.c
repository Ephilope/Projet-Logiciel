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


int condition_OK(uint8_t condition, uint8_t flag_N, uint8_t flag_Z, uint8_t flag_C, uint8_t flag_V){
    int result = 0;
    switch (condition){
    case EQ:        // Equal : Z set
        result = (flag_Z == 1);
        break;		
    case NE:        // Not equal : Z clear
        result = (flag_Z == 0);
        break;
    case CS_HS:     // Carry set/unsigned higher or same : C set
        result = ( flag_C == 1);
        break;
    case CC_LO: 	// Carry clear/unsigned lower : C clear
        result = ( flag_C == 0);
        break;
    case MI:		// Minus/negative : N set
        result = ( flag_N == 1);
    case PL:		// Plus/positive or zero : N clear
        result = ( flag_N == 0 );
        break;
    case VS:		// Overflow V set
        result = ( flag_V == 1);
        break;
    case VC:		// No overflow : V clear
        result = ( flag_V == 0);
        break;
    case HI: 		// Unsigned higher : C set and Z clear
        result = ( flag_C == 1 && flag_Z == 0);
        break;
    case LS:		// Unsigned lower or same : C clear or Z set
        result = ( flag_C == 0 && flag_Z == 1);
        break;
    case GE:		// Signed greater than or equal : N set and V set, or N clear and V clear (N == V)
        result = ( flag_N == flag_V);
        break;
    case LT:		// Signed less than : N set and V clear, or N clear and V set (N != V)
        result = ( flag_N != flag_V);
        break;
    case GT:  		// Signed greater than : Z clear, and either N set and V set, or N clear and V clear (Z == 0 and N == V)
        result = ( flag_Z == 0 && flag_N == flag_V);
        break;
    case LE:		// Signed less than or equal : Z set, or N set and V clear, or N clear and V set (Z == 1 or N != V)
        result = ( flag_Z == 1 && flag_N != flag_V);
        break;
    case AL:  		// Always (unconditional)
        result = 1;
        break;
    case UX:        // Unconditional extension : Condition code 0b1111
        result = 1;		
    default:
        result = 0;
        fprintf(stderr, "Code condition instruction inconnu [31-28]\n");
        break;
    }
    return result;
}

int arm_decode(arm_core p){
    uint32_t instruction;
    arm_fetch(p,&instruction);
    int (*fct_decode)(arm_core, uint32_t);
    int result = 0;

    uint8_t two_first_bits_opcode = 0;
    uint8_t S = 0;
    uint8_t bit4 = 0;
    uint8_t bit7 = 0;
    
    /** Condition of execution of the instruction 
     * See ARM Manual reference A1-6 section A1.2 paragraphe 3 (p.32)
    **/
    uint8_t cond = (uint8_t) get_bits(instruction, 31,28);
    uint32_t cpsr = arm_read_cpsr(p);
    uint8_t flag_N = (u_int8_t) get_bit(cpsr, N);
    uint8_t flag_Z = (u_int8_t) get_bit(cpsr, Z);
    uint8_t flag_C = (u_int8_t) get_bit(cpsr, C);
    uint8_t flag_V = (u_int8_t) get_bit(cpsr, V);

    if (!condition_OK(cond, flag_N, flag_Z, flag_C, flag_V)){
        return 1;
    }
    // ge bit fields in the interval [27-25]
    uint8_t code_instruction = (uint8_t)get_bits(instruction,27, 25);
    switch (code_instruction){
    // data processing instruction : code_instruction 000 or 001
    case 0x0:
        two_first_bits_opcode = (uint8_t) get_bits(instruction, 24, 23);
        S = (uint8_t) get_bit(instruction, 20);
        bit4 = (uint8_t) get_bit(instruction, 4);
        bit7 = (uint8_t) get_bit(instruction, 7);
        if ( (two_first_bits_opcode & 0xF) == 0x2 && ( S & 0xF) == 0x0) {
            if ( (bit4 & 0xF) == 0x0 || ( bit7 & 0xF) == 0x0){
                fct_decode = arm_miscellaneous;
                result = fct_decode(p, instruction);
            }
        }else{
            //fprintf(stderr, "Data process\n");
            fct_decode = arm_data_processing_shift;
            result = fct_decode(p, instruction);
        }
        break;
    case 0x1:
        fct_decode = arm_data_processing_immediate_msr;
        result = fct_decode(p, instruction);
        break;
    // Simple memory access, load/ strore : code instruction 010 or 011
    case 0x2:
    case 0x3:
        fct_decode = arm_load_store;
        result =  fct_decode(p, instruction);
        break;
    // Multiple memory access :  code_instruction 100
    case 0x4:
        // LDM / STM
        fct_decode = arm_load_store_multiple;
        result = fct_decode(p, instruction);
        break;
    // Branch or other : code_instruction 101
    case 0x5:
        // branch
        fct_decode = arm_branch;
        result = fct_decode(p, instruction);
        break;
    case 0x6:
        fct_decode = arm_coprocessor_load_store;
        result = fct_decode(p, instruction);
        break;
    case 0x7:
        fct_decode = arm_coprocessor_others_swi;
        result = fct_decode(p, instruction);
        break;
    default:
        fprintf(stderr, "Instruction non prise en compte\n");
        result = UNDEFINED_INSTRUCTION;
        break;
    }
    return result;
}

static int arm_execute_instruction(arm_core p) {
    int result = arm_decode(p);
    if (result)
        arm_exception(p, result);
    return result;
}

int arm_step(arm_core p) {
    int result;

    result = arm_execute_instruction(p);
    if (result)
        arm_exception(p, result);
    return result;
}