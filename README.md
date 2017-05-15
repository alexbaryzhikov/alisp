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
where `token0` is a language instruction/operator, function name or expression that evaluates to function. Other tokens are anything including numbers, quoted strings, variables, expressions and are treated as arguments to `token0`. Language syntax is uniform, so anything you do will resemble this pattern. For example, making a block statement will look like
```
(block expr1 expr2...)
```
White spaces and newlines are ignored, but they are required to separate symbolic names and numbers, like in
```
(def x 1)
```
Variable name should not start with a number and may contain any symbols except (, ), ", #, $.

Comments start with #:
```
# This is a comment
```
## Language constants

Name    | Value
------- | -----------
`NULL`  | Null object  
`TRUE`  | 1  
`FALSE` | 0  
`E`     | Euler constant  
`PI`    | Pi  

## Keywords

Alisp has the following keywords:

Name      | Description
--------- | -----------
`def`     | define a variable: `(def var [expr])`. If expression is not present then variable initializes to NULL.  
`=`       | assign a value to a variable: `(= var expr)`. Variable must be defined before assignment.  
`if`      | branching instruction: `(if test pro_exp [con_expr])`.  
`func`    | create a function: `(func (var ...) expr1 expr2 ...)`. Expressions are function body.  
`block`   | create a block statement: `(block expr1 expr2 ...)`. Returns the value of the last expression in a list.  
`ret`     | return from a block statement: `(ret expr)`. Interrupts block evaluation and returns a value of an expression.  
`null?`   | test if object/expression is a NULL: `(null? expr)`.  
`print`   | print arguments to stdout: `(print expr1 expr2 ...)`. Arguments may be quoted strings `"this is string"`.  
`println` | same as `print` but with newline at the end.  
`inc`     | increment the value of the argument: `(inc expr)`. Mutates the argument and returns the new value.  
`dec`     | decrement the value of the argument: `(dec expr)`. Mutates the argument and returns the new value.  

## Operators

**Relational** operators have form `(op arg1 arg2)`.

Name      | Description
--------- | -----------
`==`      | equal  
`!=`      | not equal  
`<`       | less than  
`>`       | more than  
`<=`      | less or equal  
`>=`      | more or equal  

**Logical** operators:
- `and`, `or` have form `(op arg1 arg2 ...)`
- unary `not` have form `(op arg)`

**Arithmetic** operators have form `(op arg1 arg2 ...)`.

Name      | Description
--------- | -----------
`+`       | add
`-`       | subtract
`*`       | multiply
`/`       | divide
`%`       | remainder of arg1 divided by arg2

**Bitwise** operators have form `(op arg1 arg2 ...)`:

Name      | Description
--------- | -----------
`&`       | and
`|`       | or
`^`       | xor
`~`       | flip bits
`<<`      | left shift
`>>`      | right shift

**Math** operators:

Form          | Description
------------- | -----------
`(acos x)`    | arc cosine of x in radians
`(asin x)`    | arc sine of x in radians
`(atan x)`    | arc tangent of x in radians
`(atan2 y x)` | arc tangent in radians of y/x based on the signs of both values to determine the correct quadrant
`(cos x)`     | cosine of a radian angle x
`(cosh x)`    | hyperbolic cosine of x
`(sin x)`     | sine of a radian angle x
`(sinh x)`    | hyperbolic sine of x
`(tanh x)`    | hyperbolic tangent of x
`(exp x)`     | value of e raised to the xth power
`(frexp x)`   | mantissa of x value: x = mantissa * 2 ^ n
`(ldexp x y)` | x multiplied by 2 raised to the power of y
`(log x)`     | natural logarithm (base-e logarithm) of x
`(log10 x)`   | common logarithm (base-10 logarithm) of x
`(frac x)`    | fraction component of x (part after the decimal)
`(pow x, y)`  | x raised to the power of y
`(sqrt x)`    | square root of x
`(ceil x)`    | the smallest integer value greater than or equal to x
`(abs x)`     | absolute value of x
`(floor x)`   | the largest integer value less than or equal to x

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
