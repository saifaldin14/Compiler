def double f(double x, double y)
    double a;
    double i;
    i = 1.0;
    while(i < y) do
         if(x < 5.0) then
              a=x+y*i;
         fi;
         i = i + 1;
     od;
    return a;
fed;
double s;
s=f(2.4,1)+f(1.3,2.4)+f(1.5 - 10,0.1);
print(s).
