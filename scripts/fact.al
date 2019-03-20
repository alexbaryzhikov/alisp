# Compute a factorial of a number

(def fact (func (n)
    (if (== n 1)
        1
        (* n (fact (- n 1))))))

(println "Factorial of 7: " (fact 5))
