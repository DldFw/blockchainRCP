#include "hdwallet.h"
#include "multy_core/ethereum.h"
#include "multy_core/src/ethereum/ethereum_transaction.h"
#include "multy_core/common.h"
#include "multy_core/src/api/key_impl.h"
#include "multy_core/src/api/account_impl.h"
#include "multy_core/key.h"
#include "multy_core/error.h"
#include "multy_core/src/utility.h"
#include "multy_core/mnemonic.h"
#include "multy_core/src/ethereum/ethereum_transaction_builder.h"

#include "multy_core/src/bitcoin/bitcoin_transaction.h"
#include "multy_core/src/api/properties_impl.h"
#include "multy_core/account.h"
#include "multy_core/big_int.h"
#include "multy_core/properties.h"
#include "multy_core/binary_data.h"
#include "multy_core/src/api/account_impl.h"
#include "multy_core/src/exception.h"
#include "multy_core/src/exception_stream.h"
#include "multy_core/src/u_ptr.h"
#include "multy_core/transaction_builder.h"
#include "utility.h"
#include "rpc.h"
#include "multy_core/src/api/transaction_builder_impl.h"

#include <wally_bip32.h>
#include <iostream>
#include <qlogging.h>
#include <map>
#include "multy_core/account.h"
#include "multy_core/bitcoin.h"
#include "multy_core/ethereum.h"
#include "multy_core/golos.h"
#include "multy_core/eos.h"
#include "wally_core.h"

#include <QString>
#include "dialogmasterkey.h"
#include "mainwindow.h"
#include <QMessageBox>

const BlockchainType BITCOIN_MAIN_NET {BLOCKCHAIN_BITCOIN, BITCOIN_NET_TYPE_MAINNET};
const BlockchainType BITCOIN_TEST_NET {BLOCKCHAIN_BITCOIN, BITCOIN_NET_TYPE_TESTNET};

const BlockchainType ETHEREUM_MAIN_NET { BLOCKCHAIN_ETHEREUM, ETHEREUM_CHAIN_ID_MAINNET };
const BlockchainType ETHEREUM_LOCAL_NET { BLOCKCHAIN_ETHEREUM, 10 };

const BlockchainType GOLOS_MAIN_NET { BLOCKCHAIN_GOLOS, GOLOS_NET_TYPE_MAINNET };
const BlockchainType GOLOS_TEST_NET { BLOCKCHAIN_GOLOS, GOLOS_NET_TYPE_TESTNET };

const BlockchainType EOS_MAIN_NET { BLOCKCHAIN_EOS, EOS_NET_TYPE_MAINNET};
const BlockchainType EOS_TEST_NET { BLOCKCHAIN_EOS, EOS_NET_TYPE_TESTNET};

using namespace multy_core::internal;
using namespace wallet_utility;
static size_t feed_silly_entropy(void*, size_t size, void* dest)
{
    /** Poor man's entropy, using uninitialized data from stack, which is:
     * - Fast;
     * - Unsecure;
     * - Somewhat predictable;
     * And hence SHOULD not be used in production, but Ok in POC driver program.
     */
    static const size_t entropy_max_size = 1024;
    unsigned char silly_entropy[entropy_max_size];

    uint64_t current_time = 0;
    QDateTime::currentDateTime().setSecsSinceEpoch(current_time);
    for(size_t i = 0; i < 1024; i++)
    {
        uint64_t radom = current_time + silly_entropy[i] + rand();
        silly_entropy[i] = radom%256;
    }

    if (size > entropy_max_size)
    {
        return 0;
    }

    memcpy(dest, silly_entropy, size);
    return size;
}

static ExtendedKey s_master_key;
static std::map<size_t, ExtendedKey> s_map_childcode_key;

static std::string GetMasterPublicKey(ExtendedKey& master_key)
{
    unsigned char serialized_key[BIP32_SERIALIZED_LEN] = {'\0'};
    bip32_key_serialize(&master_key.key, 1, serialized_key, sizeof(serialized_key));
    CharPtr out_str;
    wally_base58_from_bytes(serialized_key, sizeof(serialized_key), BASE58_FLAG_CHECKSUM, reset_sp(out_str));
    std::string ret = std::string(out_str.get());
    return ret;
}

bool is_between(const BigInt& left, const BigInt& value, const BigInt& right)
{
    if (left > right)
    {
        THROW_EXCEPTION("Invalid values: left > right")
                << " left: " << left << ", right: " << right;
    }
    return (left <= value) && (value <= right);
}

