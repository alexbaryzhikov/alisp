# Compute a factorial of a number
# Do not use explicit self-reference

(def y_fact
    ((func (!)
        (func (n)
            ((! !) n)))
    (func (!)
        (func (n)
            (if (== n 0)
                1
                (* n ((! !) (- n 1))))))))

(println (y_fact 5))
