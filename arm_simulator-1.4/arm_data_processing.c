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



void update_flags(arm_core p, uint8_t S, uint8_t Rd, uint8_t flag_C, uint8_t flag_V) {
	if ( S == 1 && Rd == 15) {
		if(arm_current_mode_has_spsr(p)) {
			arm_write_cpsr(p, arm_read_spsr(p));
		}else
			arm_exception(p, UNDEFINED_INSTRUCTION);
	} else if (S == 1) {
		write_flags(p, Rd, flag_C, flag_V);
	}
}

void write_flags(arm_core p, uint8_t Rd, uint8_t flag_C, uint8_t flag_V) {
  uint32_t cpsr = arm_read_cpsr(p);
  uint32_t flag_N = get_bit(arm_read_register(p, Rd), N);   //N Flag = Rd[31]

  uint32_t flag_Z = (arm_read_register(p,Rd) == 0) ;

  cpsr = (flag_N == 1) ? set_bit(cpsr , N) : clr_bit(cpsr, N);
  cpsr = (flag_Z == 1) ? set_bit(cpsr , Z) : clr_bit(cpsr, Z);
  if (flag_C != UNAFFECTED)
  	cpsr = (flag_C == 1) ? set_bit(cpsr , C) : clr_bit(cpsr, C);
  if (flag_V != UNAFFECTED)
  	cpsr = (flag_V == 1) ? set_bit(cpsr , V) : clr_bit(cpsr, V);

  arm_write_cpsr(p, cpsr);

}


/* carryFrom indicates that the operation gives a result that is too large for
** relative to the number of bits of the ALU.
** In the case of substraction, see borrowFrom
*/

uint8_t carryFrom(uint32_t x, uint32_t y) {
	uint32_t max = ~0;

	return max < x + y;
}

/* In the case of a subtraction, the withholding
**(borrow) is set to 1 if the deduced operand (y) is the
** larger of the two.
*/
uint8_t borrowFrom(uint32_t x, uint32_t y) {
	return x < y;
}

/* it indicates that the sum of 2 numbers with the same sign gives a
** result of opposite sign.
*/

uint8_t overflowFrom(uint32_t x, uint32_t y, uint32_t z, uint8_t opcode) {
	uint8_t result = 0;
	switch (opcode){
	case ADD:
		result = ( ((x >> 31) == (y >> 31)) && (x >> 31) == z >> 31 );
		break;
	case SUB:
		result = ((x >> 31) != (y >> 31)) && ((x >> 31) != (z >> 31));
	default:
		fprintf(stderr, "Erreur : Dans overflowFrom");
		break;
	}
	return result;
}

uint32_t and(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint32_t shifter_operand){
    //uint8_t shifter_carry_out = 0;
	uint32_t value_rn = arm_read_register(p, Rn);
    uint32_t value_rd = (value_rn & shifter_operand);
    arm_write_register(p, Rd, value_rd);
    //uint8_t flag_C = shifter_carry_out;
	update_flags(p, S, Rd, get_flag_C(p), UNAFFECTED); //0xFF means no updating flag
    return 0;
}


uint32_t eor(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint32_t shifter_operand){
    //uint8_t shifter_carry_out = 0;
	uint32_t value_rn = arm_read_register(p, Rn);
    uint32_t value_rd = (value_rn ^ shifter_operand);
    arm_write_register(p, Rd, value_rd);
    //uint8_t flag_C = shifter_carry_out;
	update_flags(p, S, Rd, get_flag_C(p), UNAFFECTED); //0xFF means no updating flag

	return 0;
}

uint32_t sub(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint32_t shifter_operand){
	uint32_t value_rn = arm_read_register(p, Rn);
	uint32_t value_rd = value_rn - shifter_operand;
	arm_write_register(p, Rd, value_rd);
	uint8_t flag_C = ~borrowFrom(value_rn, shifter_operand);
	uint8_t flag_V = overflowFrom(value_rn, shifter_operand, value_rd, SUB);
	update_flags(p, S, Rd, flag_C, flag_V);

	return 0;
}

