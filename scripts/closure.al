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

