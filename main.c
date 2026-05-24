#include <stdio.h>
#include <math.h>

// The mathematical function we want to integrate: f(x) = x^2
double function_to_integrate(double x) {
    return x * x;
}

// Computes the definite integral using the Trapezoidal Rule
double trapezoidal_integration(double a, double b, int intervals) {
    double h = (b - a) / intervals;
    double integral = (function_to_integrate(a) + function_to_integrate(b)) / 2.0;

    for (int i = 1; i < intervals; i++) {
        double x = a + i * h;
        integral += function_to_integrate(x);
    }

    return integral * h;
}

int main() {
    double lower_limit = 0.0;
    double upper_limit = 3.0;
    int intervals = 1000;

    printf("========================================\n");
    printf("      Numerical Integration Engine      \n");
    printf("========================================\n");
    printf("Integrating f(x) = x^2 from %.1f to %.1f\n", lower_limit, upper_limit);

    double result = trapezoidal_integration(lower_limit, upper_limit, intervals);

    printf("----------------------------------------\n");
    printf("Calculated Area (Integral): %.5f\n", result);
    printf("Expected Exact Value      : %.5f\n", (pow(upper_limit, 3) - pow(lower_limit, 3)) / 3.0);
    printf("========================================\n");

    return 0;
}