uint32_t rsb(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint32_t shifter_operand){
    uint32_t value_rn = arm_read_register(p, Rn);
    uint32_t value_rd = shifter_operand - value_rn;
    arm_write_register(p, Rd, value_rd);
	uint8_t flag_C = ~borrowFrom(shifter_operand, value_rn);
	uint8_t flag_V = overflowFrom(shifter_operand, value_rn, value_rd, SUB);
    update_flags(p, S, Rd, flag_C, flag_V);

    return 0;

}


uint32_t add(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint32_t shifter_operand){

	uint32_t value_rn = arm_read_register(p, Rn);
	uint32_t value_rd = value_rn + shifter_operand;
	arm_write_register(p, Rd, value_rd);
	uint8_t flag_C = carryFrom(value_rn, shifter_operand);
	uint8_t flag_V = overflowFrom(value_rn,shifter_operand, value_rd, ADD);
	update_flags(p, S, Rd,flag_C , flag_V);
    return 0;
}

uint32_t adc(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint32_t shifter_operand){

	uint32_t value_rd = arm_read_register(p, Rn) + shifter_operand + get_flag_C(p);
    arm_write_register(p, Rd, value_rd);
	uint8_t flag_C = carryFrom(arm_read_register(p,Rn)+shifter_operand, get_flag_C(p));
	uint8_t flag_V = overflowFrom(arm_read_register(p,Rn)+shifter_operand, get_flag_C(p), value_rd, ADD);
    update_flags(p, S, Rd, flag_C, flag_V);

    return 0;
}
uint32_t sbc(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint32_t shifter_operand){

    uint32_t value_rd = arm_read_register(p, Rn) - shifter_operand - (~get_flag_C(p));
    arm_write_register(p, Rd, value_rd);
	uint8_t flag_C =  ~borrowFrom(arm_read_register(p, Rn), shifter_operand - (~get_flag_C(p)));
	uint8_t flag_V = overflowFrom(arm_read_register(p, Rn), shifter_operand - (~get_flag_C(p)), value_rd, SUB);
    update_flags(p, S, Rd, flag_C, flag_V);

    return 0;
}

uint32_t rsc(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint32_t shifter_operand){

    uint32_t value_rd = shifter_operand - arm_read_register(p, Rn) - (~get_flag_C(p));
    arm_write_register(p, Rd, value_rd);
	uint8_t flag_C = ~borrowFrom(shifter_operand - (~get_flag_C(p)), arm_read_register(p, Rn));		//NOT BorrowFrom(shifter_operand - Rn - NOT(C Flag))
	uint8_t flag_V = overflowFrom(shifter_operand - (~get_flag_C(p)), arm_read_register(p, Rn), value_rd, SUB);

    update_flags(p, S, Rd, flag_C, flag_V);

    return 0;
}

uint32_t tst(arm_core p, uint8_t Rn, uint32_t shifter_operand){
	//uint8_t shifter_carry_out = 0 ;
	uint32_t alu_out = arm_read_register(p, Rn) & shifter_operand;
	//uint8_t flag_C = shifter_carry_out;
	write_flags(p,alu_out, UNAFFECTED, UNAFFECTED);

	return 0;
}

uint32_t teq(arm_core p, uint8_t Rn, uint32_t shifter_operand){
	//uint8_t shifter_carry_out = 0 ;
	uint32_t alu_out = arm_read_register(p, Rn) ^ shifter_operand;
	//uint8_t flag_C = shifter_carry_out;
	write_flags(p,alu_out, UNAFFECTED, UNAFFECTED);

	return 0;
}

uint32_t cmp(arm_core p, uint8_t Rn, uint32_t shifter_operand){

	uint32_t alu_out = arm_read_register(p, Rn) - shifter_operand;
	uint8_t flag_C = ~borrowFrom(arm_read_register(p, Rn), shifter_operand);
	uint8_t flag_V = overflowFrom(arm_read_register(p, Rn), shifter_operand, alu_out, SUB);
	write_flags(p,alu_out, flag_C, flag_V);

	return 0;
}
uint32_t cmn(arm_core p, uint8_t Rn, uint32_t shifter_operand){
	uint32_t alu_out = arm_read_register(p, Rn) + shifter_operand;
	uint8_t flag_C = carryFrom(arm_read_register(p, Rn), shifter_operand);
	uint8_t flag_V = overflowFrom(arm_read_register(p, Rn), shifter_operand, alu_out, ADD);
	write_flags(p,alu_out, flag_C, flag_V);

	return 0;
}

