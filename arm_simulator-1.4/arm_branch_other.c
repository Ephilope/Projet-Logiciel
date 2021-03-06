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
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"
#include <debug.h>
#include <stdlib.h>


int arm_branch(arm_core p, uint32_t ins) {
    uint32_t res;
    if(get_bit(ins,24)){//BL 
        //enregistre pc dans lr
        arm_write_register(p,14,arm_read_register(p,15));
    }
    if(get_bit(ins,23)){
        res = (get_bits(ins,23,0) | 0x3F000000)<< 2; 
        arm_write_register(p, 15, (arm_read_register(p, 15) + res ));
    }
    else{
    	res = get_bits(ins,23,0)<< 2 ;
        arm_write_register(p, 15, (arm_read_register(p, 15) + res ));
    }
    return 0;
}

int arm_coprocessor_others_swi(arm_core p, uint32_t ins) {//Instruction SWI page 360 doc ARM
    /*if ConditionPassed(cond) then
        R14_svc = address of next instruction after the SWI instruction
        SPSR_svc = CPSR
        CPSR[4:0] = 0b10011  Enter Supervisor mode 
        CPSR[5] = 0  Execute in ARM state 

        CPSR[6] is unchanged 
        CPSR[7] = 1  Disable normal interrupts 

        CPSR[8] is unchanged 
        CPSR[9] = CP15_reg1_EEbit
        
        if high vectors configured then
            PC = 0xFFFF0008
        else
            PC = 0x00000008
    */



   // !!!!    A TERMINER PLUS TARD   !!!! //
    if (get_bit(ins, 24)) {
        /* Here we implement the end of the simulation as swi 0x123456 */
        if ((ins & 0xFFFFFF) == 0x123456){
            exit(0);
        }
        return SOFTWARE_INTERRUPT;
    } 
    return UNDEFINED_INSTRUCTION;
}

int arm_miscellaneous(arm_core p, uint32_t ins) {//Instruction MRS page 222 doc ARM
    if( get_bit(ins,22)){
        arm_write_register(p,get_bits(ins,15,12),arm_read_spsr(p));
    }
    else{
        arm_write_register(p,get_bits(ins,15,12),arm_read_cpsr(p));
    }

    return UNDEFINED_INSTRUCTION;
}
