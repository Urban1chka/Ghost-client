//
//#pragma once
//#include <string>
//#include <array>
//#include <utility>
//#include <cstdarg>
//
//namespace xorstr_impl {
//
//#ifdef _MSC_VER
//#define XORSTR_INLINE __forceinline
//#else
//#define XORSTR_INLINE inline
//#endif
//
//    constexpr auto time = __TIME__;
//    constexpr auto seed = static_cast<int>(time[ 7 ]) +
//        static_cast<int>(time[ 6 ]) * 10 +
//        static_cast<int>(time[ 4 ]) * 60 +
//        static_cast<int>(time[ 3 ]) * 600 +
//        static_cast<int>(time[ 1 ]) * 3600 +
//        static_cast<int>(time[ 0 ]) * 36000;
//
//
//    template <int N, unsigned Seed>
//    struct random_generator {
//    private:
//        static constexpr unsigned a = 16807;  // 7^5
//        static constexpr unsigned m = 2147483647;  // 2^31 - 1
//        static constexpr unsigned s = random_generator<N - 1, seed>::value;
//        static constexpr unsigned lo = a * (s & 0xFFFF);  // multiply lower 16 bits by 16807
//        static constexpr unsigned hi = a * (s >> 16);  // multiply higher 16 bits by 16807
//        static constexpr unsigned lo2 = lo + ((hi & 0x7FFF) << 16);  // combine lower 15 bits of hi with lo's upper bits
//        static constexpr unsigned hi2 = hi >> 15;  // discard lower 15 bits of hi
//        static constexpr unsigned lo3 = lo2 + hi;
//
//    public:
//        static constexpr unsigned max = m;
//        static constexpr unsigned value = lo3 > m ? lo3 - m : lo3;
//    };
//
//    template <unsigned Seed>
//    struct random_generator<0, Seed> {
//        static constexpr unsigned value = seed;
//    };
//
//    template <int N, int M, unsigned Seed>
//    struct random_int {
//        static constexpr auto value = random_generator<N + 1, Seed>::value % M;
//    };
//
//    template <int N>
//    struct random_char {
//        static const char value = static_cast<char>(1 + random_int<N, 0x7F - 1, Seed>::value);
//    };
//
//    template <int N>
//    struct random_wchar {
//        static const wchar_t value = static_cast<wchar_t>(1 + random_int<N, 0x7F - 1>::value);
//    };
//
//    template <size_t N, int K>
//    struct string {
//    private:
//        const char key_;
//        std::array<char, N + 1> encrypted_;
//
//        constexpr char enc(char c) const {
//            return c ^ key_;
//        }
//
//        char dec(char c) const {
//            return c ^ key_;
//        }
//
//    public:
//        template <size_t... Is>
//        constexpr XORSTR_INLINE string(const char* str, std::index_sequence<Is...>) :
//            key_(random_char<K>::value), encrypted_{ { enc(str[ Is ])... } } {}
//
//        XORSTR_INLINE decltype(auto) decrypt( ) {
//            for (size_t i = 0; i < N; ++i) {
//                encrypted_[ i ] = dec(encrypted_[ i ]);
//            }
//            encrypted_[ N ] = '\0';
//            return encrypted_.data( );
//        }
//    };
//
//    template <size_t N, int K>
//    struct wstring {
//    private:
//        const wchar_t key_;
//        std::array<wchar_t, N + 1> encrypted_;
//
//        constexpr wchar_t enc(wchar_t c) const {
//            return c ^ key_;
//        }
//
//        wchar_t dec(wchar_t c) const {
//            return c ^ key_;
//        }
//
//    public:
//        template <size_t... Is>
//        constexpr XORSTR_INLINE wstring(const wchar_t* str, std::index_sequence<Is...>) :
//            key_(random_wchar<K>::value), encrypted_{ { enc(str[ Is ])... } } {}
//
//        XORSTR_INLINE decltype(auto) decrypt( ) {
//            for (size_t i = 0; i < N; ++i) {
//                encrypted_[ i ] = dec(encrypted_[ i ]);
//            }
//            encrypted_[ N ] = '\0';
//            return encrypted_.data( );
//        }
//    };
//
//#undef XORSTR_INLINE
//
//}  // namespace xorstr_impl
//
//#define xorstr_(s) (xorstr_impl::string<sizeof(s) - 1, \
//  __COUNTER__>(s, std::make_index_sequence<sizeof(s) - 1>()).decrypt())
//
//#define wxorstr_(s) (xorstr_impl::wstring<sizeof(s) - 1, \
//  __COUNTER__>(s, std::make_index_sequence<sizeof(s) - 1>()).decrypt())
//
//#define OBFUSCATE_STR(s) xorstr_(s)
//#define OBFUSCATE_WSTR(s) wxorstr_(s)