uint32_t orr(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint32_t shifter_operand){
	//uint8_t shifter_carry_out = 0;
	uint32_t value = arm_read_register(p, Rn) | shifter_operand;
	arm_write_register(p, Rd, value);
	//uint8_t flag_C = shifter_carry_out ;
	update_flags(p, S, Rd, get_flag_C(p), UNAFFECTED);
	return 0;
}

uint32_t mov(arm_core p, uint8_t S, uint8_t Rd, uint32_t shifter_operand){
	//uint8_t shifter_carry_out = 0;
	arm_write_register(p, Rd, shifter_operand);
	//uint8_t flag_C = shifter_carry_out;
	update_flags(p, S, Rd, UNAFFECTED, UNAFFECTED);
	return 0;
}

uint32_t bic(arm_core p, uint8_t S, uint8_t Rd, uint8_t Rn, uint32_t shifter_operand){
	//uint8_t shifter_carry_out = 0;
	uint32_t value = arm_read_register(p, Rn) & (~shifter_operand);
	arm_write_register(p, Rd, value);
	//uint8_t flag_C = shifter_carry_out;
	update_flags(p, S, Rd, UNAFFECTED, UNAFFECTED);
	return 0;
}

uint32_t mvn(arm_core p, uint8_t S, uint8_t Rd, uint32_t shifter_operand){
	//uint8_t shifter_carry_out = 0;
	arm_write_register(p, Rd, ~shifter_operand);
	//uint8_t flag_C = shifter_carry_out;
	update_flags(p, S, Rd, UNAFFECTED, UNAFFECTED);

	return 0;
}


uint8_t get_flag_N(arm_core p){
	return (uint8_t) get_bit(arm_read_cpsr(p), N);
}

uint8_t get_flag_Z(arm_core p){
	return (uint8_t) get_bit(arm_read_cpsr(p), Z);
}
uint8_t get_flag_C(arm_core p){
	return (uint8_t) get_bit(arm_read_cpsr(p), C);
}
uint8_t get_flag_V(arm_core p){
	return (uint8_t) get_bit(arm_read_cpsr(p), V);
}


