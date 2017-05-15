# Compute n-th fibonacci number

(def fib (func (n)
    (def next_fib (func (a b c)
        (if c
            (next_fib b (+ a b) (- c 1))
            b)))
    (next_fib 0 1 n)))

# Print n fibonacci numbers

(def print_n_fibs (func (n)
    (def c 0)
    (def loop (func ()
        (if (<= c n)
            (block
                (print (fib c) " ")
                (inc c)
                (loop)))))
    (loop)))

(print_n_fibs 11) (println)
