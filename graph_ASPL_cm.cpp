#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <popcntintrin.h>

//#undef __AVX2__

#ifdef __AVX2__
  #include <immintrin.h>
#endif


// Maximum graph size
#define N 10000000

// Column size (multiple of 4 for AVX2)
#define K 24



std::vector<std::vector<int> > G(N);
uint64_t m;
unsigned int row_len;

uint64_t mul(const uint64_t * __restrict__ A, uint64_t * __restrict__ B){
  uint64_t c;
//  c = 0;
  for(std::size_t i = 0; i < G.size(); ++i){
    #ifdef __AVX2__
      __m256i *x;
      const __m256i *y;
      x = reinterpret_cast<__m256i*>(B + i*K);
    #endif
    
    for(std::vector<int>::iterator it = G[i].begin(); it != G[i].end(); ++it){
#ifdef __AVX2__
      y = reinterpret_cast<const __m256i*>(A + (*it)*K);
      for(std::size_t j = 0; j < K/4; ++j){
        __m256i xx = _mm256_load_si256(x+j);
        __m256i yy = _mm256_load_si256(y+j);
        _mm256_store_si256(x+j, _mm256_or_si256(xx, yy));
      }
#else
      for(std::size_t j = 0; j < K; ++j){
        B[i*K+j] |= A[(*it)*K+j];
      }
#endif
/*
      if(*it == G[i].back()){
        for(j = 0; j < row_len; ++j){
          c += _mm_popcnt_u64(B[i*row_len*(bits/64)+j]);
        }
      }
*/
    }
  }
  c = 0;
  for(unsigned int i=0; i<K*m; ++i){
    c += _mm_popcnt_u64(B[i]);
  }
 
  return c;  
}


int main(){
  unsigned int k;
  unsigned int a, b;
  uint64_t *A, *B;
  uint64_t e;
  uint64_t ASPL;

  e = 0;
  m = 0;
  while(std::cin >> a >> b){
//    if(find(G[a].begin(), G[a].end(), b) != G[a].end()) puts("duplicate");
    G[a].push_back(b);
    G[b].push_back(a);
    if(a > m) m = a;
    if(b > m) m = b;
    ++e;
  }

  m++;
  G.resize(m);
//  G.shrink_to_fit();

  row_len = (m+63)/64;

//  std::cout << G.size() << std::endl;

#ifdef __AVX2__
  A = (uint64_t *) _mm_malloc(K*m*sizeof(uint64_t), 32);
  B = (uint64_t *) _mm_malloc(K*m*sizeof(uint64_t), 32);
#else
  A = (uint64_t *) malloc(K*m*sizeof(uint64_t));
  B = (uint64_t *) malloc(K*m*sizeof(uint64_t));
#endif

  if(A==NULL || B==NULL){
    return 1;
  }

  std::cout << G.size() << ", " << (double)2*e/m << std::endl;


//std::cout<<row_len<< std::endl;
//std::cout<<(row_len +K-1)/K<< std::endl;
  int parsize = (row_len +K-1)/K;


  ASPL = m*(m-1);
  k = 0;
  for(unsigned int t=0; t < (row_len+K-1)/K; ++t){
    unsigned int kk, l;
    std::memset(A, 0, K*m*sizeof(uint64_t));
    std::memset(B, 0, K*m*sizeof(uint64_t));
    for(l = 0; l < 64*K && 64*t*K+l < m; ++l){
      A[(64*t*K+l)*K+l/64] |= (0x1ULL<<(l%64));
      B[(64*t*K+l)*K+l/64] = A[(64*t*K+l)*K+l/64];
    }
    for(kk=1; kk <= m; ++kk){
      uint64_t num = mul(A, B);
    
      std::cout << t << " / " << parsize << ": " << kk << " " << num << std::endl;
      std::swap(A, B);

      if(num == m*l) break;
      ASPL += (m*l-num);
    }
    k = std::max(k, kk);
  }

  if(k <= m) std::cout << k << ", " << std::setprecision(32) << static_cast<double>(ASPL)/(m*(m-1)) << std::endl;
  else { std::cout << "disconnected" << std::endl; }

#ifdef __AVX2__
  _mm_free(A);
  _mm_free(B);
#else
  free(A);
  free(B);
#endif
  return 0;
  
}
