#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**
 * Structure to hold the roots of a quadratic equation: ax^2 + bx + c = 0
 */
typedef struct {
    double real1;
    double imag1;
    double real2;
    double imag2;
    int is_complex;
} QuadraticRoots;

/**
 * Calculates the roots based on coefficients a, b, and c.
 * Returns 1 if successful, 0 if 'a' is zero (not a quadratic equation).
 */
int solve_quadratic(double a, double b, double c, QuadraticRoots *roots) {
    if (a == 0) {
        return 0; // Division by zero condition
    }

    double discriminant = (b * b) - (4 * a * c);

    if (discriminant >= 0) {
        // Real and distinct or real and equal roots
        roots->real1 = (-b + sqrt(discriminant)) / (2 * a);
        roots->real2 = (-b - sqrt(discriminant)) / (2 * a);
        roots->imag1 = 0.0;
        roots->imag2 = 0.0;
        roots->is_complex = 0;
    } else {
        // Complex conjugate roots
        roots->real1 = -b / (2 * a);
        roots->real2 = -b / (2 * a);
        roots->imag1 = sqrt(-discriminant) / (2 * a);
        roots->imag2 = -sqrt(-discriminant) / (2 * a);
        roots->is_complex = 1;
    }

    return 1;
}

int main() {
    double a, b, c;
    QuadraticRoots roots;

    printf("====================================\n");
    printf("     Quadratic Equation Solver      \n");
    printf("====================================\n");
    printf("Equation Format: ax^2 + bx + c = 0\n\n");

    printf("Enter coefficient a: ");
    if (scanf("%lf", &a) != 1) return 1;
    
    printf("Enter coefficient b: ");
    if (scanf("%lf", &b) != 1) return 1;
    
    printf("Enter coefficient c: ");
    if (scanf("%lf", &c) != 1) return 1;

    printf("\nProcessing equation: (%.2f)x^2 + (%.2f)x + (%.2f) = 0\n", a, b, c);

    if (!solve_quadratic(a, b, c, &roots)) {
        printf("Error: Coefficient 'a' cannot be 0. This is a linear equation.\n");
        return 1;
    }

    // Displaying Results
    printf("------------------------------------\n");
    if (roots.is_complex) {
        printf("Roots are complex conjugates:\n");
        printf("Root 1 = %.4f + %.4fi\n", roots.real1, roots.imag1);
        printf("Root 2 = %.4f - %.4fi\n", roots.real2, fabs(roots.imag2));
    } else {
        if (roots.real1 == roots.real2) {
            printf("Roots are real and equal:\n");
            printf("Root 1 = Root 2 = %.4f\n", roots.real1);
        } else {
            printf("Roots are real and distinct:\n");
            printf("Root 1 = %.4f\n", roots.real1);
            printf("Root 2 = %.4f\n", roots.real2);
        }
    }
    printf("====================================\n");

    return 0;
}