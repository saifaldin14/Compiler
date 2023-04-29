# Compiler
 
An implementation of a compiler written entirely in C++ that performs lexical, syntax and semantic analysis before finally creating intermediate code through Three Adress Code. 

The program defines its own programming language EZ as a simple and readable psuedocode style language. The code is loaded from files containing '.cp' extension. 

The language supports if-else conditional operations, while loops, recursion, function definitions, integer and double variables, boolean algebra and arithmetic operations.

Below is an example of EZ code:

## EZ Code Example

```c++:
def int gcd(int a, int b)
    if (a==b) then
        return (a)
    fi;
    
    if (a>b) then
        return(gcd(a-b,b))
    else
        return(gcd(a,b-a))
    fi;
fed;

print gcd(21,15);
print 45;
print 2*(gcd(21, 28) + 6).
```

## Description of Project
The program first reads the file contents using Double Buffering and breaks it up into a stream to tokens in the 'lexer.cpp' class. Lexical analysis is performed on the Tokens to ensure that they are correct and keywords are recognized.

Next, the program performs syntax analysis through the 'parser.cpp' class. It creates the FIRST and FOLLOW sets for the language and makes sure that the provided code is in the correct grammatical order and is synticatically correct.

Then, if the parsing phase is successful the program will perform semantic analysis to ensure that the code is not only syntactically correct but is also semantically correct. For example it will make sure that all variables are properly declared before being called, the return value of the function matches its return type and everything is in the correct scope.

Finally, after all of the analysis phases are complete the program will generated Three Adress Code corresponding to the input code as intermediate code. The program will generate temporary variables as needed within expressions and function calls. 

The final output of the program are two files containing the Symbol Table of the code and the generated Three Adress Code.

## Symbol Tables
For the above code the symbol table without the temporary variables is:

```c++:
GLOBAL SCOPE
	FUNCTION SCOPE
		gcd, integer, FUNCTION
		a, integer, FUNCTION
		b, integer, FUNCTION
```

For the above code the symbol table with the temporary variables is:

```c++:
GLOBAL SCOPE
	FUNCTION SCOPE
		gcd, integer, FUNCTION
		a, integer, FUNCTION
		b, integer, FUNCTION
			t1, id, IF
			t2, function, IF
			t3, id, ELSE
			t4, function, ELSE
	t1, integer, GLOBAL
	t2, integer, GLOBAL
	t3, function, GLOBAL
	t4, integer, GLOBAL
	t5, integer, GLOBAL
	t6, integer, GLOBAL
	t7, function, GLOBAL
	t8, integer, GLOBAL
	t9, integer, GLOBAL
	t10, integer, GLOBAL
	t11, integer, GLOBAL
	t12, integer, GLOBAL
 ```

## Three Address Code
For the above code the generated Three Adress Code is:
```c++:
B main
push{LR}
push{FP}

gcd 24:
Begin: 
b = fp + 8
a = fp + 12
cmp a, b
beq lab1
bgt lab2
b lab3

lab1:
FP - 4 = a
b exitgcd

lab2:
t1 = a - b 
push {t1}
push {b}
t2 = BL gcd
pop {b}
pop {t1}
FP - 4 = t2
b exitgcd

lab3:
t3 = b - a 
push {a}
push {t3}
t4 = BL gcd
pop {t3}
pop {a}
FP - 4 = t4
b exitgcd

exitgcd:
pop {FP}
pop {PC}

main: 
Begin 44: 
t1 = 21 
t2 = 15 
push{t1}
push{t2}
t3 = BL gcd
pop{t2}
pop{t1}
print(t3)

t4 = 45 
print(t4)

t5 = 21 
t6 = 28 
push{t5}
push{t6}
t7 = BL gcd
pop{t6}
pop{t5}
t8 = 2 
t9 = 6 
t10 = t9 + t7 
t11 = t10 * t8
print(t11)
```
