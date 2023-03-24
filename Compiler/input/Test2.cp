int a,b,r;
a=21; b=15;
while (b<>0) do
	r = a % b;
	a=b;
	b=r;
od;
print(a).
