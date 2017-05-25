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
