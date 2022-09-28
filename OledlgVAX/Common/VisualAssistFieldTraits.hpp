#pragma once
#include <stddef.h>
#include <stdint.h>
#include <intrin.h>
#include <memory.h>
#include <vector>
#include <stdexcept>

// GF(2^113) with respect to type 2 ONB 
struct VisualAssistFieldTraits {

    using ElementType = __m128i;
    using TraceType = size_t;

    static constexpr size_t BinaryBitSizeValue = 113;
    static constexpr size_t BinaryByteSizeValue = (BinaryBitSizeValue + 7) / 8;

    static void Verify(const ElementType& Element) {
        bool Valid = _mm_movemask_epi8(
            _mm_cmpeq_epi32(
                _mm_and_si128(
                    Element, 
                    _mm_set_epi32(0xfffe0000, 0, 0, 0)
                ),
                _mm_setzero_si128()
            )
        ) == 0xFFFF;

        if (Valid == false) {
            throw std::invalid_argument("Element is not in GF(2 ^ 113).");
        }
    }

    [[nodiscard]]
    static size_t Serialize(const ElementType& Element, void* lpBinary, size_t cbBinary) {
        if (cbBinary < BinaryByteSizeValue) {
            throw std::length_error("Insufficient buffer.");
        } else {
            memcpy(lpBinary, &Element, BinaryByteSizeValue);
            return BinaryByteSizeValue;
        }
    }

    [[nodiscard]]
    static std::vector<uint8_t> Serialize(const ElementType& Element) noexcept {
        return std::vector<uint8_t>(
            reinterpret_cast<const uint8_t*>(&Element),
            reinterpret_cast<const uint8_t*>(&Element) + BinaryByteSizeValue
        );
    }

    static void Deserialize(ElementType& Element, const void* lpSerializedBytes, size_t cbSerializedBytes) {
        if (cbSerializedBytes != BinaryByteSizeValue) {
            throw std::length_error("The length of buffer is not correct.");
        } else {
            ElementType t;

            t = _mm_setzero_si128();
            memcpy(&t, lpSerializedBytes, BinaryByteSizeValue);
            Verify(t);

            Element = t;
        }
    }

    static inline void SetZero(ElementType& Element) noexcept {
        Element = _mm_setzero_si128();
    }

    static inline void SetOne(ElementType& Element) noexcept {
        Element = _mm_set_epi32(0x0001ffff, 0xffffffff, 0xffffffff, 0xffffffff);
    }

    static inline bool IsEqual(const ElementType& A, const ElementType& B) noexcept {
        return _mm_movemask_epi8(
            _mm_cmpeq_epi32(A, B)
        ) == 0xffff;
    }

    static inline bool IsZero(const ElementType& Element) noexcept {
        return _mm_movemask_epi8(
            _mm_cmpeq_epi32(
                Element,
                _mm_setzero_si128()
            )
        ) == 0xffff;
    }

    static inline bool IsOne(const ElementType& Element) noexcept {
        return _mm_movemask_epi8(
            _mm_cmpeq_epi32(
                Element,
                _mm_set_epi32(0x0001ffff, 0xffffffff, 0xffffffff, 0xffffffff)
            )
        ) == 0xffff;
    }

    // Result = -A
    static inline void Negative(ElementType& Result, const ElementType& A) {
        Result = A;
    }

    // Result = A + B
    static inline void Add(ElementType& Result, const ElementType& A, const ElementType& B) noexcept {
        Result = _mm_xor_si128(A, B);
    }

    // A += B
    static inline void AddAssign(ElementType& A, const ElementType& B) noexcept {
        A = _mm_xor_si128(A, B);
    }

    // Result = A + 1
    static inline void AddOne(ElementType& Result, const ElementType& A) noexcept {
        Result = _mm_xor_si128(
            A, 
            _mm_set_epi32(0x0001ffff, 0xffffffff, 0xffffffff, 0xffffffff)
        );
    }

    // A += 1
    static inline void AddOneAssign(ElementType& A) noexcept {
        A = _mm_xor_si128(
            A, 
            _mm_set_epi32(0x0001ffff, 0xffffffff, 0xffffffff, 0xffffffff)
        );
    }

    // Result = A - B
    static inline void Substract(ElementType& Result, const ElementType& A, const ElementType& B) noexcept {
        Result = _mm_xor_si128(A, B);
    }

    // A -= B
    static inline void SubstractAssign(ElementType& A, const ElementType& B) noexcept {
        A = _mm_xor_si128(A, B);
    }

