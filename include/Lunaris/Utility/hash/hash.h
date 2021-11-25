#pragma once

#include <Lunaris/__macro/macros.h>
#include <Lunaris/Utility/random.h>

#include <stdio.h>
#include <string>
#include <vector>

// This was not made by me (the SHA256 part)
// Source: http://www.zedwood.com/article/cpp-sha256-function (modified)

#define SHA2_SHFR(x, n)    (x >> n)
#define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA2_UNPACK32(x, str)                 \
{                                             \
    *((str) + 3) = (uint8) ((x)      );       \
    *((str) + 2) = (uint8) ((x) >>  8);       \
    *((str) + 1) = (uint8) ((x) >> 16);       \
    *((str) + 0) = (uint8) ((x) >> 24);       \
}
#define SHA2_PACK32(str, x)                   \
{                                             \
    *(x) =   ((uint32) *((str) + 3)      )    \
           | ((uint32) *((str) + 2) <<  8)    \
           | ((uint32) *((str) + 1) << 16)    \
           | ((uint32) *((str) + 0) << 24);   \
}

namespace Lunaris {

    const unsigned int sha256_k[64] = //UL = uint32
    { 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
     0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
     0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
     0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
     0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
     0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
     0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
     0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
     0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
     0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
     0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
     0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
     0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
     0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
     0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
     0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };
    constexpr unsigned int SHA224_256_BLOCK_SIZE = (512 / 8);

    class _SHA256
    {
    protected:
        typedef unsigned char uint8;
        typedef unsigned int uint32;
        typedef unsigned long long uint64;
    public:
        void init();
        void update(const unsigned char* message, unsigned int len);
        void final(unsigned char* digest);
        static const unsigned int DIGEST_SIZE = (256 / 8);

    protected:
        void transform(const unsigned char* message, unsigned int block_nb);
        unsigned int m_tot_len;
        unsigned int m_len;
        unsigned char m_block[2 * SHA224_256_BLOCK_SIZE];
        uint32 m_h[8];
    };

    /// <summary>
    /// <para>Gets the string and generates a SHA256 code based on it.</para>
    /// </summary>
    /// <param name="{std::string}">The data itself.</param>
    /// <returns>{std::string} The hash.</returns>
    std::string sha256(const std::string&);

    /// <summary>
    /// <para>Gets the vector of char and generates a SHA256 code based on it.</para>
    /// </summary>
    /// <param name="{std::vector}">The data itself.</param>
    /// <returns>{std::string} The hash.</returns>
    std::string sha256(const std::vector<char>&);

    /// <summary>
    /// <para>Generates a SHA256 based on content in a file.</para>
    /// </summary>
    /// <param name="{std::string}">The path to the file.</param>
    /// <returns>{std::string} The hash.</returns>
    std::string sha256_f(const std::string&);

    /// <summary>
    /// <para>Very simple algorithm (better than nothing, fast) that adds a value to every character, every time adding itself too.</para>
    /// </summary>
    /// <param name="{std::string}">The original string.</param>
    /// <param name="{unsigned char}">The value offset each step. (recommended: something bigger than 16, and not a multiple of 2).</param>
    /// <returns>{std::string} Encrypted string (using this very simple algorithm)</returns>
    std::string encrypt_one_sum_each(std::string, const unsigned char);

    /// <summary>
    /// <para>Decrypt the algorithm encrypted using one_sum_each.</para>
    /// </summary>
    /// <param name="{std::string}">The encrypted string.</param>
    /// <param name="{unsigned char}">The secret value offset.</param>
    /// <returns>{std::string} Decrypted string, if things were right.</returns>
    std::string decrypt_one_sum_each(std::string, const unsigned char);

    /// <summary>
    /// <para>Very simple algorithm (better than nothing, slower than one_sum_each, but more complex) that moves the bits around each byte.</para>
    /// </summary>
    /// <param name="{std::string}">The original string.</param>
    /// <param name="{unsigned char}">Special sauce (how many bits to move and sum every byte).</param>
    /// <returns>{std::string} Encrypted string.</returns>
    std::string encrypt_move_bytes(std::string, const unsigned);

    /// <summary>
    /// <para>Decrypt a encrypted string using move_bytes.</para>
    /// </summary>
    /// <param name="{std::string}">Encrypted string.</param>
    /// <param name="{unsigned char}">The sauce used to encrypt.</param>
    /// <returns>{std::string} Hopefully the string you have ever wanted.</returns>
    std::string decrypt_move_bytes(std::string, const unsigned);

    /// <summary>
    /// <para>Move characters around in number times that argument, really funky mess algorithm, of course not secure, but maybe good if combined with others.</para>
    /// </summary>
    /// <param name="{std::string}">The original string.</param>
    /// <param name="{unsigned char}">The secret value offset (it moves stuff using this number).</param>
    /// <returns>{std::string} Encrypted string (more like randomized with seed string).</returns>
    std::string encrypt_mess_string_order(std::string, const size_t);

