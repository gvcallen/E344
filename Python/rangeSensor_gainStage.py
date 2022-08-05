import math


m = 5.97
b = -2.82
k = 10**3
Vref = 3.3
fo = 0.6

Rf = 10 * k

Rg = Rf / (m - 1)
Rg2 = Rg / 10
Rg1 = Rg - Rg2
VrefP = (-b * Rg1) / (Rg1 + Rf)
R1 = ((Rg2) * (Vref - VrefP)) / (VrefP)

#Co = 1 / (2 * math.pi * Ro * fo)

x = 5