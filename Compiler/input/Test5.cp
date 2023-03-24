int x,i;
x=0;i=1;
while(i<10) do if (i>2)
then x=2*x else x=x*x; if (x>2) then x=i+x else x = i fi fi;
x = x+i*i; i=i+1
od;
print(x);.