#ifndef JM_XORSTR_HPP
#define JM_XORSTR_HPP

#if defined(_M_ARM64) || defined(__aarch64__) || defined(_M_ARM) || defined(__arm__)
#include <arm_neon.h>
#elif defined(_M_X64) || defined(__amd64__) || defined(_M_IX86) || defined(__i386__)
#include <immintrin.h>
#else
#error Unsupported platform
#endif

#include <cstdint>
#include <cstddef>
#include <utility>
#include <type_traits>

#define xorstr(str) ::jm::xor_string([]() { return str; }, std::integral_constant<std::size_t, sizeof(str) / sizeof(*str)>{}, std::make_index_sequence<::jm::detail::_buffer_size<sizeof(str)>()>{})
#define xorstr_(str) xorstr(str).crypt_get()

#ifdef _MSC_VER
#define XORSTR_FORCEINLINE __forceinline
#else
#define XORSTR_FORCEINLINE __attribute__((always_inline)) inline
#endif

namespace jm {

    namespace detail {

        template<std::size_t Size>
        XORSTR_FORCEINLINE constexpr std::size_t _buffer_size()
        {
            return ((Size / 16) + (Size % 16 != 0)) * 2;
        }

        template<std::uint32_t Seed>
        XORSTR_FORCEINLINE constexpr std::uint32_t key4() noexcept
        {
            std::uint32_t value = Seed;
            for (char c : __TIME__)
                value = static_cast<std::uint32_t>((value ^ c) * 16777619ull);
            return value;
        }

        template<std::size_t S>
        XORSTR_FORCEINLINE constexpr std::uint64_t key8()
        {
            constexpr auto first_part = key4<2166136261 + S>();
            constexpr auto second_part = key4<first_part>();
            return (static_cast<std::uint64_t>(first_part) << 32) | second_part;
        }

        // loads up to 8 characters of string into uint64 and xors it with the key
        template<std::size_t N, class CharT>
        XORSTR_FORCEINLINE constexpr std::uint64_t
            load_xored_str8(std::uint64_t key, std::size_t idx, const CharT* str) noexcept
        {
            using cast_type = typename std::make_unsigned<CharT>::type;
            constexpr auto value_size = sizeof(CharT);
            constexpr auto idx_offset = 8 / value_size;

            std::uint64_t value = key;
            for (std::size_t i = 0; i < idx_offset && i + idx * idx_offset < N; ++i)
                value ^=
                (std::uint64_t{ static_cast<cast_type>(str[i + idx * idx_offset]) }
            << ((i % idx_offset) * 8 * value_size));

            return value;
        }

        // forces compiler to use registers instead of stuffing constants in rdata
        XORSTR_FORCEINLINE std::uint64_t load_from_reg(std::uint64_t value) noexcept
        {
#if defined(__clang__) || defined(__GNUC__)
            asm("" : "=r"(value) : "0"(value) : );
            return value;
#else
            volatile std::uint64_t reg = value;
            return reg;
#endif
        }

    } // namespace detail

    template<class CharT, std::size_t Size, class Keys, class Indices>
    class xor_string;

