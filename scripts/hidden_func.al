# The inner function is created within 'foo' environment.
# It is not bound directly in it, but rather exported via
# a list. This should be detected and 'foo' environment
# should not be garbage collected after 'foo' returns,
# because 'x' is reachable through anonymous function in
# exported list.
(def foo (func()
    (def x 0)
    (list (func ()
        (inc x)))))

(def bar (foo))

# Now 'bar' is a list with anonymous function in it.
# Let's call it.
(if (== ((list_get bar 0)) 1)
    (println "OK -- Hidden function export")
    (println "FAIL -- Hidden function export"))
