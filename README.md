# TDMA
This is a simple case of using Tomas algorithm to solve a mathematical equation of the type d²y/dx² = F(x) where we need to find y.


* Thomas Algorithm:
*
* d²f/dx² = F(x)
* yi = alph * yi+1 + beta			and 		Ci * yi = Ai * yi+1 + Bi * yi-1 + Di
* => d²f/dx² = d/dx (yi+1 / h) - d/dx (yi / h)
* = 1/h² (yi+1  - 2yi + yi-1) = F(x)
* => Ai = 1  && 	Bi = 1   &&		Ci = 2		&&		Di = - h² * F(x)

*		alph(i) = Ai / (Ci - Bi * alph(i - 1))
*		beta(i) = (Bi + Di) / (Ci - Bi * alph(i - 1))

*		for i = 0 	B0 = 0 		&&		for i = N	AN = 0
