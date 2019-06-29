#include <algorithm>
#include <cmath>
#include <cstdio>

using namespace std;

/**
 * @brief Solve linear equations ax=b using Gauss-Jordan elimination 
 * 
 * @param a  mxm matrix, changed in this function
 * @param b  mxn matrix, save the solve x
 * 
 * @return true is if a is a non-singular matrix
 */
bool gauss_jordan_solve(float* a, int a_step, float* b, int b_step, int m, int n) {
    const float epsilon = 1e-11;
    for (int j=0; j < m; ++j) {
        // NOTES:
        // a(i,j) is a[i*a_step+j]
        // b(i,j) is b[i*b_step+j]

        // select the max value in a(:,j)
        int k = j;
        for (int i=k+1; i < m; ++i) {
            if (fabs(a[i*a_step+j]) > fabs(a[k*a_step+j]))
                k = i;
        }
        // NOTES:
        // the max vaule a(k,j) is near to 0, maybe overflow
        if (fabs(a[k*a_step+j]) < epsilon)
            return false;

        // swap a(k,:) and a(j,:)
        if (k != j) {
            for (int i=0; i < m; ++i)
                swap(a[j*a_step+i], a[k*a_step+i]);
            for (int i=0; i < n; ++i)
                swap(b[j*b_step+i], b[k*b_step+i]); 
        }
        // Now a(j,j) is the max value
        for (k=0; k < m; ++k) {
            if (k != j) {
                // alpha = -a(k,j)/a(j,j)
                // a(k,:) +=  a(j,:)*alpha
                float alpha = -a[k*a_step+j]/a[j*a_step+j];
                for (int i=0; i < m; ++i)
                    a[k*a_step+i] += a[j*a_step+i]*alpha;
                for (int i=0; i < n; ++i)
                    b[k*b_step+i] += b[j*b_step+i]*alpha; 
            }
        }
    }
    // Got the solve
    for (int j=0; j< m; ++j) {
        // b(j,:) = b(j,:)/a(j,j)
        for (int i=0; i < n; ++i)
            b[j*b_step+i] /= a[j*a_step+j];
    }
    return true;
}

/**
 * @brief Solve linear equations ax=b using Gauss elimination 
 * 
 * @param a  mxm matrix, changed in this function
 * @param b  mxn matrix, save the solve x
 * 
 * @return true is if a is a non-singular matrix
 */
bool gauss_solve(float* a, int a_step, float* b, int b_step, int m, int n) {
    const float epsilon = 1e-11;
    for (int j=0; j < m; ++j) {
        // NOTES:
        // a(i,j) is a[i*a_step+j]
        // b(i,j) is b[i*b_step+j]

        // select the max value in a(:,j)
        int k = j;
        for (int i=k+1; i < m; ++i) {
            if (fabs(a[i*a_step+j]) > fabs(a[k*a_step+j]))
                k = i;
        }
        // NOTES:
        // the max vaule a(k,j) is near to 0, maybe overflow
        if (fabs(a[k*a_step+j]) < epsilon)
            return false;

        // swap a(k,:) and a(j,:)
        if (k != j) {
            for (int i=j; i < m; ++i)
                swap(a[j*a_step+i], a[k*a_step+i]);
            for (int i=0; i < n; ++i)
                swap(b[j*b_step+i], b[k*b_step+i]); 
        }
        // Now a(j,j) is the max value
        for (k=j+1; k < m; ++k) {
            // alpha = -a(k,j)/a(j,j)
            // a(k,:) +=  a(j,:)*alpha
            float alpha = -a[k*a_step+j]/a[j*a_step+j];
            for (int i=j; i < m; ++i)
                a[k*a_step+i] += a[j*a_step+i]*alpha;
            for (int i=0; i < n; ++i)
                b[k*b_step+i] += b[j*b_step+i]*alpha; 
        }
    }
    // Got the solve
    for (int j=m-1; j >= 0; --j) {
        for (int k=0; k < n; ++k) {
            for (int i=j+1; i < m; ++i) {
                b[j*b_step+k] -= a[j*a_step+i]*b[i*b_step+k];
            }
            b[j*b_step+k] /= a[j*a_step+j];
        }
    }
    return true;
}

void test_solve_linear_equations() {
    float a[3][3] = {
        {4, -9, 2},
        {2, -4, 6},
        {1, -1, 3}
    };
    float b[3][2] = {
        {5, 3},
        {3, 4},
        {4, 5}
    };
    gauss_solve((float*)a, 3, (float*)b, 2, 3, 2);

    float c[3][3] = {
        {4, -9, 2},
        {2, -4, 6},
        {1, -1, 3}
    };
    float d[3][2] = {
        {5, 3},
        {3, 4},
        {4, 5}
    };
    gauss_jordan_solve((float*)c, 3, (float*)d, 2, 3, 2);
    
    printf("ground_truth_solve: {%.2f,%.2f,%.2f,%.2f,%.2f,%.2f}\n", 6.95, 2.5, -0.15, 7.4, 3.0, 0.2);
    printf("gauss_solve: {%.2f,%.2f,%.2f,%.2f,%.2f,%.2f}\n", b[0][0], b[1][0], b[2][0], b[0][1], b[1][1], b[2][1]);
    printf("gauss_jordan_solve: {%.2f,%.2f,%.2f,%.2f,%.2f,%.2f}\n", d[0][0], d[1][0], d[2][0], d[0][1], d[1][1], d[2][1]);
}

int main()
{
    test_solve_linear_equations();
    return  0;
}