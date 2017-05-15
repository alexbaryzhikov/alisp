# -----------------------------------------------------------------------------
#
# Test batch
#
# -----------------------------------------------------------------------------

# evaluating quoted string and numeric literal
(println "Numeric literal: " 3.141592)

# defining variable without initialization
(def x)
(println "Is x NULL? " (null? x))

# assignment
(= x (+ 1 (/ 1 (+ 1 (/ 1 (+ 1 (/ 1 (+ 1 (/ 1 2)))))))))
(println "Value of x after assignment: " x)

# old value shouldn't cause mem leak
(= x 100)
(println "Value of x after reassignment: " x)

# null objects
(println "Null objects: " NULL ", " ())

# branching and block statement
(if (> x 10)
    (println (block (def y (* x 2)) "OK -- correct branching"))
    (println "FAIL -- incorrect branching"))

(println "Variable defined in the branch block: y = " y)

# explicit return statement
(println (block
    (def z "OK -- block explicitly returned")
    (if z
        (ret z)
        (/ z 2))
    "FAIL -- returned block end"))

# function returning local value
(def foo (func() (def x (+ 2 3)) x))
(println "Returning local value: " (foo))

# -----------------------------------------------------------------------------
# Recursion

# Counter
(def count (func(n)
    (if n
        (block (print n " ") (dec n) (count n)))))

(print "Count down from 5 to 1: ")
(count 5) (println)

# Factorial
(def fact (func(n)
    (if (== n 1)
        1
        (* n (fact (- n 1))))))

(println "Factorial of 7: " (fact 7))

# Fibonacci numbers
(def fib (func(n)
    (def next_fib (func(a b c)
        (if c
            (next_fib b (+ a b) (- c 1))
            b)))
    (next_fib 0 1 n)))

(def print_n_fibs (func(n)
    (def c 0)
    (def loop (func()
        (if (<= c n)
            (block
                (print (fib c) " ")
                (inc c)
                (loop)))))
    (loop)))

(print "Fibonacci numbers: ")
(print_n_fibs 11) (println)


# -----------------------------------------------------------------------------
# Closure

# Counter with hidden variable
(def make_counter (func()
    (def c 0)
    (func() (inc c))))

(def c (make_counter))

(println "Count to 5: " (c) " " (c) " " (c) " " (c) " " (c))

# Export function with several hidden environments
(def bar (func()
    (def a 0)
    (def deep (func(b)
        (func(c)
            (println a b c)
            (inc a))))
    (deep " a ")))

(def dcl (bar))

(dcl "very") (dcl "deep") (dcl "closure")

