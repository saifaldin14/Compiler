def int gcd(int a, int b)
    if (a==b) then
        return (a)
    fi;

    if (a>b) then
        a = b + 100 * 9 - (a + 90 - (4 / 3 + b * 6 - 8 + 7 * b));
        int d;
        double c;
        return(gcd(a-b,b))
    else
        int q;
        b = a;
        return(gcd(a,b-a))
    fi;
fed;

def double testDouble()
    int r;
    double r5, t6, y9;
    r5 = 3.4;
    t6 = 1.6;
    return r5 * t6;
fed;

int c;
print gcd(21,15);
print 45;
print 2*(gcd(21, 28) + 6).
