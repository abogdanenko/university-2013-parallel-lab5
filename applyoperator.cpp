// for n = 2**m returns m
template <class Integer>
int intlog2(const Integer n)
{
    int result = 0;
    Integer t = n;
    while (t >>= 1)
      result++;
    return result;
}

void ApplyOperator(vector<complexd>& x,
    const vector< vector<complexd> >& U,
    const int k)
{
    const int n = intlog2(x.size());
    const Index mask = 1L << (n - k); // k-th most significant bit
    y.resize(x.size());
    const Index N = x.size();
    
    for (Index i = 0; i < N; i++)
    {
        // bit of i corresponding to k-th qubit ("selected bit")
        const int i_k = i & mask ? 1 : 0; 
        if (i_k == 0)
        {
            const Index i0 = i & ~ mask; // clear selected bit
            const Index i1 = i | mask; // set selected bit

            const complexd a = x[i0];
            const complexd b = x[i1];

            x[i0] = U[0][0] * a + U[0][1] * b;
            x[i1] = U[1][0] * a + U[1][1] * b;
        }
    }
    
}

