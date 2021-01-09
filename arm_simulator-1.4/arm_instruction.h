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
#ifndef __ARM_INSTRUCTION_H__
#define __ARM_INSTRUCTION_H__
#include "arm_core.h"

#define EQ 0 		// 0000 EQ Equal Z set
#define NE 1 		// 0001 NE Not equal Z clear
#define CS_HS 2 	// 0010 CS/HS Carry set/unsigned higher or same C set
#define CC_LO 3 	// 0011 CC/LO Carry clear/unsigned lower C clear
#define MI 4		// 0100 MI Minus/negative N set
#define PL 5		// 0101 PL Plus/positive or zero N clear
#define VS 6		// 0110 VS Overflow V set
#define VC 7		// 0111 VC No overflow V clear
#define HI 8		// 1000 HI Unsigned higher C set and Z clear
#define LS 9		// 1001 LS Unsigned lower or same C clear or Z set
#define GE 10		// 1010 GE Signed greater than or equal N set and V set, or N clear and V clear (N == V)
#define LT 11		// 1011 LT Signed less than N set and V clear, or N clear and V set (N != V)
#define GT 12 		// 1100 GT Signed greater than Z clear, and either N set and V set, or N clear and V clear (Z == 0,N == V)
#define LE 13		// 1101 LE Signed less than or equal Z set, or N set and V clear, or N clear and V set (Z == 1 or N != V)
#define AL 14 		// 1110 AL Always (unconditional) -
#define UX 15		// 1111 See Condition code 0b1111

int arm_step(arm_core p);

#endif