    template<class CharT, std::size_t Size, std::uint64_t... Keys, std::size_t... Indices>
    class xor_string<CharT, Size, std::integer_sequence<std::uint64_t, Keys...>, std::index_sequence<Indices...>> {
#ifndef JM_XORSTR_DISABLE_AVX_INTRINSICS
        constexpr static inline std::uint64_t alignment = ((Size > 16) ? 32 : 16);
#else
        constexpr static inline std::uint64_t alignment = 16;
#endif

        alignas(alignment) std::uint64_t _storage[sizeof...(Keys)];

    public:
        using value_type = CharT;
        using size_type = std::size_t;
        using pointer = CharT*;
        using const_pointer = const CharT*;

        template<class L>
        XORSTR_FORCEINLINE xor_string(L l, std::integral_constant<std::size_t, Size>, std::index_sequence<Indices...>) noexcept
            : _storage{ ::jm::detail::load_from_reg((std::integral_constant<std::uint64_t, detail::load_xored_str8<Size>(Keys, Indices, l())>::value))... }
        {
        }

        XORSTR_FORCEINLINE constexpr size_type size() const noexcept
        {
            return Size - 1;
        }

        XORSTR_FORCEINLINE void crypt() noexcept
        {
            // everything is inlined by hand because a certain compiler with a certain linker is _very_ slow
#if defined(__clang__)
            alignas(alignment)
                std::uint64_t arr[]{ ::jm::detail::load_from_reg(Keys)... };
            std::uint64_t* keys =
                (std::uint64_t*)::jm::detail::load_from_reg((std::uint64_t)arr);
#else
            alignas(alignment) std::uint64_t keys[]{ ::jm::detail::load_from_reg(Keys)... };
#endif

#if defined(_M_ARM64) || defined(__aarch64__) || defined(_M_ARM) || defined(__arm__)
#if defined(__clang__)
            ((Indices >= sizeof(_storage) / 16 ? static_cast<void>(0) : __builtin_neon_vst1q_v(
                reinterpret_cast<uint64_t*>(_storage) + Indices * 2,
                veorq_u64(__builtin_neon_vld1q_v(reinterpret_cast<const uint64_t*>(_storage) + Indices * 2, 51),
                    __builtin_neon_vld1q_v(reinterpret_cast<const uint64_t*>(keys) + Indices * 2, 51)),
                51)), ...);
#else // GCC, MSVC
            ((Indices >= sizeof(_storage) / 16 ? static_cast<void>(0) : vst1q_u64(
                reinterpret_cast<uint64_t*>(_storage) + Indices * 2,
                veorq_u64(vld1q_u64(reinterpret_cast<const uint64_t*>(_storage) + Indices * 2),
                    vld1q_u64(reinterpret_cast<const uint64_t*>(keys) + Indices * 2)))), ...);
#endif
#elif !defined(JM_XORSTR_DISABLE_AVX_INTRINSICS)
            ((Indices >= sizeof(_storage) / 32 ? static_cast<void>(0) : _mm256_store_si256(
                reinterpret_cast<__m256i*>(_storage) + Indices,
                _mm256_xor_si256(
                    _mm256_load_si256(reinterpret_cast<const __m256i*>(_storage) + Indices),
                    _mm256_load_si256(reinterpret_cast<const __m256i*>(keys) + Indices)))), ...);

            if constexpr (sizeof(_storage) % 32 != 0)
                _mm_store_si128(
                    reinterpret_cast<__m128i*>(_storage + sizeof...(Keys) - 2),
                    _mm_xor_si128(_mm_load_si128(reinterpret_cast<const __m128i*>(_storage + sizeof...(Keys) - 2)),
                        _mm_load_si128(reinterpret_cast<const __m128i*>(keys + sizeof...(Keys) - 2))));
#else
            ((Indices >= sizeof(_storage) / 16 ? static_cast<void>(0) : _mm_store_si128(
                reinterpret_cast<__m128i*>(_storage) + Indices,
                _mm_xor_si128(_mm_load_si128(reinterpret_cast<const __m128i*>(_storage) + Indices),
                    _mm_load_si128(reinterpret_cast<const __m128i*>(keys) + Indices)))), ...);
#endif
        }

