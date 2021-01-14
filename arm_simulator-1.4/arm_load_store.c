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
#include "arm_load_store.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_data_processing.h"
#include "util.h"
#include "debug.h"
#include "arm_instruction.h"
#include <assert.h>


uint8_t number_set_bits_in(uint16_t n) {
	uint8_t c = 0;
	while(n) {
	    c += n % 2;
	    n = n >> 1;
	}
	return c;
}

// Traite les instructions : LDR | LDRB | STR | STRB
int arm_load_store(arm_core p, uint32_t ins) {

	uint8_t cond = get_bits(ins, 31, 28);

	uint32_t rn = get_bits(ins, 19, 16);
	rn = arm_read_register(p, rn);

	uint32_t rd = get_bits(ins, 15, 12);
	uint32_t offset_12 = get_bits(ins, 11, 0);
	uint32_t rm = get_bits(ins, 3, 0);

	uint8_t L = get_bit(ins, 20);
	uint8_t W = get_bit(ins, 21);
	uint8_t B = get_bit(ins, 22);
	uint8_t U = get_bit(ins, 23);
	uint8_t P = get_bit(ins, 24);

	
	uint32_t address;
	//  Immediate : Offset/Index
	if (get_bits(ins, 27, 25) == 2) {
	
		// Load/Store : Word/Unsigned Byte - Immediate offset
		if (P == 1 && W == 0) {
			if (U == 1)
				address = rn + offset_12;
			else
				address = rn - offset_12;
		}
		
		// Load/Store : Word/Unsigned Byte - Immediate pre-indexed
		else if (P == 1 && W == 1) {
			if (U == 1)
				address = rn + offset_12;
			else
				address = rn - offset_12;
			if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p)))
				rn = address;
		}
		// Load/Store : Word/Unsigned Byte - Immediate post-indexed
		else if (P == 0 && W == 0) {
			address = rn;
			if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p))) {
				if (U == 1)
					rn = rn + offset_12;
				else
					rn = rn - offset_12;
			}
		}
	}
	
	// Register: Offset/Index
	else if (get_bits(ins, 27, 25) == 3 && get_bits(ins, 11, 4) == 0) {
		rm = arm_read_register(p, rm);
		
		// Load/Store : Word/Unsigned Byte - Register offset
		if (P == 1 && W == 0) {
			if (U == 1)
				address = rn + rm;
			else
				address = rn - rm;
		}
		// Load/Store : Word/Unsigned Byte - Register pre-indexed
		else if (P == 1 && W == 1) {
			if (U == 1)
				address = rn + rm;
			else
				address = rn - rm;
		if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p)))
			rn = address;
		}
		//Load/Store : Word/Unsigned Byte - Register post-indexed
		else if (P == 0 && W == 0) {
			address = rn;
			if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p))) {
				if (U == 1)
					rn = rn + rm;
				else
					rn = rn - rm;
			}
		}
	}

	// ScaledRegister : Offset/Index
	else if (get_bits(ins, 27, 25) == 3 && get_bit(ins, 4) == 0) {
		uint32_t shift_imm = get_bits(ins, 11, 7);
		uint8_t shift = get_bits(ins, 6, 5);
		uint32_t index;
		rm = arm_read_register(p, rm);
		
		//Load/Store : Word/Unsigned Byte - Scaled register offset
		if (P == 1 && W == 0) {
			switch(shift) {
				case LSL: index = rm << shift_imm; break;
				case LSR: 
					if (shift_imm == 0)/* LSR #32 */ index = 0;
					else index = rm >> shift_imm;
				
				break;
				case ASR:
				if (shift_imm == 0) { /* ASR #32 */
					if (get_bit(rm, 31) == 1) index = 0xFFFFFFFF;
					else index = 0;
				}
				else
					index = asr(rm, shift_imm);
				break;
				case ROR:
					if (shift_imm == 0) /* RRX */
						index = (get_bit(arm_read_cpsr(p), C) << 31) | (rm >> 1);
					else
						index = ror(rm, shift_imm);
				break;
				default: break;
			}
			if (U == 1)
				address = rn + index;
			else
				address = rn - index;
		}
		
		// Load/Store : Word/Unsigned Byte - Scaled register pre-indexed
		else if (P == 1 && W == 1) {
			switch(shift) {
				case LSL: index = rm << shift_imm; break;
				case LSR: 
					if (shift_imm == 0) /* LSR #32 */
						index = 0;
					else
						index = rm >> shift_imm;
				break;
				
				case ASR:
					if (shift_imm == 0) { /* ASR #32 */
						if (get_bit(rm, 31) == 1)
							index = 0xFFFFFFFF;
						else
							index = 0;
						}
						else index = asr(rm, shift_imm);
				break;
				case ROR:
				/* RRX */
					if (shift_imm == 0) /* RRX */
						index = (get_bit(arm_read_cpsr(p), C) << 31) | (rm >> 1);
					else /* ROR */
					 	index = ror(rm, shift_imm);
				break;
				default: break;
			}
		if (U == 1)
			address = rn + index;
		else 
			address = rn - index;
		if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p)))
			rn = address;
	}

	 	// Load/Store : Word/Unsigned Byte - Scaled register post-indexed
	 	else if (P == 0 && W == 0) {
	 		address = rn;
	 		switch(shift) {
	 			case LSL: index = rm << shift_imm; break;
	 			case LSR:
	 				if (shift_imm == 0) /* LSR #32 */ index = 0;
	 				else index = rm >> shift_imm;
	 			break;
	 			case ASR:
	 				if (shift_imm == 0) /* ASR #32 */ {
	 					if (get_bit(rm, 31) == 1) index = 0xFFFFFFFF;
	 					else index = 0;
	 				}
	 				else index = asr(rm, shift_imm);
	 			break;
	 			
	 			case ROR:
	 				if (shift_imm == 0) /* RRX */
	 					index = (get_bit(arm_read_cpsr(p), C) << 31) | (rm >> 1);
	 				else 
						index = ror(rm, shift_imm);
	 			break;
	 			default: break;
	 		}
	 		if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p))) {
	 			if (U == 1) 
					rn = rn + index;
	 			else  
					rn = rn - index;
	 		}
	 	}
 	}
 	
 	uint8_t byte;
 	uint32_t word;
 	// Load
 	if (L == 1) {
 		if (B == 1) {
 			// Load : Unsigned Byte
 			arm_read_byte(p, address, &byte);
 			arm_write_register(p, rd, (uint32_t) byte);
 		}
 		else {
 			// Load : Word
 			arm_read_word(p, address, &word);
 			arm_write_register(p, rd, word);
 		}
 	}
 	// Store
 	else {
 		if (B == 1) {
 			// store unsigned byte
 			byte = arm_read_register(p, rd);
 			arm_write_byte(p, address, byte);		
 		}
 		else {
 			// store word
 			word = arm_read_register(p, rd);
 			arm_write_word(p, address, word);
 		}
 	}
    return 0;
}