AccountPtr make_account(BlockchainType blockchain, const char* serialized_private_key)
{
    AccountPtr account;
    throw_if_error(make_account(blockchain,BITCOIN_ACCOUNT_DEFAULT,serialized_private_key,reset_sp(account)));
    return account;
}

struct TransactionFee
{
    BigInt amount_per_byte;
};

struct TransactionSource
{
    BigInt available;
    bytes prev_tx_hash;
    size_t prev_tx_index;
    bytes prev_tx_scrip_pubkey;
    PrivateKey* private_key;
};

struct TransactionDestination
{
    std::string address;
    BigInt amount;
    bool is_change = false;

    TransactionDestination(
            std::string address,
            BigInt amount)
        : TransactionDestination(std::move(address), std::move(amount), false)
    {}

protected:
    TransactionDestination(
            std::string address,
            BigInt amount,
            bool is_change)
        : address(std::move(address)),
          amount(std::move(amount)),
          is_change(is_change)
    {}
};

struct TransactionChangeDestination : public TransactionDestination
{
    TransactionChangeDestination(const std::string& address)
        : TransactionDestination(address, BigInt{}, true)
    {}
};

struct TransactionTemplate
{
    Account* account;
    TransactionFee fee;
    std::vector<TransactionSource> sources;
    std::vector<TransactionDestination> destinations;
};


/*static bool GetAccount(ExtendedKey& key, size_t index)
{
    bool ret = true;
    HDAccountPtr root_account;
    BlockchainType blockchain_type;
    //blockchain_type.blockchain = BLOCKCHAIN_BITCOIN;
    blockchain_type.blockchain = BLOCKCHAIN_ETHEREUM;
    blockchain_type.net_type = 0;

    make_hd_account(&key, blockchain_type ,ACCOUNT_TYPE_DEFAULT,0, reset_sp(root_account));
    AccountPtr account;
    make_hd_leaf_account(root_account.get(), ADDRESS_EXTERNAL, index, reset_sp(account));

    ConstCharPtr address_str;
    account_get_address_string(account.get(), reset_sp(address_str));
    KeyPtr private_key;
    account_get_key(account.get(), KEY_TYPE_PRIVATE, reset_sp(private_key));
    KeyPtr public_key;
    account_get_key(account.get(), KEY_TYPE_PUBLIC, reset_sp(public_key));
    return ret;
}*/

bool HDwallet::createMasterKey(const std::string &password)
{
    bool ret = false;
    if (init_hd_)
    {
        qInfo("has init hd master key");
        return false;
    }
    try
    {
        const EntropySource entropy{nullptr, &feed_silly_entropy};
        ConstCharPtr mnemonic;
        throw_if_error(make_mnemonic(entropy, reset_sp(mnemonic)));
        ConstCharPtr mnemonic_directory;
        const char* directory;
        mnemonic_get_dictionary(&directory);
        std::string str_mnemonic = mnemonic.get();
        BinaryDataPtr seed;
        throw_if_error(make_seed(mnemonic.get(), password.c_str(), reset_sp(seed)));
        ConstCharPtr seed_string;
        throw_if_error(seed_to_string(seed.get(), reset_sp(seed_string)));
        //std::cout << "Seed: " << seed_string.get() << std::endl;
        memset(&s_master_key.key, 0, sizeof(s_master_key.key));
        bip32_key_from_seed(seed->data,seed->len,BIP32_VER_MAIN_PRIVATE,0,&s_master_key.key);
        //bip32_key_from_seed(seed->data,seed->len,BIP32_VER_TEST_PRIVATE,0,&s_master_key.key);
        //std::cout << s_master_key.to_string() << std::endl;
        std::string pubkey = GetMasterPublicKey(s_master_key);
        // std::cout << "pubkey: " << pubkey << std::endl;
        //GetAccount(s_master_key, 0);
        static DialogMasterKey dlg;
        dlg.setShowText(QString(s_master_key.to_string().c_str()), QString(pubkey.c_str()), QString(str_mnemonic.c_str()));
        dlg.show();

        if(s_master_key.is_valid())
        {
            hd_info_.mnemonic = mnemonic.get();
            hd_info_.passwd = password;
            hd_info_.seed = seed_string.get();
            init_hd_ = true;
            ret = true;
        }
    }
    catch (Error* e)
    {
        qWarning("create master key error: ");
        qWarning(e->message);
        free_error(e);
        ret = false;
    }
    return ret;
}

