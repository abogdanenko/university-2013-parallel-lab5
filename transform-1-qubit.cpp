/* 
    Name: transform-1-qubit
    Author: Alexey Bogdanenko
    Contact: alex01@vpsbox.ru
    Date: Sep 2013
    Description: This program performs a unitary transform on one qubit of
        n-qubit system. Only pure states are considered so system state is
        represented by a vector 2**n complex numbers long.

*/

int main()
{
    psi; // state vector
    k; // target qubit index
    U; // transform matrix
    Transform1Qubit(psi, k, transform_matrix);
    return 0;
}
