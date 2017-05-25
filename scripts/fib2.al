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