        XORSTR_FORCEINLINE const_pointer get() const noexcept
        {
            return reinterpret_cast<const_pointer>(_storage);
        }

        XORSTR_FORCEINLINE pointer get() noexcept
        {
            return reinterpret_cast<pointer>(_storage);
        }

        XORSTR_FORCEINLINE pointer crypt_get() noexcept
        {
            // crypt() is inlined by hand because a certain compiler with a certain linker is _very_ slow
#if defined(__clang__)
            alignas(alignment)
                std::uint64_t arr[]{ ::jm::detail::load_from_reg(Keys)... };
            std::uint64_t* keys =
                (std::uint64_t*)::jm::detail::load_from_reg((std::uint64_t)arr);
#else
            alignas(alignment) std::uint64_t keys[]{ ::jm::detail::load_from_reg(Keys)... };
#endif

#if defined(_M_ARM64) || defined(__aarch64__) || defined(_M_ARM) || defined(__arm__)
#if defined(__clang__)
            ((Indices >= sizeof(_storage) / 16 ? static_cast<void>(0) : __builtin_neon_vst1q_v(
                reinterpret_cast<uint64_t*>(_storage) + Indices * 2,
                veorq_u64(__builtin_neon_vld1q_v(reinterpret_cast<const uint64_t*>(_storage) + Indices * 2, 51),
                    __builtin_neon_vld1q_v(reinterpret_cast<const uint64_t*>(keys) + Indices * 2, 51)),
                51)), ...);
#else // GCC, MSVC
            ((Indices >= sizeof(_storage) / 16 ? static_cast<void>(0) : vst1q_u64(
                reinterpret_cast<uint64_t*>(_storage) + Indices * 2,
                veorq_u64(vld1q_u64(reinterpret_cast<const uint64_t*>(_storage) + Indices * 2),
                    vld1q_u64(reinterpret_cast<const uint64_t*>(keys) + Indices * 2)))), ...);
#endif
#elif !defined(JM_XORSTR_DISABLE_AVX_INTRINSICS)
            ((Indices >= sizeof(_storage) / 32 ? static_cast<void>(0) : _mm256_store_si256(
                reinterpret_cast<__m256i*>(_storage) + Indices,
                _mm256_xor_si256(
                    _mm256_load_si256(reinterpret_cast<const __m256i*>(_storage) + Indices),
                    _mm256_load_si256(reinterpret_cast<const __m256i*>(keys) + Indices)))), ...);

            if constexpr (sizeof(_storage) % 32 != 0)
                _mm_store_si128(
                    reinterpret_cast<__m128i*>(_storage + sizeof...(Keys) - 2),
                    _mm_xor_si128(_mm_load_si128(reinterpret_cast<const __m128i*>(_storage + sizeof...(Keys) - 2)),
                        _mm_load_si128(reinterpret_cast<const __m128i*>(keys + sizeof...(Keys) - 2))));
#else
            ((Indices >= sizeof(_storage) / 16 ? static_cast<void>(0) : _mm_store_si128(
                reinterpret_cast<__m128i*>(_storage) + Indices,
                _mm_xor_si128(_mm_load_si128(reinterpret_cast<const __m128i*>(_storage) + Indices),
                    _mm_load_si128(reinterpret_cast<const __m128i*>(keys) + Indices)))), ...);
#endif

            return (pointer)(_storage);
        }
    };

    template<class L, std::size_t Size, std::size_t... Indices>
    xor_string(L l, std::integral_constant<std::size_t, Size>, std::index_sequence<Indices...>) -> xor_string<
        std::remove_const_t<std::remove_reference_t<decltype(l()[0])>>,
        Size,
        std::integer_sequence<std::uint64_t, detail::key8<Indices>()...>,
        std::index_sequence<Indices...>>;

} // namespace jm

#endif // include guard