    // Result = A - 1
    static inline void SubstractOne(ElementType& Result, const ElementType& A) noexcept {
        Result = _mm_xor_si128(
            A, 
            _mm_set_epi32(0x0001ffff, 0xffffffff, 0xffffffff, 0xffffffff)
        );
    }

    // A -= 1
    static inline void SubstractOneAssign(ElementType& A) noexcept {
        A = _mm_xor_si128(
            A, 
            _mm_set_epi32(0x0001ffff, 0xffffffff, 0xffffffff, 0xffffffff)
        );
    }

    /*
Generated by the following sage script:

#!/usr/bin/env sage

m = 113                 # GF(2 ^ 113)
T = 2                   # Type 2 ONB
P = T * m + 1
assert(P.is_prime())

F2pTm.<b> = GF(2 ^ (T * m))
r = F2pTm.primitive_element() ^ ((2 ^ (T * m) - 1) // P)
l = GF(P).primitive_element() ^ ((P - 1) // 2)
beta = sum([ r ^ (l ^ i) for i in range(T) ])
NormalBasis = [ beta ^ (2 ^ i) for i in range(m) ]

NormalBasisToF2pTmMatrix = MatrixSpace(GF(2), m, T * m)([
    F2pTm.vector_space()(n) for n in NormalBasis
]).transpose()

MultiplicationTable = MatrixSpace(GF(2), m, m)()
for i in range(m):
    for j in range(m):
        ab = NormalBasis[i] * NormalBasis[j]
        tt = NormalBasisToF2pTmMatrix.solve_right(F2pTm.vector_space()(ab))
        MultiplicationTable[i, j] = tt[0]
    print '%d/%d' % (i, m)

T0 = []
T1 = []
for i in range(m):
    ts = []
    for j in range(m):
        if MultiplicationTable[i, j] == 1:
            ts.append(j)
    assert(len(ts) == 1 or len(ts) == 2)
    if len(ts) == 1:
        T0.append(ts[0])
        T1.append(0)
    else:
        T0.append(ts[0])
        T1.append(ts[1])

T0 = [ '0x%.2x' % i for i in T0 ]
T1 = [ '0x%.2x' % i for i in T1 ]

print 'static inline constexpr uint8_t T0[] = {'
print '    %s' % ', '.join(T0)
print '};'
print 'static inline constexpr uint8_t T1[] = {'
print '    %s' % ', '.join(T1)
print '};'

        */

    static  constexpr uint8_t T0[] = {
        0x01, 0x00, 0x0b, 0x29, 0x39, 0x4a, 0x14, 0x18, 0x2b, 0x5a, 0x28, 0x02, 0x1c, 0x41, 0x12, 0x47,
        0x4b, 0x24, 0x0e, 0x47, 0x06, 0x18, 0x31, 0x20, 0x07, 0x2b, 0x27, 0x3c, 0x0c, 0x22, 0x25, 0x36,
        0x17, 0x15, 0x1d, 0x67, 0x11, 0x1e, 0x36, 0x1a, 0x0a, 0x03, 0x39, 0x08, 0x27, 0x0a, 0x01, 0x0b,
        0x1c, 0x16, 0x52, 0x49, 0x5e, 0x50, 0x1f, 0x1a, 0x3c, 0x04, 0x08, 0x5a, 0x1b, 0x2a, 0x16, 0x20,
        0x56, 0x0d, 0x4d, 0x44, 0x43, 0x43, 0x4e, 0x0f, 0x06, 0x33, 0x05, 0x10, 0x33, 0x42, 0x44, 0x0f,
        0x35, 0x1f, 0x17, 0x07, 0x05, 0x14, 0x21, 0x0d, 0x12, 0x60, 0x09, 0x1b, 0x03, 0x41, 0x34, 0x46,
        0x13, 0x3b, 0x38, 0x04, 0x34, 0x10, 0x24, 0x1e, 0x51, 0x23, 0x11, 0x0e, 0x45, 0x35, 0x26, 0x09,
        0x2d
    };

