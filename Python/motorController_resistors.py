from scipy.optimize import fsolve
import math

def equations(p):
    x, y = p
    return ((15 * x + 9.9 * y) / (x + y) - 7.2, (15 * x + 1.8 * y) / (x + y) - 1.2)

x, y = fsolve(equations, (10, 10))

print((x, y))