bool HDwallet::createChildKey(size_t chain_code)
{
    bool ret = false;

    if (!init_hd_)
    {
        qWarning("master key has not init");
        return ret;
    }
    try
    {
        BinaryDataPtr seed;
        throw_if_error(make_seed(hd_info_.mnemonic.c_str(), hd_info_.passwd.c_str(), reset_sp(seed)));
        ExtendedKey master_key;
        memset(&master_key.key, 0, sizeof(master_key.key));
        bip32_key_from_seed(seed->data,seed->len,BIP32_VER_MAIN_PRIVATE,0,&master_key.key);
        //bip32_key_from_seed(seed->data,seed->len,BIP32_VER_TEST_PRIVATE,0,&master_key.key);
        ExtendedKeyPtr child_key = make_child_key(master_key, chain_code);
        if (child_key->is_valid())
        {
            vect_child_code_.push_back(chain_code);
            s_map_childcode_key[chain_code] = *child_key;
            ret = true;
        }
    }
    catch (Error*e)
    {
        qWarning("create child key error: ");
        qWarning(e->message);
        free_error(e);
    }
    return ret;

}

bool HDwallet::importMasterKey(const std::string &mnemonic, const std::string &passwd, bool reset)
{
    bool ret = false;
    if (!reset && init_hd_)
    {
        qInfo("has been init, can't reset");
        return  ret;
    }

    try
    {
        BinaryDataPtr seed;
        throw_if_error(make_seed(mnemonic.c_str(), passwd.c_str(), reset_sp(seed)));
        memset(&s_master_key.key, 0, sizeof(s_master_key.key));
        bip32_key_from_seed(seed->data,seed->len,BIP32_VER_MAIN_PRIVATE,0,&s_master_key.key);

        if( s_master_key.is_valid() )
        {
            init_hd_ = true;
            hd_info_.mnemonic = mnemonic;
            hd_info_.passwd = passwd;
            ConstCharPtr seed_string;
            throw_if_error(seed_to_string(seed.get(), reset_sp(seed_string)));
            hd_info_.seed = seed_string.get();
        }
    }
    catch (Error*e)
    {
        qWarning("create child key error: ");
        qWarning(e->message);
        free_error(e);
    }
    return ret;
}

bool HDwallet::getHdInfo(HDwallet::HdInfo &hd_info)
{
    hd_info.mnemonic = hd_info_.mnemonic;
    hd_info.passwd = hd_info_.passwd;
    hd_info.seed = hd_info_.seed;
    return init_hd_;
}

bool HDwallet::getAccount(HDwallet::Account &account, HDwallet::CoinType coin_type, const size_t &chain_code, const size_t &index)
{
    bool ret = false;

    if (s_map_childcode_key.find(chain_code) == s_map_childcode_key.end() &&
            !createChildKey(chain_code))
    {
        qInfo("not have chain code");
        return ret;
    }

    ExtendedKey key = s_map_childcode_key[chain_code];
    HDAccountPtr root_account;

    BlockchainType blockchain_type;
    if (coin_type == BTC)
    {
        blockchain_type.blockchain = BLOCKCHAIN_BITCOIN;
        blockchain_type.net_type = BITCOIN_NET_TYPE_MAINNET;
    }
    else if(coin_type == ETH)
    {
        blockchain_type.blockchain = BLOCKCHAIN_ETHEREUM;
        blockchain_type.net_type = 0;
    }
    //blockchain_type.net_type = 0;

    make_hd_account(&key, blockchain_type ,ACCOUNT_TYPE_DEFAULT,0, reset_sp(root_account));
    AccountPtr ptr_account;
    make_hd_leaf_account(root_account.get(), ADDRESS_EXTERNAL, index, reset_sp(ptr_account));

    ConstCharPtr address_str;
    account_get_address_string(ptr_account.get(), reset_sp(address_str));
    account.address = address_str.get();

    KeyPtr private_key;
    account_get_key(ptr_account.get(), KEY_TYPE_PRIVATE, reset_sp(private_key));
    account.privkey = private_key->to_string();

    KeyPtr public_key;
    account_get_key(ptr_account.get(), KEY_TYPE_PUBLIC, reset_sp(public_key));
    return ret;

}

bool HDwallet::transferCoin(const HDwallet::Transfer &transfer, std::string &txid)
{
    bool ret = false;

    switch (transfer.coin_type)
    {
    case HDwallet::BTC:
        ret = transferBtc(transfer, txid);
        break;
    case HDwallet::ETH:
        ret = transferEth(transfer, txid);
        break;
    default:
        break;
    }

    return ret;
}