    static  constexpr uint8_t T1[] = {
        0x00, 0x2e, 0x2e, 0x5c, 0x63, 0x54, 0x48, 0x53, 0x3a, 0x6f, 0x2d, 0x2f, 0x5c, 0x57, 0x6b, 0x4f,
        0x65, 0x6a, 0x58, 0x60, 0x55, 0x21, 0x3e, 0x52, 0x15, 0x3e, 0x37, 0x5b, 0x30, 0x57, 0x67, 0x51,
        0x3f, 0x56, 0x31, 0x69, 0x66, 0x58, 0x6e, 0x2c, 0x5b, 0x2f, 0x3d, 0x19, 0x6f, 0x70, 0x02, 0x29,
        0x3d, 0x22, 0x69, 0x4c, 0x64, 0x6d, 0x26, 0x3f, 0x62, 0x2a, 0x54, 0x61, 0x38, 0x30, 0x19, 0x37,
        0x62, 0x5d, 0x6b, 0x45, 0x4e, 0x6c, 0x5f, 0x13, 0x4b, 0x53, 0x64, 0x48, 0x6a, 0x5e, 0x46, 0x6c,
        0x65, 0x68, 0x32, 0x49, 0x3a, 0x61, 0x40, 0x1d, 0x25, 0x6e, 0x3b, 0x28, 0x0c, 0x63, 0x4d, 0x6d,
        0x59, 0x55, 0x40, 0x5d, 0x4a, 0x50, 0x68, 0x23, 0x66, 0x32, 0x4c, 0x42, 0x4f, 0x5f, 0x59, 0x2c,
        0x70
    };

    static inline void RotateShiftLeftByOne(ElementType& Result, const ElementType& A) noexcept {
        __m128i ShiftOut = _mm_shuffle_epi32(
            _mm_and_si128(
                A, 
                _mm_set_epi32(0x10000, 0x80000000, 0x80000000, 0x80000000)
            ),
            _MM_SHUFFLE(2, 1, 0, 3)
        );
        __m128i ShiftOutH = _mm_srli_epi32(
            ShiftOut,
            31
        );
        __m128i ShiftOutL = _mm_and_si128(
            _mm_srli_epi32(ShiftOut, 16),
            _mm_set_epi32(0, 0, 0, 0xffffffff)
        );

        Result = _mm_xor_si128(
            _mm_xor_si128(
                _mm_slli_epi32(A, 1), 
                ShiftOutL
            ),
            ShiftOutH
        );

        Result = _mm_and_si128(
            Result, 
            _mm_set_epi32(0x0001ffff, 0xffffffff, 0xffffffff, 0xffffffff)
        );
    }

    static inline void RotateShiftLeftByOneAssign(ElementType& A) noexcept {
        __m128i ShiftOut = _mm_shuffle_epi32(
            _mm_and_si128(
                A, 
                _mm_set_epi32(0x10000, 0x80000000, 0x80000000, 0x80000000)
            ),
            _MM_SHUFFLE(2, 1, 0, 3)
        );
        __m128i ShiftOutH = _mm_srli_epi32(
            ShiftOut,
            31
        );
        __m128i ShiftOutL = _mm_and_si128(
            _mm_srli_epi32(ShiftOut, 16),
            _mm_set_epi32(0, 0, 0, 0xffffffff)
        );

        A = _mm_xor_si128(
            _mm_xor_si128(
                _mm_slli_epi32(A, 1),
                ShiftOutL
            ),
            ShiftOutH
        );

        A = _mm_and_si128(A, _mm_set_epi32(0x0001ffff, 0xffffffff, 0xffffffff, 0xffffffff));
    }

    static inline void RotateShiftRightByOne(ElementType& Result, const ElementType& A) noexcept {
        __m128i ShiftOut = _mm_shuffle_epi32(
            _mm_and_si128(
                A, 
                _mm_set_epi32(1, 1, 1, 1)
            ),
            _MM_SHUFFLE(0, 3, 2, 1)
        );
        __m128i ShiftOutH = _mm_slli_epi32(
            _mm_and_si128(
                ShiftOut, 
                _mm_set_epi32(0xffffffff, 0, 0, 0)
            ),
            16
        );
        __m128i ShiftOutL = _mm_slli_epi32(
            _mm_and_si128(
                ShiftOut, 
                _mm_set_epi32(0, 0xffffffff, 0xffffffff, 0xffffffff)
            ),
            31
        );

        Result = _mm_xor_si128(
            _mm_xor_si128(
                _mm_srli_epi32(A, 1),
                ShiftOutL
            ),
            ShiftOutH
        );
    }

