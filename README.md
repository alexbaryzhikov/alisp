# Alisp
Alisp is a small functional scripting language with lisp-like syntax.

Features:
- read-eval-print-loop mode
- interpreting scripts from files
- whole range of math and logical operators
- output to stdout
- variables + assignment
- branching
- compound expressions
- functions + recursion
- closures

Author: Alex Baryzhikov

## Hello World!
```
(println "Hello World!")
```

## Quick Manual

Alisp expressions have form
```
(token0 token1 ...)
```
where `token0` is a language instruction/operator, callable object or expression that evaluates to callable object. Other tokens are anything including numbers, variables, expressions and are viewed as a arguments to `token0`. Language syntax is uniform, so anything you do will have this form. For example, making a block statement will look like
```
(block expr1 expr2...)
```
White spaces and newlines are ignored, but they are required to separate symbolic names and numbers, like in
```
(def x 1)
```
Variable name should not start with a number and may contain any symbols except '(', ')', '"', '#', '$'.

Comments start with '#':
```
# This is a comment
```
## Language constants
`NULL`\tNull object
`TRUE`      1
`FALSE`     0
`E`         Euler constant
`PI`        Pi

## Keywords

Alisp has the following keywords:

`def`define a variable: `(def var [expr])`. If expression is not present then variable initializes to NULL.  
`=`         assign a value to a variable: `(= var expr)`.  
`if`        branching instruction: `(if test pro_exp [con_expr])`.  
`func`      create a function: `(func (var ...) expr1 expr2 ...)`. Expressions are function body.  
`block`     create a block statement: `(block expr1 expr2 ...)`. Returns the value of the last expression in a list.  
`ret`       return from a block statement: `(ret expr)`. Interrupts block evaluation and returns a value of an expression.  
`null?`     test if object/expression is a NULL: `(null? expr)`.  
`print`     print arguments to stdout: `(print expr1 expr2 ...)`. Arguments may be quoted strings `"this is string"`.  
`println`   same as `print` but with newline at the end.  
`inc`       increment the value of the argument: `(inc expr)`. Mutates the argument and returns the new value.  
`dec`       decrement the value of the argument: `(dec expr)`. Mutates the argument and returns the new value.  

## Operators

**Relational** operators have form `(op arg1 arg2)`.

- `==` equal, `!=` not equal, `<`  less than, `>`  more than, `<=` less or equal, `>=` more or equal


**Logical** operators:
- `and`, `or` have form `(op arg1 arg2 ...)`
- unary `not` have form `(op arg)`


**Arithmetic** operators:

+
-
*
/
%

**Bitwise** operators:
&
|
^
~
<<
>>

**Math** operators:
acos
asin
atan
atan2
cos
cosh
sin
sinh
tanh

exp
frexp
ldexp
log
log10
frac
pow
sqrt
ceil
abs
floor

## Example program

This is a simple example of how a program on Alisp looks like. It calculates a factorial of a number.

```
# fact: compute a factorial of a number
(def fact (func(n)
    (if (== n 1)
        1
        (* n (fact (- n 1))))))

(println "Factorial of 7: " (fact 7))
```
More examples in scripts/ directory.

And don't forget to have fun!
