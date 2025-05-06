// minimal implementation of ascon-xof and ascon-aead.
// doesn't work currently haha
#include <stdint.h>
#include <string.h>

// References:
//  * https://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-232.ipd.pdf
//  * https://ascon.isec.tugraz.at/specification.html
//  * https://github.com/ascon/ascon-c/blob/main/crypto_hash/asconxof128/opt64/hash.c
//  * https://github.com/ascon/ascon-c/blob/main/crypto_hash/asconxof128/opt64/word.h
//  * https://github.com/ascon/ascon-c/blob/main/crypto_aead_hash/asconaeadxof128/opt64_lowsize/round.h
typedef union {
    uint64_t s[5];
} Ascon;

typedef Ascon AsconXof;

uint8_t RC[12] = { 0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87, 0x78, 0x69, 0x5a, 0x4b };

#define ROR(x, n) (((x) >> (n) | (x) << (-(n) & 63)))

void _round(Ascon *a) {
  uint64_t xtemp;

  for (int i = 0; i < 12; i++)
  {
    /* round constant */
    a->s[2] ^= RC[i];
    /* s-box layer */
    a->s[0] ^= a->s[4];
    a->s[4] ^= a->s[3];
    a->s[2] ^= a->s[1];
    xtemp = a->s[0] & ~a->s[4];
    a->s[0] ^= a->s[2] & ~a->s[1];
    a->s[2] ^= a->s[4] & ~a->s[3];
    a->s[4] ^= a->s[1] & ~a->s[0];
    a->s[1] ^= a->s[3] & ~a->s[2];
    a->s[3] ^= xtemp;
    a->s[1] ^= a->s[0];
    a->s[3] ^= a->s[2];
    a->s[0] ^= a->s[4];
    /* linear layer */
    xtemp = a->s[0] ^ ROR(a->s[0], 28 - 19);
    a->s[0] ^= ROR(xtemp, 19);
    xtemp = a->s[1] ^ ROR(a->s[1], 61 - 39);
    a->s[1] ^= ROR(xtemp, 39);
    xtemp = a->s[2] ^ ROR(a->s[2], 6 - 1);
    a->s[2] ^= ROR(xtemp, 1);
    xtemp = a->s[3] ^ ROR(a->s[3], 17 - 10);
    a->s[3] ^= ROR(xtemp, 10);
    xtemp = a->s[4] ^ ROR(a->s[4], 41 - 7);
    a->s[4] ^= ROR(xtemp, 7);
    a->s[2] = ~a->s[2];
  }
}

uint64_t to64(const uint8_t *input, size_t n)
{
    uint64_t t = 0;
    memcpy(&t, input, n);
    return t;
}

AsconXof xof_init(void)
{
    return (AsconXof){ .s = { 0xda82ce768d9447eb, 0xcc7ce6c75f1ef969,
        0xe7508fd780085631, 0x0ee0ea53416b58cc, 0xe0547524db6f0bde } };
}

void xof_update(AsconXof *xof, const uint8_t *input, size_t input_len)
{
    while (input_len >= 8)
    {
        xof->s[0] ^= to64(input, 8);
        _round(xof);
        input += 8;
        input_len -= 8;
    }
    xof->s[0] ^= to64(input, input_len);
    xof->s[0] ^= 0x01ull << (8 * input_len);
}

void xof_finish(AsconXof *xof, uint8_t *output, size_t output_len)
{
    _round(xof);
    while (output_len > 8)
    {
        memcpy(output, &xof->s[0], 8);
        _round(xof);
        output += 8;
        output_len -= 8;
    }
    memcpy(output, &xof->s[0], output_len);
}

#include <stdio.h>

// asdf = 500f99e1699aa70bb13d4934881acd4e08bb5f9537807a70ec7abb8654e885a9
int main(int argc, char **argv)
{
    if (argc != 2)
        return -1;

    char *input = argv[1];

    AsconXof xof = xof_init();
    xof_update(&xof, (uint8_t *)input, strlen(input));
    
    uint8_t output[32];
    xof_finish(&xof, output, sizeof (output));

    for (size_t i = 0; i < sizeof (output); i++)
        printf("%02x", output[i]);
    printf("\n");
}