    static inline void RotateShiftRightByOneAssign(ElementType& A) noexcept {
        __m128i ShiftOut = _mm_shuffle_epi32(
            _mm_and_si128(
                A, 
                _mm_set_epi32(1, 1, 1, 1)
            ),
            _MM_SHUFFLE(0, 3, 2, 1)
        );
        __m128i ShiftOutH = _mm_slli_epi32(
            _mm_and_si128(
                ShiftOut, 
                _mm_set_epi32(0xffffffff, 0, 0, 0)
            ),
            16
        );
        __m128i ShiftOutL = _mm_slli_epi32(
            _mm_and_si128(
                ShiftOut, 
                _mm_set_epi32(0, 0xffffffff, 0xffffffff, 0xffffffff)
            ),
            31
        );

        A = _mm_xor_si128(
            _mm_xor_si128(
                _mm_srli_epi32(A, 1),
                ShiftOutL
            ),
            ShiftOutH
        );
    }

    // Result = A * B
    // https://www.princeton.edu/~rblee/ELE572Papers/Fall04Readings/NingYin-FiniteFieldMul.pdf
    static inline void Multiply(ElementType& Result, const ElementType& A, const ElementType& B) noexcept {
        ElementType MatrixB[BinaryBitSizeValue];

        MatrixB[0] = B;
        for (size_t i = 1; i < BinaryBitSizeValue; ++i) {
            RotateShiftRightByOne(MatrixB[i], MatrixB[i - 1]);
        }

        Result = _mm_and_si128(A, MatrixB[T0[0]]);

        ElementType Ak = A;
        for (size_t i = 1; i < BinaryBitSizeValue; ++i) {
            RotateShiftRightByOneAssign(Ak);
            Result = _mm_xor_si128(
                Result,
                _mm_and_si128(
                    Ak,
                    _mm_xor_si128(MatrixB[T0[i]], MatrixB[T1[i]])
                )
            );
        }
    }

    // A *= B
    // https://www.princeton.edu/~rblee/ELE572Papers/Fall04Readings/NingYin-FiniteFieldMul.pdf
    static inline void MultiplyAssign(ElementType& A, const ElementType& B) noexcept {
        ElementType MatrixB[BinaryBitSizeValue];

        MatrixB[0] = B;
        for (size_t i = 1; i < BinaryBitSizeValue; ++i) {
            RotateShiftRightByOne(MatrixB[i], MatrixB[i - 1]);
        }

        ElementType Ak = A;

        A = _mm_and_si128(A, MatrixB[T0[0]]);

        for (size_t i = 1; i < BinaryBitSizeValue; ++i) {
            RotateShiftRightByOneAssign(Ak);
            A = _mm_xor_si128(
                A,
                _mm_and_si128(
                    Ak,
                    _mm_xor_si128(MatrixB[T0[i]], MatrixB[T1[i]])
                )
            );
        }
    }

    static inline void Divide(ElementType& Result, const ElementType& A, const ElementType& B) {
        ElementType InverseOfB;
        Inverse(InverseOfB, B);
        Multiply(Result, A, InverseOfB);
    }

    static inline void DivideAssign(ElementType& A, const ElementType& B) {
        ElementType InverseOfB;
        Inverse(InverseOfB, B);
        MultiplyAssign(A, InverseOfB);
    }

    // Result = A ^ -1
    // IEEE P1363/D9. Standard Specifications for Public Key Cryptography
    // Annex A (informative)
    // Number-Theoretic Background.
    // Page. 100
    static inline void Inverse(ElementType& Result, const ElementType& A) {
        const uint8_t bs[] = { 0, 0, 0, 0, 1, 1, 1 };   // bits of (113 - 1)
        ElementType eta = A;
        size_t k = 1;

        for (int i = 6 - 1; i >= 0; --i) {
            ElementType mu = eta;

            for (size_t j = 1; j <= k; ++j) {
                SquareAssign(mu);
            }

            MultiplyAssign(eta, mu);    // original document has a typo, it should be "eta <- mu * eta"
            k *= 2;

            if (bs[i]) {
                SquareAssign(eta);
                MultiplyAssign(eta, A);
                ++k;
            }
        }

        Square(Result, eta);
    }

    // A = A ^ -1
    static inline void InverseAssign(ElementType& A) {
        Inverse(A, A);
    }

    // Result = A ^ 2
    static inline void Square(ElementType& Result, const ElementType& A) noexcept {
        RotateShiftLeftByOne(Result, A);
    }

    // A = A ^ 2
    static inline void SquareAssign(ElementType& A) noexcept {
        RotateShiftLeftByOneAssign(A);
    }

