// First naive implementation using matrices in row major format
__kernel void GEMM1(const int M, const int N, const int K,
                    const __global float* A,
                    const __global float* B,
                    __global float* C,
                    const __global float* D) {

    // Thread identifiers
    const int m = get_global_id(0); // Row ID of C (0..M)
    const int n = get_global_id(1); // Col ID of C (0..N)

    // Compute a single element (loop over K)
    float acc = 0.0f;
    for (int k=0; k<K; k++) {
        acc += A[m*K + k] * B[k*N + n];
    }

    // Add bias
    acc += D[m];

    // Store the result
    C[m*N + n] = acc;
}
