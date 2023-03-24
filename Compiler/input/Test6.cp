def int gcd(int a, int b)
int r;
if a<0 then a = -a fi;
if b<0 then b = -b fi;

if a<b then r=a; a=b; b=r fi;
while (b<>0) do
	r = a % b;
	a=b;
	b=r;
od;
return(a)
fed;

int x,y,z;
x=12; y=15;

z=gcd(x,y);
print(z);

z=gcd(x+y, y);
print(z);


z=gcd(x+y, x);
print(z);
.
