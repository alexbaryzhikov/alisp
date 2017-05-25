# Counter with hidden variable
(def make_counter (func()
    (def c 0)
    (func() (inc c))))

(def c (make_counter))

(def c1 (c)) (def c2 (c)) (def c3 (c))

(if (and (== c1 1) (== c2 2) (== c3 3))
    (println "OK -- Count to 3: " c1 " " c2 " " c3)
    (println "FAIL -- Count to 3: " c1 " " c2 " " c3))
