# Create closure

(def make_counter (func ()
    (def c 0)
    (func () (inc c))))

(def x (make_counter))

(print (x) " ")
(print (x) " ")
(print (x) " ")
(print (x) " ")
(println (x))

