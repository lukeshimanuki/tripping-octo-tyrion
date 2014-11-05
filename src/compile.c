/* =============================================================================
 * @file compile.h
 * @author Luke Shimanuki
 * @date 2 Nov 2014
 * @brief Functions for converting symbolic structure to assembly.
 *
 * This file is part of MCC.
 *
 * -----------------------------------------------------------------------------
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Luke Shimanuki
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * ========================================================================== */

#include <stdio.h>

#include "symbol.h"
#include "string.h"

// iterative over lists, recursive over children
// eg: recurse over function->parameters->declare->variable
// iterative over declare, declare, add, call
// but the string list is iterative
struct String* compile(struct Symbol* symbols)
{
	struct String* assembly = NULL;
	while (symbols != NULL)
	{
		// create empty string
		struct String* current = newString(0);
		// create temporary buffer
		char buffer[32] = "";
		// process the current symbol
		switch (symbols->type)
		{
			// results stored in %eax
			case VARIABLE:
				break;
			case STRING:
				break;
			case VALUE:
				break;
			case TYPE:
				break;
			case DECLARE:
				break;
			case ASSIGN:
				break;
			case ADD:
				// place lhs in %eax and rhs in %ecx
				addString(current, compile(symbols->rhs));
				addString(current, getString("\tmov %eax,%ecx\n"));
				addString(current, compile(symbols->lhs));
				addString(current, getString("\tadd %ecx,%eax\n"));
				break;
			case SUBTRACT: // a - b: lhs = a, rhs = b
				// place lhs in %eax and rhs in %ecx
				addString(current, compile(symbols->rhs));
				addString(current, getString("\tmov %eax,%ecx\n"));
				addString(current, compile(symbols->lhs));
				addString(current, getString("\tsub %ecx,%eax\n"));
				break;
			case MULTIPLY:
				// place lhs in %eax and rhs in %ecx
				addString(current, compile(symbols->rhs));
				addString(current, getString("\tmov %eax,%ecx\n"));
				addString(current, compile(symbols->lhs));
				addString(current, getString("\timul %ecx,%eax\n"));
				break;
			case DIVIDE: // a / b: lhs = a, rhs = b
				// place lhs in %eax and rhs in %ecx, %edx = 0
				addString(current, compile(symbols->rhs));
				addString(current, getString("\tmov %eax,%ecx\n"));
				addString(current, compile(symbols->lhs));
				addString(current, getString("\tmov $0,%edx\n"));
				addString(current, getString("\tidiv %ecx\n"));
				break;
			case FUNCTION: // lhs: paramters; rhs: instructions
			{
				// header
				addString(current, getString("\t\n"));
				// label
				sprintf(buffer, "%s:\n", symbols->name);
				addString(current, getString(buffer));
				// save stack pointer
				addString(current, getString("\tpush %ebp\n"));
				addString(current, getString("\tmov %esp,%ebp\n"));
				// create scope
				
				// process parameters
				struct Symbol* parameter = symbols->lhs;
				unsigned int parameterIndex = 0;
				while (parameter)
				{
					// declare: reference to var is stored in %eax
					addString(current, compile(parameter));
					// assign: +8 because we push %ebp and instruction pointer
					sprintf(buffer, "\tmov %i(%ebp),(%eax)\n", 4 * parameterIndex + 8);
					addString(current, getString(buffer));
					parameterIndex++;
					parameter = parameter->next;
				}
				// process instructions
				struct Symbol* instruction = symbols->rhs;
				while (instruction)
				{
					addString(current, compile(instruction));
					instruction = instruction->next;
				}
				// restore scope
				
				break;
			}
			case CALL: // rhs: arguments
			{
				// process arguments in reverse order
				struct Symbol* argument = symbols->rhs;
				while (argument)
				{
					// evaluate
					struct String* arg = compile(argument);
					// push
					addString(current, getString("\tpush %eax\n"));
					// insert string at beginning
					addString(arg, current);
					current = arg;
				}
				// call the function, the result is stored in %eax
				// it implicitly pushes the instruction pointer
				sprintf(buffer, "\tcall %s\n", symbols->name);
				addString(current, getString(buffer));
				break;
			}
			case RETURN:
				// if there is an operand (to the right), store it in %eax
				if (symbols->rhs)
					addString(current, compile(symbols->rhs));
				// reset stack pointer and return
				addString(current, getString("\tmov %ebp,%esp\n"));
				addString(current, getString("\tpop %ebp\n"));
				addString(current, getString("\tret\n"));
				break;
		}
		if (assembly == NULL)
			assembly = current;
		else
			addString(assembly, current);
		// process the next symbol in the next cycle
		symbols = symbols->next;
	}
}