bool HDwallet::transferBtc(const HDwallet::Transfer &transfer, std::string &txid)
{
    bool ret = false;
    std::vector<Rpc::Utxo> vect_utxo;
    double total = 0;
    Rpc::instance().getUtxo(transfer.from, vect_utxo, total);
    TransactionPtr transaction;
    QString privkey  ;
    MainWindow::wallet_.getPrivkey(QString(transfer.from.c_str()), privkey);

    AccountPtr account = make_account(BITCOIN_MAIN_NET,privkey.toStdString().c_str());
    throw_if_error(make_transaction(account.get(), reset_sp(transaction)));
    uint64_t fee_price = (uint64_t)(std::atof(transfer.fee.c_str()) * 100000000 / 1000);

    if (fee_price == 0)
    {
        qInfo("BTC transfer fee is too small.");
        return ret;
    }

    Properties& fee = transaction->get_fee();
    BigInt fee_byte(fee_price);
    fee.set_property_value("amount_per_byte", fee_byte);

    {
        uint64_t amount = std::atof(transfer.amount.c_str()) * 100000000;
        if (total * 100000000 < amount + fee_price * 225)
        {
            qWarning("BTC Insufficient balance");
            return ret;
        }
    }

    {
        auto amount = std::atof(transfer.amount.c_str());
        if (amount < 0.00000546)
        {
            qWarning("BTC transfer amount is too few.");
            return ret;
        }
    }

    {
        uint64_t amount = std::atof(transfer.amount.c_str()) * 100000000;
        BigInt balance(amount);
        Properties& destination = transaction->add_destination();
     //                                                                                                                                                                                             destination.set_property_value("amount", balance);
        destination.set_property_value("address", transfer.to);
        destination.set_property_value("is_change", static_cast<int32_t>(false));
    }

    {
        uint64_t amount = total * 100000000 - std::atof(transfer.amount.c_str()) * 100000000 ;
        BigInt balance(amount);
        Properties& destination = transaction->add_destination();
        destination.set_property_value("amount", balance);
        destination.set_property_value("address", transfer.from);
        destination.set_property_value("is_change", static_cast<int32_t>(true));
    }

    for(size_t i = 0; i < vect_utxo.size(); i++)
    {
        Properties& source = transaction->add_source();
        uint64_t amount = vect_utxo[i].amount * 100000000;
        BigInt balance(amount);
        source.set_property_value("amount", balance);

        BinaryData* txid;
        make_binary_data_from_hex(vect_utxo[i].txid.c_str(), &txid);
        source.set_property_value("prev_tx_hash", *txid);
        source.set_property_value("prev_tx_out_index", vect_utxo[i].n);

        BinaryData* pubkey_script;
        make_binary_data_from_hex(vect_utxo[i].pubkey_script.c_str(), &pubkey_script);
        source.set_property_value("prev_tx_out_script_pubkey", *pubkey_script);

        PrivateKeyPtr account_privkey = account->get_private_key();
        source.set_property_value("private_key", *account_privkey);
    }

    BigInt estimated_fee = transaction->estimate_total_fee(1, 1);
    std::string btc_fee = "btc estimated fee: " + estimated_fee.get_value();

    QMessageBox::StandardButton question = QMessageBox::question(nullptr,"BTC fee", btc_fee.c_str());
    if (question == QMessageBox::Yes)
    {
        const BinaryDataPtr serialied = transaction->serialize();
        std::string hex_tx = to_hex(*serialied);
        qDebug(hex_tx.data());
        ret = Rpc::instance().sendBtcSignTx(hex_tx,txid);
    }
    if (question == QMessageBox::No || question == QMessageBox::Close)
    {

    }
    return ret;
}

