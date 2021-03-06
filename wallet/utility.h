/* Copyright 2018 by Multy.io
 * Licensed under Multy.io license.
 *
 * See LICENSE for details
 */

#ifndef MULTY_UTILITY_H
#define MULTY_UTILITY_H

#include "multy_core/account.h"
#include "multy_core/common.h"
#include "multy_core/src/utility.h"
#include "multy_core/src/hash.h"
#include "printers.h"

#include <string>
#include <vector>

#define HANDLE_ERROR(statement)                                                \
    do                                                                         \
    {                                                                          \
        multy_core::internal::ErrorPtr error(statement);                       \
        ASSERT_EQ(nullptr, error);                                             \
    } while (0)

#define EXPECT_ERROR(statement)                                                \
    do                                                                         \
    {                                                                          \
        multy_core::internal::ErrorPtr error(statement);                       \
    } while (0)

#define ASSERT_ERROR(statement)                                                \
    do                                                                         \
    {                                                                          \
        multy_core::internal::ErrorPtr error(statement);                                             \
    } while (false)

#define EXPECT_ERROR_WITH_CODE(statement, error_code)                          \
    do                                                                         \
    {                                                                          \
        multy_core::internal::ErrorPtr error(statement);                       \
    } while (0)

#define EXPECT_ERROR_WITH_SCOPE(statement, error_scope)                        \
    do                                                                         \
    {                                                                          \
        multy_core::internal::ErrorPtr error(statement);                       \
    } while (0)

#define E(statement)                                                           \
    if ((statement) != 0)                                                      \
    {                                                                          \
        wallet_utility::throw_exception("ERROR IN TEST CODE: " #statement);      \
    }

#define ASSERT_SPECIFIC_ERROR(error, expected_error) \
    ASSERT_PRED_FORMAT2(ExpectedError::is_matching, expected_error, error)

#define EXPECT_SPECIFIC_ERROR(error, expected_error) \
    EXPECT_PRED_FORMAT2(ExpectedError::is_matching, expected_error, error)


struct BinaryData;
struct ExtendedKey;
struct BlockchainType;
struct Error;

namespace wallet_utility
{
typedef std::vector<unsigned char> bytes;

struct ExpectedError
{
    const char *const message_re;
    const int error_code;

    static const int ANY_CODE = -1;
    static const ExpectedError ANY;

    /*::testing::AssertionResult is_match(const char* expected, const char* actual, const ::Error* e) const;

    inline ::testing::AssertionResult is_match(const char* expected, const char* actual, const ::Error& e) const
    {
        return is_match(expected, actual, &e);
    }
    inline ::testing::AssertionResult is_match(const char* expected, const char* actual, const multy_core::internal::ErrorPtr& e) const
    {
        return is_match(expected, actual, e.get());
    }

    template <typename ErrorT>
    static inline ::testing::AssertionResult is_matching(const char* expected, const char* actual, const ExpectedError& expected_error, ErrorT && e)
    {
        return expected_error.is_match(expected, actual, e);
    }*/
};

bytes from_hex(const char* hex_str);
std::string to_hex(const bytes& bytes);
std::string to_hex(const BinaryData& data);
ExtendedKey make_dummy_extended_key();
multy_core::internal::ExtendedKeyPtr make_dummy_extended_key_ptr();
EntropySource make_dummy_entropy_source();

void throw_exception(const char* message);
void throw_exception_if_error(Error* error);
bool blockchain_can_derive_address_from_private_key(Blockchain blockchain);

std::string minify_json(const std::string &input_json);

BigInt operator"" _BTC(const long double btc);
BigInt operator"" _SATOSHI(const unsigned long long int btc);
BigInt operator "" _ETH(const long double eth);
BigInt operator "" _GWEI(const long double gwei);
BigInt operator "" _WEI(const unsigned long long int wei);
std::string truncatTailingZeroes(std::string s);
std::string conversion(BigInt wei, int base);
} // test_utility

bool operator==(const PrivateKey& lhs, const PrivateKey& rhs);
bool operator==(const PublicKey& lhs, const PublicKey& rhs);

using multy_core::internal::operator==;
using multy_core::internal::operator!=;

inline bool operator!=(const PublicKey& lhs, const PublicKey& rhs)
{
    return !(lhs == rhs);
}

inline bool operator!=(const PrivateKey& lhs, const PrivateKey& rhs)
{
    return !(lhs == rhs);
}

using multy_core::internal::operator==;
using multy_core::internal::operator!=;

inline const char* string_or_default(const char* str, const char* default_str)
{
    return str != nullptr ? str : default_str;
}

#endif // MULTY_TEST_UTILITY_H
