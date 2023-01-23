import string
import random
 
print("Content-type: text/html")
print()
# initializing size of string
N = 10000000
 
# using random.choices()
# generating random strings
res = ''.join(random.choices(string.ascii_uppercase +
                             string.digits, k=N))
 
# print result
print("The generated random string : " + str(res))
