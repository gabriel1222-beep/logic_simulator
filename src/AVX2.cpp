#include <immintrin.h>
#include <string>
void upperCaseAVX2(std::string &s){
    char* p = s.data(); // gravável em C++17 ou posterior
    std::size_t i = 0;

    const __m256i antes_de_a = _mm256_set1_epi8('a' - 1);
    const __m256i depois_de_z = _mm256_set1_epi8('z' + 1);
    const __m256i bit_da_letra = _mm256_set1_epi8(0x20);

    for (; s.size() - i >= 32; i += 32) {
        __m256i v = _mm256_loadu_si256(
            reinterpret_cast<const __m256i*>(p + i)
        );

        // Cria máscaras para os bytes entre 'a' e 'z'.
        __m256i pelo_menos_a = _mm256_cmpgt_epi8(v, antes_de_a);
        __m256i no_maximo_z = _mm256_cmpgt_epi8(depois_de_z, v);
        __m256i minusculas = _mm256_and_si256(pelo_menos_a, no_maximo_z);

        // Em ASCII, inverter o bit 0x20 muda a-z para A-Z.
        __m256i alterar = _mm256_and_si256(minusculas, bit_da_letra);
        v = _mm256_xor_si256(v, alterar);

        _mm256_storeu_si256(reinterpret_cast<__m256i*>(p + i), v);
    }

    // Restante: de zero a 31 bytes.
    for (; i < s.size(); ++i) {
        if (p[i] >= 'a' && p[i] <= 'z')
            p[i] -= ('a' - 'A');
    }
}
int main(){
  
  return 0;
}
