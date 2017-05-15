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


## Quick Manual

Generally all Alisp expressions have form
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
Variables names must not start with a number and can contain any symbols except '(', ')', '"', '#', '$'.

Comments start with '#'.


