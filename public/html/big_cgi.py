import stringOL6B6K5P654JS2D48
import random
 
print "Content-type: text/html"
print
# initializing size of string
N = 1000000
 
# generating random strings
res = ''.join(random.choice(string.ascii_uppercase +
                            string.digits) for i in range(N))

print("The generated random string : " + str(res))