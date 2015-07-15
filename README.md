# Apply noisy unitary transform to random quantum state

This program performs hadamard transform on each qubit of
n-qubit system, then adds noise and performs the transform again. The
transform is performed a number of times. The number of iterations is
specified by user. Initial state is randomly generated.

Only pure states are considered so system state is represented by a
vector 2**n complex numbers long.

On each iteration scalar product is taken of the two result vectors.
Fidelity is computed as square of absolute value of the scalar product.

See *Parser::PrintUsage* function for invocation information.

This program uses a parallel algorithm to speed up the transform and
allow for larger vectors.

A specific shmem implementation (DISLIB) is used for communication
between processes.
