/*------------------------------------------------------------------------------------------------------*/
/* Problem artylerzysty                                                                                 */
/* Program liczy tor lotu pocisku wystrzelonego z armaty                                                */
/* Uwzglednia przypadek w ktorym nie istnieje opor powietrza oraz taki w ktorym opor powietrza istnieje */
/*------------------------------------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define G 9.81 // m/s^2 przyspieszenie ziemskie
#define outfile "./output.txt"
#define outop "./outopor.txt"

// Makro dla oporu powietrza dla kuli o kalibrze 560mm
// ro - Gestosc powietrza (g/l - gram na litr)  Cx - Wspolczynnik oporu  s - Powierzchnia czolowa kuli (mm - milimetry)
//                 ro      Cx   s
#define k(Cx, s) ( 1.293 * Cx * s )

struct throw{
    double  x[2],
    y[2],
    Vx[2],
    Vy[2];
};

typedef struct throw throw;

// Funkcja liczace predkosc, pozycje oraz przyspieszenie ze wzorow fizycznych

double velocity(double V0, double t, double a){
    double V = V0 + a * t;
    return V;
}

double position(double z0, double V0, double t, double a){
    double z = z0 + V0 * t + (a * pow(t, 2)) / 2;
    return z;
}

double acceleration(double m, double g, double v, double k){
    double a = ( (-m * g) + (- k * pow(v, 2)) ) / m;
    return a;
}

int main()
{
    // Deklaracja zmiennych
    FILE *output,
         *outopor;

    int projectile;

    throw res,
          no_res;

    double m, // Masa
    V0, // Predkosc poczatkowa
    V0x, // Predkosc poczatkowa wzgledem osi X
    V0y, // Predkosc poczatkowa wzgledem osi Y
    t = 0, // Czas
    A, // Kat
    Tc, // Czas przelotu calkowity(bez oporu)
    Z, // Zasieg maksymalny
    ax, // przyspieszenie w poziomie
    ay, // przyspieszenie w pionie
    Cx, // Wspolczynnik oporu
    s, // Kaliber pocisku
    F; // Sila pionowa rzutu z oporem



    // Otwieranie plikow i sprawdzanie czy istnieja
    output = fopen(outfile, "w");
    if (output == NULL) {
        printf("Plik outfile.txt nie istnieje!");
        return 1;
    }
    outopor = fopen(outop, "w");
    if (outopor == NULL) {
        printf("Plik outopor.txt nie istnieje!");
        return 1;
    }

    // Wczytywanie zmiennych
    printf("Podaj dane\n");
    printf(" Predkosc poczatkowa [m/s]:");
    scanf("%lf", &V0);

    printf(" Kat [stopnie]:");
    scanf("%lf", &A);

    printf(" Mase pocisku [kg]:");
    scanf("%lf", &m);

    printf("\n1. Kula\n2. Pocisk artyleryjski \n3. Wpisz wlasny Cx\nWybierz pocisk:");
    scanf("%d", &projectile);


    switch(projectile)
    {
        case 1:
            Cx = 0.47;
            break;
        case 2:
            Cx = 0.04;
            break;
        case 3:
            printf("Podaj wspolczynnik oporu:");
            scanf("%lf", &Cx);
            break;
        default:
            Cx = 0.47;
    }

    printf("Kaliber pocisku [mm]:");
    scanf("%lf",&s);

    if (V0 <= 0 || A <= 0 || m <= 0 || Cx <= 0 || s <= 0)
    {
        printf("Dane nie moga byc ujemne.\nPodaj wlasciwe dane.");
        return 1;
    }

    // Zamiana kalibru z milimetrow na metry
    s = s / 1000;

    // Zamiana kalibru na pole przekroju poprzecznego pocisku
    s = M_PI * (pow(s, 2)/2); // pi * r^2

    // Zamiana stopni na radiany
    A = (A * M_PI)/180;

    // Obliczanie predkosci
    V0x = V0 * cos(A);
    V0y = V0 * sin(A);

    // Poczatkowe wartosci dla ruchu z oporem powietrza
    res.Vx[0] = V0x;
    res.Vy[0] = V0y;
    res.x[0] = 0;
    res.y[0] = 0;

    // Rzut ukosny
    do {
        // Bez oporu
        no_res.x[0] = position(0, V0x, t, 0);
        no_res.y[0] = position(0, V0y, t, -G);

        fprintf(output, "%lf %lf\n", no_res.x[0], no_res.y[0]);


        // Z oporem
        if (res.y[0] >= 0) {

            ax = acceleration(m, 0, res.Vx[0], k(Cx, s));
            ay = acceleration(m, G, res.Vy[0], k(Cx, s));

            res.Vx[1] = velocity(res.Vx[0], t, ax);
            res.Vy[1] = velocity(res.Vy[0], t, ay);

            res.x[1] = position(res.x[0], res.Vx[0], t, ax);
            res.y[1] = position(res.y[0], res.Vy[0], t, ay);

            res.Vx[0] = res.Vx[1];
            res.Vy[0] = res.Vy[1];
            res.x[0] = res.x[1];
            res.y[0] = res.y[1];

            F = ay * m;
            fprintf(outopor, "%lf %lf %lf %lf\n", res.x[0], res.y[0], ay, F);
        }

        t += 0.001;
        // t += 0.0001;
    } while (no_res.y[0] >= 0);

    Tc = (2 * V0 * sin(A)) / G;
    Z = V0x * Tc;

    printf("\n");
    printf("Calkowity czas przelotu(Bez oporu): %lf\n", Tc);
    printf("Zasieg przelotu(Bez oporem): %lf\n",Z );


    fclose(output);
    fclose(outopor);
    system("gnuplot -p ./gnuplot.gp");
    return 0;
}
