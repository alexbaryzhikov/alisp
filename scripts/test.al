# -----------------------------------------------------------------------------
#
# Test batch
#
# -----------------------------------------------------------------------------

# null objects
(if (null? NULL)
    (println "OK -- Null object: " NULL)
    (println "FAIL -- Null object: " NULL))

# evaluating quoted string and numeric literal
(if 3.141592
    (println "OK -- Numeric literal: " 3.141592)
    (println "FAIL -- Numeric literal: " 3.141592))

# defining variable without initialization
(def x)
(if (null? x)
    (println "OK -- x is NULL")
    (println "FAIL -- x is not NULL"))

# assignment
(= x (+ 1 (/ 1 (+ 1 (/ 1 (+ 1 (/ 1 (+ 1 (/ 1 2)))))))))
(if (== x 1.625)
    (println "OK -- Value of x after assignment: " x)
    (println "FAIL -- Value of x after assignment: " x))

# old value shouldn't cause mem leak
(= x 100)
(if (== x 100)
    (println "OK -- Value of x after reassignment: " x)
    (println "FAIL -- Value of x after reassignment: " x))

# string comparison and block statement
(if (> "ba" "aaa")
    (println (block (def y (* x 2)) "OK -- String comparison"))
    (println "FAIL -- String comparison"))

(if (== y 200)
    (println "OK -- Variable defined in the branch block: y = " y)
    (println "FAIL -- Variable defined in the branch block: y = " y))

# type
(if (== (type x) "NUMBER")
    (println "OK -- Type of x: " (type x))
    (println "FAIL -- Type of x: " (type x)))

# copy
(= y (copy x))
(inc x)

(if (!= x y)
    (println "OK -- Copy: x = " x ", y = " y)
    (println "FAIL -- Copy: x = " x ", y = " y))

# branching-2
(def a 1)
(cond 
    (0         (println "FAIL -- cond"))
    ((> a 10)  (println "FAIL -- cond"))
    (else      (print "OK") (println " -- cond")))

# explicit return statement
(println (block
    (def z "OK -- Return from block")
    (if z
        (ret z))
    "FAIL -- Return from block"))

# function returning local value
(def foo (func() (def x (+ 2 3)) x))
(= a (foo))
(if (== a 5)
    (println "OK -- Returning local value: " a)
    (println "FAIL -- Returning local value: " a))


# -----------------------------------------------------------------------------
# Lists

(def tmp)

(def lst ())

(if lst
    (println "FAIL -- List is not empty: " lst)
    (println "OK -- Created empty list: " lst))

(= lst (list 0 1 (+ 1 1)))
(if (== (list_len lst) 3)
    (println "OK -- New list created: " lst)
    (println "FAIL -- New list created: " lst))

(list_add lst "ab" PI)
(if (and (== (list_len lst) 5) (== (list_get lst 3) "ab") (== (list_get lst 4) PI))
    (println "OK -- New elements appended to list: " lst)
    (println "FAIL -- New elements appended to list: " lst))

(list_ins lst 1 99)
(if (and (== (list_len lst) 6) (== (list_get lst 1) 99))
    (println "OK -- New element inserted to list: " lst)
    (println "FAIL -- New element inserted to list: " lst))

(if (= tmp (list_get lst -1))
    (println "OK -- Returned list element: " tmp)
    (println "FAIL -- Returned list element: " tmp))

(if (== (list_len (= tmp (list_get lst 2 100))) 4)
    (println "OK -- Subset of list elements: " tmp)
    (println "FAIL -- Subset of list elements: " tmp))

(if (== (= tmp (list_len lst)) 6)
    (println "OK -- List length: " tmp)
    (println "FAIL -- List length: " tmp))

(if (== (list_len (list_rem lst 4)) 5)
    (println "OK -- List element deleted: " lst)
    (println "FAIL -- List element deleted: " lst))

(if (== (list_len (= tmp (list_merge lst (list 4 5)))) 7)
    (println "OK -- Lists merged: " tmp)
    (println "FAIL -- Lists merged: " tmp))


# -----------------------------------------------------------------------------
# Recursion

# Counter
(def count (func(n l)
    (if n
        (block (list_add l n) (count (- n 1) l)))))

(= tmp ())
(count 3 tmp)

(if (and (== (list_get tmp 0) 3) (== (list_get tmp 1) 2) (== (list_get tmp 2) 1))
    (println "OK -- Count down from 3 to 1: " (list_get tmp 0) " " (list_get tmp 1) " " (list_get tmp 2))
    (println "FAIL -- Count down from 3 to 1: " (list_get tmp 0) " " (list_get tmp 1) " " (list_get tmp 2)))

# Factorial
(def fact (func(n)
    (if (== n 1)
        1
        (* n (fact (- n 1))))))

(if (== (= tmp (fact 7)) 5040)
    (println "OK -- Factorial of 7: " tmp)
    (println "FAIL -- Factorial of 7: " tmp))

# Fibonacci numbers
(def fib (func(n)
    (def next_fib (func(a b c)
        (if c
            (next_fib b (+ a b) (- c 1))
            b)))
    (next_fib 0 1 n)))

(def fib0 (fib 0)) (def fib1 (fib 1)) (def fib2 (fib 2)) (def fib3 (fib 3)) (def fib4 (fib 4))
(def fib5 (fib 5)) (def fib6 (fib 6)) (def fib7 (fib 7)) (def fib8 (fib 8)) (def fib9 (fib 9))
(def fib10 (fib 10))

(if (and (== fib0 1) (== fib1 1) (== fib2 2) (== fib3 3) (== fib4 5) (== fib5 8) (== fib6 13)
    (== fib7 21) (== fib8 34) (== fib9 55) (== fib10 89))
    (println "OK -- Fibonacci numbers: " fib0 " " fib1 " " fib2 " " fib3 " " fib4 " " fib5 " "
        fib6 " " fib7 " " fib8 " " fib9 " " fib10)
    (println "FAIL -- Fibonacci numbers: " fib0 " " fib1 " " fib2 " " fib3 " " fib4 " " fib5 " "
        fib6 " " fib7 " " fib8 " " fib9 " " fib10))


# -----------------------------------------------------------------------------
# Closure

# Counter with hidden variable
(def make_counter (func()
    (def c 0)
    (func() (inc c))))

(def c (make_counter))

(def c1 (c)) (def c2 (c)) (def c3 (c))

(if (and (== c1 1) (== c2 2) (== c3 3))
    (println "OK -- Count to 3: " c1 " " c2 " " c3)
    (println "FAIL -- Count to 3: " c1 " " c2 " " c3))

# Export function with several hidden environments
(def bar (func()
    (def a 0)
    (def deep (func()
        (func()
            (inc a))))
    (deep)))

(def dcl (bar))

(if (and (== (dcl) 1) (== (dcl) 2) (== (dcl) 3))
    (println "OK -- Deep closure")
    (println "FAIL -- Deep closure"))

# The inner function is created within foo evironment,
# but is not bound in it. It is exported via list. It
# should be detected for ghosting of foo environment.
(def hid (func()
    (def x 0)
    (list (func () 
        (inc x)))))

(def hf (hid))
# Now 'a' is a list with anonimous function in it.
# Let's call it.
(if (== ((list_get hf 0)) 1)
    (println "OK -- Hidden function export")
    (println "FAIL -- Hidden function export"))