uint32_t select_operation(arm_core p, uint32_t ins){
	uint32_t result = 0;
	uint8_t opcode = 0;
	uint8_t S = 0;
	uint8_t Rn = 0;
	uint8_t Rd = 0;
	uint32_t shifter_operand = 0;
	uint8_t shifter_carry_out = 0;
	set_parameters(p,ins, &opcode, &S, &Rn, &Rd, &shifter_operand, &shifter_carry_out);
	switch (opcode){
	case AND: 		// 0000 Logical AND, Rd := Rn AND shifter_operand
		result = and(p, S, Rd, Rn, shifter_operand);
		break;
	case EOR:		// 0001 Logical Exclusive OR,  Rd := Rn EOR shifter_operand
		result = eor(p, S, Rd, Rn, shifter_operand);
		break;
	case SUB:		// 0010 SUB Subtract, Rd := Rn - shifter_operand
		result = sub(p, S, Rd, Rn, shifter_operand);
		break;
	case RSB:		// 0011 RSB Reverse Subtract, Rd := shifter_operand - Rn
		result = rsb(p, S, Rd, Rn, shifter_operand);
		break;
	case ADD: 		// 0100 ADD Add, Rd := Rn + shifter_operand
		result = add(p, S, Rd, Rn, shifter_operand);
		break;
	case ADC:		// 0101 ADC Add with Carry Rd := Rn + shifter_operand + Carry Flag
		result = adc(p, S, Rd, Rn, shifter_operand);
		break;
	case SBC:		// 0110 SBC Subtract with Carry, Rd := Rn - shifter_operand - NOT(Carry Flag)
		result = sbc(p, S, Rd, Rn, shifter_operand);
		break;
	case RSC:		// 0111 RSC Reverse Subtract with Carry, Rd := shifter_operand - Rn - NOT(Carry Flag)
		result = rsc(p, S, Rd, Rn, shifter_operand);
		break;
	case TST: 		// 1000 TST Test Update flags after, Rn AND shifter_operand
		result = tst(p, Rn, shifter_operand);
		break;
	case TEQ: 		// 1001 TEQ Test Equivalence Update flags after, Rn EOR shifter_operand
		result = teq(p, Rn, shifter_operand);
		break;
	case CMP: 		// 1010 CMP Compare Update flags after, Rn - shifter_operand
		result = cmp(p, Rn, shifter_operand);
		break;
	case CMN:  		// 1011 CMN Compare Negated Update flags after, Rn + shifter_operand
		result = cmn(p,Rn, shifter_operand);
		break;
	case ORR: 		// 1100 ORR Logical (inclusive) OR, Rd := Rn OR shifter_operand
		result = orr(p, S, Rd, Rn, shifter_operand);
		break;
	case MOV: 		// 1101 MOV Move, Rd := shifter_operand (no first operand)
		result = mov(p, S, Rd, shifter_operand);
		break;
	case BIC: 		// 1110 BIC Bit Clear Rd := Rn AND NOT(shifter_operand)
		result = bic(p, S, Rd, Rn, shifter_operand);
		break;
	case MVN: 		// 1111 MVN Move Not, Rd := NOT shifter_operand (no first operand)
		result = mvn(p, S, Rd, shifter_operand);
		break;
	default:
		fprintf(stderr, "Opération non reconnue\n");
		result = 1;
	}

    return result;

}
/*
uint32_t split_merge_shifter_operand(arm_core p, uint32_t ins){
	uint32_t shifter_operand = 0;
	//fprintf(stdout, "Avant split op2 = 0x%08x\n", get_bits(ins, 11,0));
	uint8_t I = (uint8_t) get_bit(ins, 25);
	if (!I){
		uint32_t Rm = (uint32_t) get_bits(ins, 3, 0);
		uint32_t  value_rs = arm_read_register(p, Rm);
		//fprintf(stdout, "Rm avant split op2 = 0x%08x\n", value_rs);
		uint8_t code_shift = (uint8_t) get_bits(ins, 6, 5);
		uint8_t bit4 = (uint8_t) get_bit(ins,4);
		if (!bit4){
			uint8_t shift_amount = (uint8_t) get_bits(ins, 11, 7);
			shifter_operand = shift_operation(value_rs, code_shift,shift_amount);
		}else{
			if (!get_bit(ins, 7)){
				uint8_t Rs = (uint8_t) get_bits(ins, 11, 8);
				uint32_t value_rs = arm_read_register(p, Rs);
				shifter_operand = shift_operation(value_rs, code_shift, value_rs);
			}
		}

	}else{
		uint8_t rotate = (uint8_t) get_bits(ins, 11,8);
		uint8_t immediate = (uint8_t) get_bits(ins, 7,0);
		shifter_operand = ror(immediate, 2 *rotate);
		
	}
	//fprintf(stdout, "Apres split op2 = 0x%08x\n", shifter_operand);
	//fprintf(stdout, "Rm apres split op2 = 0x%08x\n", get_bits(ins, 3,0));
	return shifter_operand;
}*/

uint32_t shift_operation(uint32_t value, uint8_t shift, uint8_t shift_amount){
	uint32_t (*fct_shift)(uint32_t, uint8_t);
	uint32_t result = 0;
	switch (shift){
	case LSL:
		fct_shift = lsl;
		result =  fct_shift(value, shift_amount);
		break;
	case LSR:
		fct_shift = lsr;
		result = fct_shift(value, shift);
		break;
	case ASR:
		fct_shift = asr;
		result = fct_shift(value, shift);
		break;
	case ROR:
		fct_shift = ror;
		result = fct_shift(value, shift);
		break;
	default:
		fprintf(stderr, "Shift opération inconnue\n");
		result = 1;
		break;
	}
	return result;
}

