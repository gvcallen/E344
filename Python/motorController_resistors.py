Cmax = 7.2
Cmin = 1.8
Smax = 3.1
Smin = 0.1
Rmin = 0.6
Rmax = 3.3

L = (Rmax - Rmin)
W = (Smax - Smin)
Rc = Rmax
Sc = Smin
h = Cmax - Cmin
hb = Cmin

m1 = -h/W
k1 = h + h/W * Sc + hb
m2 = h/L
k2 = h - (h/L) * Rc + hb

print("m1 = %.4f" % m1)
print("k1 = %.4f" % k1)
print("m2 = %.4f" % m2)
print("k2 = %.4f" % k2)

print("Use S when %.3f * S + %.3f > R" % (-L/W, (L/W) * Smax))