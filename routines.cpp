#include "routines.h"

using std::conj;

complexd ScalarProduct(const Vector& a, const Vector& b)
{
    complexd sum (0.0, 0.0);
    for (int i = 0; i < a.size(); i++)
    {
        sum += conj(a[i]) * b[i];
    }
    return sum;
}

Matrix MatrixMultiply(const Matrix& A, const Matrix& B)
{
    const int rows_A = A.size();
    const int columns_A = A[0].size();
    const int columns_B = B[0].size();
    const int rows = rows_A;
    const int columns = columns_B;
    const Vector row(columns, complexd(0.0, 0.0));
    Matrix C(rows, row);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            for (int k = 0; k < columns_A; k++)
            {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return C;
}
