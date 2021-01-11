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
#ifndef __ARM_DATA_PROCESSING_H__
#define __ARM_DATA_PROCESSING_H__
#include <stdint.h>
#include "arm_core.h"

#define AND 0 		// 0000 Logical AND, Rd := Rn AND shifter_operand
#define EOR 1		// 0001 Logical Exclusive OR,  Rd := Rn EOR shifter_operand
#define SUB 2		// 0010 SUB Subtract, Rd := Rn - shifter_operand
#define RSB 3		// 0011 RSB Reverse Subtract, Rd := shifter_operand - Rn
#define ADD 4 		// 0100 ADD Add, Rd := Rn + shifter_operand
#define ADC 5		// 0101 ADC Add with Carry Rd := Rn + shifter_operand + Carry Flag
#define SBC 6		// 0110 SBC Subtract with Carry, Rd := Rn - shifter_operand - NOT(Carry Flag)
#define RSC 7		// 0111 RSC Reverse Subtract with Carry, Rd := shifter_operand - Rn - NOT(Carry Flag)
#define TST 8 		// 1000 TST Test Update flags after, Rn AND shifter_operand
#define TEQ 9 		// 1001 TEQ Test Equivalence Update flags after, Rn EOR shifter_operand
#define CMP 10 		// 1010 CMP Compare Update flags after, Rn - shifter_operand
#define CMN 11 		// 1011 CMN Compare Negated Update flags after, Rn + shifter_operand
#define ORR 12		// 1100 ORR Logical (inclusive) OR, Rd := Rn OR shifter_operand
#define MOV 13		// 1101 MOV Move, Rd := shifter_operand (no first operand)
#define BIC 14		// 1110 BIC Bit Clear Rd := Rn AND NOT(shifter_operand)
#define MVN 15 		// 1111 MVN Move Not, Rd := NOT shifter_operand (no first operand)

void set_parameters(uint32_t ins,uint8_t *opcode, uint8_t *S, uint8_t *Rn, uint8_t *Rd, uint16_t *operand2);
void updateFlags();


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
#include "arm_data_processing.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_branch_other.h"
#include "util.h"
#include "debug.h"

void updateFlags();
 uint8_t get_Cflag(arm_core p,uint8_t val_rd);

uint32_t and(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint16_t shifter_operand);
uint32_t eor(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint16_t shifter_operand);
uint32_t sub(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint16_t shifter_operand);
uint32_t rsb(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint16_t shifter_operand);
uint32_t add(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint16_t shifter_operand);
uint32_t sub(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint16_t shifter_operand);
uint32_t adc(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint16_t shifter_operand);
uint32_t sbc(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint16_t shifter_operand);
uint32_t rsc(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint16_t shifter_operand);
uint32_t tst(arm_core p, uint8_t Rn, uint16_t shifter_operand);
uint32_t teq(arm_core p, uint8_t Rn, uint16_t shifter_operand);
uint32_t cmp(arm_core p, uint8_t Rn, uint16_t shifter_operand);
int32_t cmn(arm_core p, uint8_t Rn, uint16_t shifter_operand);
uint32_t orr(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint16_t shifter_operand);
uint32_t mov(arm_core p, uint8_t Rd, uint16_t shifter_operand);
uint32_t bic(arm_core p, uint8_t Rd, uint8_t Rn, uint16_t shifter_operand);
uint32_t mvn(arm_core p, uint8_t Rd, uint16_t shifter_operand);



uint32_t select_operation(arm_core p, uint32_t ins);

uint16_t is_shifted(uint32_t ins, uint16_t shifter_operand);
uint32_t shift_operation(uint32_t value, uint8_t shift);



///
int arm_data_processing_shift(arm_core p, uint32_t ins);
int arm_data_processing_immediate_msr(arm_core p, uint32_t ins);

#endif
