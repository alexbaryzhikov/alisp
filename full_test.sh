#!/bin/bash
valgrind --leak-check=full ./alisp scripts/test.al 
