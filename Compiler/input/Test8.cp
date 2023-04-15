def int max(int x, int y)
int z;
if (x>y) then
    z = x
else
    z = y
fi;
return(z)
fed;

int a,b,c,d;
a = 2; 
b = a*2-3;
c = b/a+1;

d = max(max(a,b),c);

print d;.