bool HDwallet::transferEth(const HDwallet::Transfer &transfer, std::string &txid)
{
    bool ret = false;
    QString privkey  ;
    MainWindow::wallet_.getPrivkey(QString(transfer.from.c_str()), privkey);
    AccountPtr account ;
    make_account(ETHEREUM_MAIN_NET,ACCOUNT_TYPE_DEFAULT,privkey.toStdString().c_str(), reset_sp(account));
    std::string nonce ;
    Rpc::instance().getNonce(transfer.from, nonce);

    if(!transfer.is_token)
    {
        TransactionPtr transaction;
        make_transaction(account.get(), reset_sp(transaction));
        std::string amount ;
        Rpc::EthAccount rpcAccount;
        rpcAccount.address = transfer.from;
        rpcAccount.token = false;
        Rpc::instance().getBalance(rpcAccount, amount);
        const BigInt balance(amount.substr(2).c_str(), 16);
        BigInt value((uint64_t)(std::atof(transfer.amount.c_str()) * 1000000000));
        value *= 1000000000;
        uint64_t fee_wei = (uint64_t)(std::atof(transfer.fee.c_str()) * 1000000000 * 1000000000);
        BigInt fee (fee_wei);
        if (balance < value + fee)
        {
            qWarning("Insufficient balance");
            return ret;
        }
        const BigInt gas_limit(21000);
        const BigInt gas_price = fee / gas_limit;
        {
            Properties& properties = transaction->get_transaction_properties();
            properties.set_property_value("nonce", BigInt(nonce.substr(2).c_str(), 16));
        }

        {
            Properties& source = transaction->add_source();
            source.set_property_value("amount", balance);
        }

        {
            Properties& destination = transaction->add_destination();
            destination.set_property_value("address", transfer.to.substr(2).c_str());
            destination.set_property_value("amount", value);
        }

        {
            Properties& fee = transaction->get_fee();
            fee.set_property_value("gas_price", gas_price);
            fee.set_property_value("gas_limit", gas_limit);
        }
        const BinaryDataPtr serialied = transaction->serialize();
        std::string hex_tx ="0x" + to_hex(*serialied);
        qDebug(hex_tx.data());
        ret = Rpc::instance().sendEthSignTx(hex_tx, txid);

    }
    else
    {

        TransactionBuilderPtr builder ;
        make_transaction_builder(account.get(), ETHEREUM_TRANSACTION_BUILDER_ERC20, "transfer", reset_sp(builder));

        Properties* builder_propertie;
        BigInt request(0);
        Rpc::EthAccount rpc_account;
        rpc_account.address = transfer.from;
        rpc_account.token = false;

        std::string from_eth;
        Rpc::instance().getBalance(rpc_account, from_eth);
        BigInt balance_eth(from_eth.substr(2).c_str(), 16);

        std::string from_token;
        rpc_account.contract = transfer.contract;
        rpc_account.decimal = transfer.decimal;
        rpc_account.token = true;
        Rpc::instance().getBalance(rpc_account, from_token);
        BigInt balance_token(from_token.substr(2).c_str(), 16);
        double transfer_token = std::atof(transfer.amount.c_str());

        for (size_t i = 0; i < transfer.decimal; i++)
        {
            transfer_token *= 10;
        }

        BigInt transfer_amount_token((uint64_t)transfer_token);

        if (transfer_amount_token == 0)
        {
            qWarning("ERC20 transfer amount too few");
            return ret;
        }

        if (balance_token < transfer_amount_token)
        {
            qWarning("ERC20 Insufficient balance");
            return ret;
        }
        transaction_builder_get_properties(builder.get(), &builder_propertie);
        properties_set_big_int_value(builder_propertie, "balance_eth", &balance_eth);
        properties_set_string_value(builder_propertie, "contract_address", transfer.contract.c_str());

        properties_set_big_int_value(builder_propertie, "balance_token", &balance_token);
        properties_set_big_int_value(builder_propertie, "transfer_amount_token", &transfer_amount_token);

        properties_set_string_value(builder_propertie, "destination_address", transfer.to.c_str());
        properties_set_string_value(builder_propertie, "action", "send");
        properties_set_big_int_value(builder_propertie, "request_id", &request);

        TransactionPtr transaction = builder->make_transaction();
        Properties* transaction_properties = nullptr;

        BigInt raw_nonce = BigInt(nonce.substr(2).c_str(), 16);
        transaction_get_properties(transaction.get(), &transaction_properties);
        properties_set_big_int_value(transaction_properties, "nonce", &raw_nonce);

        Properties* fee_propeties = nullptr;
        transaction_get_fee(transaction.get(), &fee_propeties);
        uint64_t fee_wei = (uint64_t)(std::atof(transfer.fee.c_str()) * 1000000000 * 1000000000);
        BigInt fee (fee_wei);

        if (balance_eth < fee_wei)
        {
            qWarning("ERC20 tx, eth Insufficient balance");
            return ret;
        }

        const BigInt gas_limit(60000);
        const BigInt gas_price = fee / gas_limit;
        properties_set_big_int_value(fee_propeties, "gas_price", &gas_price);
        properties_set_big_int_value(fee_propeties, "gas_limit", &gas_limit);

        const BinaryDataPtr serialied = transaction->serialize();
        std::string hex_tx ="0x" + to_hex(*serialied);
        qDebug(hex_tx.data());
        ret = Rpc::instance().sendEthSignTx(hex_tx, txid);
    }

    return ret;
}