// load store multiple LDM(1) , STM(1)
int arm_load_store_multiple(arm_core p, uint32_t ins) {
	uint8_t cond = get_bits(ins, 31, 28);

	uint32_t rn = get_bits(ins, 19, 16);

	uint16_t register_list = get_bits(ins, 15, 0);
	uint8_t L = get_bit(ins, 20);
	uint8_t W = get_bit(ins, 21);
	uint8_t U = get_bit(ins, 23);
	uint8_t P = get_bit(ins, 24);

	rn = arm_read_register(p, rn);
	
	uint32_t start_address, end_address;
	int i = 0;
	
	// Load/Store : Multiple - Increment after
	if (P == 0 && U == 1) {
		start_address = rn;
		end_address = rn + (number_set_bits_in(register_list) * 4) - 4;
		if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p)) && W == 1)
			rn = rn + (number_set_bits_in(register_list) * 4);
	}
	// Load and Store Multiple - Increment before
	else if (P == 1 && U == 1) {
		start_address = rn + 4;
		end_address = rn + (number_set_bits_in(register_list) * 4);
		if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p)) && W == 1)
			rn = rn + (number_set_bits_in(register_list) * 4);
	}
	// Load/Store : Multiple - Decrement after
	else if (P == 0 && U == 0) {
		start_address = rn - (number_set_bits_in(register_list) * 4) + 4;
		end_address = rn;
		if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p)) && W == 1)
			rn = rn - (number_set_bits_in(register_list) * 4);
	}
	// Load/Store : Multiple - Decrement before
	else if (P == 1 && U == 0) {
		start_address = rn - (number_set_bits_in(register_list) * 4);
		end_address = rn - 4;
		if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p)) && W == 1)
			rn = rn - (number_set_bits_in(register_list) * 4);
	}
	uint32_t address;
	// LDM
	if (L) {
		uint32_t value;
		
		if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p))) {
			address = start_address;
			for (i = 0; i <= 14; i++) {
				if (get_bit(register_list, i) == 1) {
					arm_read_word(p, address, &value);
					arm_write_register(p, i, value);
					address = address + 4;
				}
			}
			if (get_bit(register_list, 15) == 1) {
				value = arm_read_word(p, address, &value);
				arm_write_register(p, 15, value & 0xFFFFFFFE);
				//T Bit = value[0];
				address = address + 4;
			}
			assert(end_address == (address - 4));
		}
	}
	// STM
	else {
		// MemoryAccess(B-bit, E-bit)
		if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p))) {
			address = start_address;
			for (i = 0; i <= 15; i++) {
				if (get_bit(register_list, i) == 1) {
					arm_write_word(p, address, arm_read_register(p, i));
					address = address + 4;
				}
			}
		assert(end_address == (address - 4));
		}
	}
    return 0;
}