    // Result = sqrt(A)
    static inline void SquareRoot(ElementType& Result, const ElementType& A) noexcept {
        RotateShiftRightByOne(Result, A);
    }

    // A = sqrt(A)
    static inline void SquareRootAssign(ElementType& A) noexcept {
        RotateShiftRightByOneAssign(A);
    }

    // Result = tr(A)
    static inline void Trace(TraceType& Result, const ElementType& A) {
        __m128i v = _mm_sub_epi32(
            A,
            _mm_and_si128(
                _mm_srli_epi32(A, 1), 
                _mm_set1_epi32(0x55555555)
            )
        );

        v = _mm_add_epi32(
            _mm_and_si128(
                v, 
                _mm_set1_epi32(0x33333333)
            ),
            _mm_and_si128(
                _mm_srli_epi32(v, 2), 
                _mm_set1_epi32(0x33333333)
            )
        );

        v = _mm_and_si128(
            _mm_add_epi32(
                v, 
                _mm_srli_epi32(v, 4)
            ),
            _mm_set1_epi32(0x0f0f0f0f)
        );

        __m128i l = _mm_and_si128(
            _mm_mul_epu32(
                v, 
                _mm_set1_epi32(0x01010101)
            ),
            _mm_set_epi32(0, 0xffffffff, 0, 0xffffffff)
        );
        __m128i h = _mm_shuffle_epi32(
            _mm_and_si128(
                _mm_mul_epu32(
                    _mm_shuffle_epi32(v, _MM_SHUFFLE(2, 3, 0, 1)), 
                    _mm_set1_epi32(0x01010101)
                ),
                _mm_set_epi32(0, 0xffffffff, 0, 0xffffffff)
            ),
            _MM_SHUFFLE(2, 3, 0, 1)
        );

        __m128i c = _mm_srli_epi32(_mm_xor_si128(l, h), 24);

        Result = (
            _mm_extract_epi16(c, 0) + 
            _mm_extract_epi16(c, 2) + 
            _mm_extract_epi16(c, 4) + 
            _mm_extract_epi16(c, 6)
        ) % 2u;
    }

    // Find a `z` which satisfies `z^2 + z = Beta`
    [[nodiscard]]
    static inline bool SolveQuadratic(ElementType& Element, const ElementType& Beta) {
        TraceType tr;
        Trace(tr, Beta);

        if (tr == 1) {
            return false;
        } else {
            uint32_t root[4] = {};
            uint32_t beta[4] = {};
            static_cast<void>(
                Serialize(Beta, beta, sizeof(beta))
            );

            uint32_t* proot = root;
            uint32_t* pbeta = beta;
            uint32_t mask = 0x2;
            for (size_t i = 1; i < 113; ++i) {
                if (mask != 1) {
                    proot[0] |= ((proot[0] & (mask >> 1)) << 1) ^ (pbeta[0] & mask);
                } else {
                    proot[0] |= _rotl((proot[-1] & _rotr(mask, 1)), 1) ^ (pbeta[0] & mask);
                }

                mask <<= 1;
                if (mask == 0) {
                    mask = 1;
                    ++proot;
                    ++pbeta;
                }
            }

            Element = _mm_set_epi32(root[3], root[2], root[1], root[0]);

            return true;
        }
    }

    // Find root `x`s which satisfies `A * x ^ 2 + B * x + C = 0`
    [[nodiscard]]
    static inline std::vector<ElementType> SolveQuadratic(const ElementType& A, const ElementType& B, const ElementType& C) {
        if (IsZero(A)) {
            throw std::invalid_argument("A cannot be zero.");
        }

        if (IsZero(B)) {
            // A * x ^ 2 + C = 0
            //  x = sqrt(C / A)
            ElementType Root;

            Divide(Root, C, A);
            SquareRootAssign(Root);

            return std::vector<ElementType>{ Root };
        } else {
            ElementType beta;
            ElementType x1;
            ElementType x2;

            Inverse(beta, B);           // beta = 1 / B
            SquareAssign(beta);         // beta = 1 / B^2
            MultiplyAssign(beta, A);    // beta = A / B^2
            MultiplyAssign(beta, C);    // beta = A * C / B^2
            
            if (SolveQuadratic(x1, beta)) {
                AddOne(x2, x1);

                MultiplyAssign(x1, B);
                DivideAssign(x1, A);
                MultiplyAssign(x2, B);
                DivideAssign(x2, A);

                return std::vector<ElementType>{x1, x2};
            } else {
                return std::vector<ElementType>();
            }
        }
    }
};