    /// <summary>
    /// <para>Decrypts a messy string.</para>
    /// </summary>
    /// <param name="{std::string}">The encrypted string.</param>
    /// <param name="{unsigned char}">The seed (used to mess around).</param>
    /// <returns>{std::string} This should be your final string.</returns>
    std::string decrypt_mess_string_order(std::string, const size_t);

    /// <summary>
    /// <para>This does the combo one_sum_each, move_bytes and mess_string_order in a random way, but keeping the key somewhere into it (randomly positioned).</para>
    /// <para>Maybe this is good enough for *some* security. Probably not worth it, but different enough.</para>
    /// </summary>
    /// <param name="{std::string}">The original string.</param>
    /// <returns>{std::string} Messy string.</returns>
    std::string encrypt_supermess_auto(std::string);

    /// <summary>
    /// <para>Decrypts a supermessy string.</para>
    /// </summary>
    /// <param name="{std::string}">A supermessy string.</param>
    /// <returns>{std::string} Hopefully your string.</returns>
    std::string decrypt_supermess_auto(std::string);

    /// <summary>
    /// <para>Very simple algorithm (better than nothing, fast) that adds a value to every character, every time adding itself too.</para>
    /// </summary>
    /// <param name="{std::vector<char>}">The original string.</param>
    /// <param name="{unsigned char}">The value offset each step. (recommended: something bigger than 16, and not a multiple of 2).</param>
    /// <returns>{std::vector<char>} Encrypted string (using this very simple algorithm)</returns>
    std::vector<char> encrypt_one_sum_each(std::vector<char>, const unsigned char);

    /// <summary>
    /// <para>Decrypt the algorithm encrypted using one_sum_each.</para>
    /// </summary>
    /// <param name="{std::vector<char>}">The encrypted string.</param>
    /// <param name="{unsigned char}">The secret value offset.</param>
    /// <returns>{std::vector<char>} Decrypted string, if things were right.</returns>
    std::vector<char> decrypt_one_sum_each(std::vector<char>, const unsigned char);

    /// <summary>
    /// <para>Very simple algorithm (better than nothing, slower than one_sum_each, but more complex) that moves the bits around each byte.</para>
    /// </summary>
    /// <param name="{std::vector<char>}">The original string.</param>
    /// <param name="{unsigned char}">Special sauce (how many bits to move and sum every byte).</param>
    /// <returns>{std::vector<char>} Encrypted string.</returns>
    std::vector<char> encrypt_move_bytes(std::vector<char>, const unsigned);

    /// <summary>
    /// <para>Decrypt a encrypted string using move_bytes.</para>
    /// </summary>
    /// <param name="{std::vector<char>}">Encrypted string.</param>
    /// <param name="{unsigned char}">The sauce used to encrypt.</param>
    /// <returns>{std::vector<char>} Hopefully the string you have ever wanted.</returns>
    std::vector<char> decrypt_move_bytes(std::vector<char>, const unsigned);

    /// <summary>
    /// <para>Move characters around in number times that argument, really funky mess algorithm, of course not secure, but maybe good if combined with others.</para>
    /// </summary>
    /// <param name="{std::vector<char>}">The original string.</param>
    /// <param name="{unsigned char}">The secret value offset (it moves stuff using this number).</param>
    /// <returns>{std::vector<char>} Encrypted string (more like randomized with seed string).</returns>
    std::vector<char> encrypt_mess_string_order(std::vector<char>, const size_t);

    /// <summary>
    /// <para>Decrypts a messy string.</para>
    /// </summary>
    /// <param name="{std::vector<char>}">The encrypted string.</param>
    /// <param name="{unsigned char}">The seed (used to mess around).</param>
    /// <returns>{std::vector<char>} This should be your final string.</returns>
    std::vector<char> decrypt_mess_string_order(std::vector<char>, const size_t);

    /// <summary>
    /// <para>This does the combo one_sum_each, move_bytes and mess_string_order in a random way, but keeping the key somewhere into it (randomly positioned).</para>
    /// <para>Maybe this is good enough for *some* security. Probably not worth it, but different enough.</para>
    /// </summary>
    /// <param name="{std::vector<char>}">The original string.</param>
    /// <returns>{std::vector<char>} Messy string.</returns>
    std::vector<char> encrypt_supermess_auto(std::vector<char>);

    /// <summary>
    /// <para>Decrypts a supermessy string.</para>
    /// </summary>
    /// <param name="{std::vector<char>}">A supermessy string.</param>
    /// <returns>{std::vector<char>} Hopefully your string.</returns>
    std::vector<char> decrypt_supermess_auto(std::vector<char>);
}