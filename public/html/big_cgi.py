#!/usr/bin/python
import string
import random
 
print("Content-type: text/html")
print()
# initializing size of string
N = 10000000
 
# generating random strings
res = ''.join(random.choice(string.ascii_uppercase +
                            string.digits) for i in range(N))

print("The generated random string : " + str(res))