void write_shift_operand_sco(arm_core p, uint32_t ins, uint32_t *shifter_operand, uint8_t *shifter_carry_out){
	//uint8_t shifter_carry_out = 0;
	uint8_t I = (uint8_t) get_bit(ins, 25);
	
	if (!I){
		uint8_t Rm = (uint8_t) get_bits(ins, 3,0);
		uint8_t shift_amount = (uint8_t) get_bits(ins, 11, 7);
		//uint8_t shift = (uint8_t) get_bits(ins, 6,5);
		uint32_t  value_rm = arm_read_register(p, Rm);
		//fprintf(stdout, "Rm avant split op2 = 0x%08x\n", value_rs);
		uint8_t code_shift = (uint8_t) get_bits(ins, 6, 5);
		uint8_t bit4 = (uint8_t) get_bit(ins,4);
		if ( (get_bits(ins, 11, 4) & 0xFF) == 0){
			*shifter_operand = value_rm;
			*shifter_carry_out = get_flag_C(p);
		}
		
		if (!bit4){
			if (code_shift == LSL){
				*shifter_operand = (shift_amount == 0) ? value_rm : shift_operation(value_rm, code_shift,shift_amount);
				*shifter_carry_out  = (shift_amount == 0) ? get_flag_C(p) : get_bit(value_rm, 32-shift_amount);   //Rm[32 - shift_imm]
			}else if(code_shift == LSR){
				*shifter_operand = (shift_amount == 0) ? 0 : shift_operation(value_rm, code_shift,shift_amount);
				*shifter_carry_out  = (shift_amount == 0) ? get_bit(value_rm, 31):get_bit(value_rm, shift_amount-1);
			}else if(code_shift == ASR){
				if((shift_amount == 0)){
					*shifter_operand =  (!get_bit(value_rm, 31)) ? 0 : 0xFFFFFFFF;
					*shifter_carry_out  = (!get_bit(value_rm, 31) ) ? get_bit(value_rm, 31) : get_bit(value_rm, 31);
				}else{
					*shifter_operand =  shift_operation(value_rm, code_shift,shift_amount);
					*shifter_carry_out  = get_bit(value_rm, shift_amount -1);
				}	
			}else if(code_shift == ROR){
				uint32_t x = 0 ;
				x = lsl(get_flag_C(p), 31) | lsr(value_rm,1);  //(C Flag Logical_Shift_Left 31) OR (Rm Logical_Shift_Right 1)
				*shifter_operand = (shift_amount == 0) ? x : shift_operation(value_rm, code_shift,shift_amount);
				*shifter_carry_out  = (shift_amount == 0) ? get_bit(value_rm, 0) : get_bit(value_rm, shift_amount -1);
			}
			
		}else{
			uint8_t Rs = (uint8_t) get_bits(ins, 11, 8);
			uint32_t value_rs = arm_read_register(p, Rs);
			if (!get_bit(ins, 7)){
				if (code_shift == LSL){
					uint8_t value_Rs_bits_7_0 = (uint8_t) (get_bits(value_rs, 7, 0));
					if ( value_Rs_bits_7_0 == 0){ 	//Rs[7:0] == 0 
						*shifter_operand = value_rm;
						*shifter_carry_out = get_flag_C(p);
					}
					else if ( value_Rs_bits_7_0 < 32 ){ //Rs[7:0] < 32 
						*shifter_operand = shift_operation(value_rm, code_shift, value_Rs_bits_7_0);  // Rm Logical_Shift_Left Rs[7:0]
						*shifter_carry_out =  get_bit(value_rm, 32 - value_Rs_bits_7_0);  //Rm[32 - Rs[7:0] ]
					}
					else if(value_Rs_bits_7_0 == 32){ //Rs[7:0] == 32 then
						*shifter_operand = 0 ;
						*shifter_carry_out = get_bit(value_rm,0);		//Rm[31]
					}
					else{ /* Rs[7:0] > 32 */
						*shifter_operand = 0 ;
						*shifter_carry_out = 0 ;
					}
				}else if(code_shift == LSR){
					uint8_t value_Rs_bits_7_0 = (uint8_t) (get_bits(value_rs, 7, 0));
					if ( value_Rs_bits_7_0 == 0){ 	//Rs[7:0] == 0 
						*shifter_operand = value_rm;
						*shifter_carry_out = get_flag_C(p);
					}
					else if ( value_Rs_bits_7_0 < 32 ){ //Rs[7:0] < 32 
						*shifter_operand = shift_operation(value_rm, code_shift, value_Rs_bits_7_0);  // Rm Logical_Shift_Right Rs[7:0]
						*shifter_carry_out =  get_bit(value_rm, value_Rs_bits_7_0 - 1);  //Rm[Rs[7:0] - 1]
					}
					else if(value_Rs_bits_7_0 == 32){ //Rs[7:0] == 32 then
						*shifter_operand = 0 ;
						*shifter_carry_out = get_bit(value_rm,31);		//Rm[31]
					}
					else{ /* Rs[7:0] > 32 */
						*shifter_operand = 0 ;
						*shifter_carry_out = 0 ;
					}
				}else if(code_shift == ASR){
					uint8_t value_Rs_bits_7_0 = (uint8_t) (get_bits(value_rs, 7, 0));
					if ( value_Rs_bits_7_0 == 0){ 	//Rs[7:0] == 0 
						*shifter_operand = value_rm;
						*shifter_carry_out = get_flag_C(p);
					}
					else if ( value_Rs_bits_7_0 < 32 ){ //Rs[7:0] < 32 
						*shifter_operand = shift_operation(value_rm, code_shift, value_Rs_bits_7_0);  // Rm Arithmetic_Shift_Right Rs[7:0]
						*shifter_carry_out =  get_bit(value_rm, value_Rs_bits_7_0 - 1);  //Rm[Rs[7:0] - 1]
					}
					else{ /* Rs[7:0] >= 32 */
						*shifter_operand = (!get_bit(value_rm,31)) ? 0 : 0xFFFFFFFF;
						*shifter_carry_out =  get_bit(value_rm,31);
					}
				}else if(code_shift == ROR){
					*shifter_operand = (get_bits(arm_read_register(p, Rs), 7, 0) == 0) ?  // Rs[7:0] == 0 then
				 					(get_bits(arm_read_register(p, Rs), 4, 0) == 0) ? // Rs[4:0] == 0 
									value_rm : value_rm : shift_operation(value_rm, code_shift, get_bits(value_rs, 4, 0));
									// Rm       Rm  		 Rm Rotate_Right Rs[4:0]
		
					*shifter_carry_out = (get_bits(arm_read_register(p, Rs), 7, 0) == 0) ?  
				 					 (get_bits(arm_read_register(p, Rs), 4, 0) == 0) ?
									  get_flag_C(p):get_bit(value_rm,31):get_bit(value_rm, (uint8_t)get_bits(value_rs, 4,0)- 1);
									  // flag C 		Rm[31]				Rm[Rs[4:0] - 1]
				}
			}
		}
	}else{
		uint8_t rotate_imm = (uint8_t) get_bits(ins, 11,8);
		uint8_t immediate = (uint8_t) get_bits(ins, 7,0);
		*shifter_operand = ror(immediate, 2 *rotate_imm);
		*shifter_carry_out = ( rotate_imm == 0 ) ? get_flag_C(p) : (uint8_t) get_bit(*shifter_operand, 31);	
	}
	
}


void set_parameters(arm_core p, uint32_t ins,uint8_t *opcode, uint8_t *S, uint8_t *Rn, uint8_t *Rd, uint32_t *shifter_operand, uint8_t *shifter_carry_out){
	//uint8_t I = (uint8_t) get_bit(ins, 25);
	*opcode = (uint8_t) get_bits(ins, 24,21);
	*S = (uint8_t) get_bit(ins, 20);
	*Rn = (uint8_t) get_bits(ins, 19,16);
	*Rd = (uint8_t) get_bits(ins, 15,12);
	write_shift_operand_sco(p, ins, shifter_operand, shifter_carry_out);
}
/* Decoding functions for different classes of instructions */
int arm_data_processing_shift(arm_core p, uint32_t ins) {
	int result = 0;
	result = select_operation(p, ins);

	return result;
}

int arm_data_processing_immediate_msr(arm_core p, uint32_t ins) {
	int result = 0;
	result = select_operation(p, ins);
    
	return result;
}