//Load/Store Signed Byte : HalfWord - DoubleWord
// LDRH |STRH
int arm_load_store_miscellaneous(arm_core p, uint32_t ins) {

	uint32_t cond = get_bits(ins, 31, 28);
	uint8_t  L = get_bit(ins, 20);
	uint8_t  B = get_bit(ins, 22);
	uint8_t  U = get_bit(ins, 23);
	uint8_t  H = get_bit(ins, 5);
	uint8_t  S = get_bit(ins, 6);
	
	uint32_t rd = get_bits(ins, 15, 12);
	

	uint32_t rn = get_bits(ins, 3, 0);
	rn = arm_read_register(p, rn);
	
	uint32_t rm = get_bits(ins, 19, 16);
	rm = arm_read_register(p, rn);
	

	uint32_t immedH = get_bits(ins, 11, 8);
	uint32_t immedL = get_bits(ins, 3, 0);
	
	uint32_t offset_8, address;
	uint32_t CP15_reg1_Ubit = 1;

	uint32_t extended_data;
	uint16_t data_half;
	uint8_t data_byte;
	
	// Miscellaneous Load/Store - Immediate offset
	if ( get_bits(ins, 27, 24) == 1 && get_bits(ins, 22, 21) == 2 && get_bit(ins, 7) == 1 && get_bit(ins, 4) == 1 ) {
		offset_8 = (immedH << 4) | immedL;
		if (U == 1)
			address = rn + offset_8;
		else
			address = rn - offset_8;
	}
	
	// Miscellaneous Load/Store  - Register offset
	else if (get_bits(ins, 27, 24) == 1 && get_bits(ins, 22, 21) == 0 && get_bit(ins, 7) == 1 && get_bit(ins, 4) == 1) {
		if (U == 1)
			address = rn + rm;
		else /* U == 0 */
			address = rn - rm;	
	}
	//  Miscellaneous Load/Store  - Immediate pre-indexed
	else if (get_bits(ins, 27, 24) == 1 && get_bits(ins, 22, 21) == 3 && get_bit(ins, 7) == 1 && get_bit(ins, 4) == 1) {
		offset_8 = (immedH << 4) | immedL;
		if (U == 1)
			address = rn + offset_8;
		else
			address = rn - offset_8;
		if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p)))
			rn = address;
		}

		//  Miscellaneous Load/Store  - Register pre-indexed
	else if (get_bits(ins, 27, 24) == 1 && get_bits(ins, 22, 21) == 1 && get_bit(ins, 7) == 1 && get_bit(ins, 4) == 1) {
		if (U == 1)
			address = rn + rm;
		else
			address = rn - rm;
		if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p)))
			rn = address;
	}

		//  Miscellaneous Load/Store  - Immediate post-indexed
	else if (get_bits(ins, 27, 24) == 0 && get_bits(ins, 22, 21) == 2 && get_bit(ins, 7) == 1 && get_bit(ins, 4) == 1) {
		address = rn;
		offset_8 = (immedH << 4) | immedL;
		if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p))) {
			if (U == 1)
			rn = rn + offset_8;
			else
			rn = rn - offset_8;
		}
	}
	// Miscellaneous Load/Store  - Register post-indexed
	else if (get_bits(ins, 27, 24) == 0 && get_bits(ins, 22, 21) == 2 && get_bit(ins, 7) == 1 && get_bit(ins, 4) == 1) {
		address = rn;
		if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p))) {
			if (U == 1)
				rn = rn + rm;
			else 
				rn = rn - rm;
		}
	}

	// Load
	if (L == 1) {
		// LDRH
		if (H == 1) {
			
			if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p))) {
				if (CP15_reg1_Ubit == 0) {
					if ( get_bits(address, 7, 0) == 0) arm_read_half(p, address, &data_half);
					// UNPREDICTABLE data
					else data_half = 0;
				}
				else
					arm_read_half(p, address, &data_half);
				extended_data = data_half;
				if (S && get_bit(data_half, 15))
					extended_data = ((~0) << 16) | data_half;

				arm_write_register(p, rd, extended_data);
			}
		}
		else if (B == 1) {
			
			if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p))) {
				arm_read_byte(p, address, &data_byte);
				if (S && get_bit(data_byte, 7))
					extended_data = ((~0) << 8) | data_byte;
				arm_write_register(p, rd, extended_data);
			}
		}
	}
	// Store
	else {
		// STRH
		if (H == 1) {
			
			if (condition_OK(cond,get_flag_N(p),get_flag_Z(p),get_flag_C(p),get_flag_V(p))) {
				if (CP15_reg1_Ubit == 0) {
					if ( get_bits(address, 7, 0) == 0)
						arm_write_half(p, address, (uint16_t) get_bits(arm_read_register(p, rd), 15, 0));
					// UNPREDICTABLE data
					else
						arm_write_half(p, address, 0); 
					}
				else
					arm_write_half(p, address, (uint16_t) get_bits(arm_read_register(p, rd), 15, 0));
			}
		}
	}
	return 0;
}

int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}
