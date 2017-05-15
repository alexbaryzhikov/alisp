# Alisp
Alisp -- a small functional scripting language with lisp-like syntax.

Author: Alex Baryzhikov

Features:
* read-eval-print-loop mode
* interpreting scripts from files
* whole range of math and logical operators
* output to stdout
* variables + assignment
* branching
* compound expressions
* functions + recursion
* closures

## Quick Manual

Generally all Alisp expressions have form
  (token0 token1...)
 where token0 is a language instruction/operator, callable object or expression that evaluates to callable object. 

