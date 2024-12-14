class A:
    a = None

    def __init__ (self, a):
        self.a = a
    
    def print (self):
        print (self.a)

class B:
    b = None

    def __init__ (self, b):
        self.b = b
    
    def print (self):
        print (self.b)

p = A(10)
q = B(11)

p.print = q.print
p.print()