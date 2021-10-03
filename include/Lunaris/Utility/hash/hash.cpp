#include "hash.h"

namespace Lunaris {

    LUNARIS_DECL void _SHA256::transform(const unsigned char* message, unsigned int block_nb)
    {
        uint32 w[64]{};
        uint32 wv[8]{};
        uint32 t1, t2;
        const unsigned char* sub_block;
        for (__int64 i = 0; i < (__int64)block_nb; i++) {
            sub_block = message + (i << 6);
            for (__int64 j = 0; j < 16; j++) {
                SHA2_PACK32(&sub_block[j << 2], &w[j]);
            }
            for (__int64 j = 16; j < 64; j++) {
                w[j] = SHA256_F4(w[j - 2]) + w[j - 7] + SHA256_F3(w[j - 15]) + w[j - 16];
            }
            for (__int64 j = 0; j < 8; j++) {
                wv[j] = m_h[j];
            }
            for (__int64 j = 0; j < 64; j++) {
                t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6])
                    + sha256_k[j] + w[j];
                t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
                wv[7] = wv[6];
                wv[6] = wv[5];
                wv[5] = wv[4];
                wv[4] = wv[3] + t1;
                wv[3] = wv[2];
                wv[2] = wv[1];
                wv[1] = wv[0];
                wv[0] = t1 + t2;
            }
            for (__int64 j = 0; j < 8; j++) {
                m_h[j] += wv[j];
            }
        }
    }

    LUNARIS_DECL void _SHA256::init()
    {
        m_h[0] = 0x6a09e667;
        m_h[1] = 0xbb67ae85;
        m_h[2] = 0x3c6ef372;
        m_h[3] = 0xa54ff53a;
        m_h[4] = 0x510e527f;
        m_h[5] = 0x9b05688c;
        m_h[6] = 0x1f83d9ab;
        m_h[7] = 0x5be0cd19;
        m_len = 0;
        m_tot_len = 0;
    }

    LUNARIS_DECL void _SHA256::update(const unsigned char* message, unsigned int len)
    {
        unsigned int block_nb;
        unsigned int new_len, rem_len, tmp_len;
        const unsigned char* shifted_message;
        tmp_len = SHA224_256_BLOCK_SIZE - m_len;
        rem_len = len < tmp_len ? len : tmp_len;
        memcpy(&m_block[m_len], message, rem_len);
        if (m_len + len < SHA224_256_BLOCK_SIZE) {
            m_len += len;
            return;
        }
        new_len = len - rem_len;
        block_nb = new_len / SHA224_256_BLOCK_SIZE;
        shifted_message = message + rem_len;
        transform(m_block, 1);
        transform(shifted_message, block_nb);
        rem_len = new_len % SHA224_256_BLOCK_SIZE;
        memcpy(m_block, &shifted_message[block_nb << 6], rem_len);
        m_len = rem_len;
        m_tot_len += (block_nb + 1) << 6;
    }

    LUNARIS_DECL void _SHA256::final(unsigned char* digest)
    {
        unsigned int block_nb;
        unsigned int pm_len;
        unsigned int len_b;
        int i;
        block_nb = (1 + ((SHA224_256_BLOCK_SIZE - 9)
            < (m_len % SHA224_256_BLOCK_SIZE)));
        len_b = (m_tot_len + m_len) << 3;
        pm_len = block_nb << 6;
        memset(m_block + m_len, 0, static_cast<size_t>(pm_len) - static_cast<size_t>(m_len));
        m_block[m_len] = 0x80;
        SHA2_UNPACK32(len_b, m_block + pm_len - 4);
        transform(m_block, block_nb);
        for (i = 0; i < 8; i++) {
            SHA2_UNPACK32(m_h[i], &digest[i << 2]);
        }
    }

    LUNARIS_DECL std::string sha256(const std::string& input)
    {
        unsigned char digest[_SHA256::DIGEST_SIZE];
        memset(digest, 0, _SHA256::DIGEST_SIZE);

        std::basic_string<unsigned char> conv;
        for (auto& i : input) conv += static_cast<unsigned char>(i);

        _SHA256 ctx = _SHA256();
        ctx.init();
        ctx.update(conv.c_str(), static_cast<unsigned int>(conv.length()));
        ctx.final(digest);

        char buf[2 * _SHA256::DIGEST_SIZE + 1]{};
        buf[2 * _SHA256::DIGEST_SIZE] = 0;
        for (int i = 0; i < _SHA256::DIGEST_SIZE; i++)
            sprintf_s(buf + (long long)i * 2, 2 * _SHA256::DIGEST_SIZE + 1 - ((long long)i * 2), "%02x", digest[i]);
        return std::string(buf);
    }

    LUNARIS_DECL std::string sha256(const std::vector<char>& input)
    {
        unsigned char digest[_SHA256::DIGEST_SIZE];
        memset(digest, 0, _SHA256::DIGEST_SIZE);

        std::basic_string<unsigned char> conv;
        for (auto& i : input) conv += static_cast<unsigned char>(i);

        _SHA256 ctx = _SHA256();
        ctx.init();
        ctx.update(conv.c_str(), static_cast<unsigned int>(conv.length()));
        ctx.final(digest);

        char buf[2 * _SHA256::DIGEST_SIZE + 1]{};
        buf[2 * _SHA256::DIGEST_SIZE] = 0;
        for (int i = 0; i < _SHA256::DIGEST_SIZE; i++)
            sprintf_s(buf + (long long)i * 2, 2 * _SHA256::DIGEST_SIZE + 1 - ((long long)i * 2), "%02x", digest[i]);
        return std::string(buf);
    }

    LUNARIS_DECL std::string sha256_f(const std::string& fpath)
    {
        FILE* fp = nullptr;
        if (fopen_s(&fp, fpath.c_str(), "rb") != 0) return ""; // failed
        std::string myself;
        for (char ubuf; fread_s(&ubuf, 1, sizeof(char), 1, fp);) myself += ubuf;
        fclose(fp);
        return sha256(myself);

    }
}