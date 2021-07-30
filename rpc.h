#ifndef RPC_H_
#define RPC_H_
#include "json.hpp"
#include <iostream>
using json = nlohmann::json;
class Rpc
{
public:
    Rpc()
    {
    }

    ~Rpc()
    {
    }

    struct Utxo
    {
        std::string txid;
        int n ;
        double amount;
        std::string pubkey_script;
    };

    struct EthAccount
    {
        std::string address;
        std::string contract;
        bool token;
        int decimal;
    };

    static Rpc& instance();

    void initProxy(const std::string& proxy, const std::string& auth)
    {
        proxy_ = proxy;
        auth_ = auth;
    }

    std::string getProxy()
    {
        return proxy_;
    }

public:
   
    bool getBalance(EthAccount &eth_account, std::string &balance);//eth

    bool getUtxo(const std::string &address, std::vector<Utxo>& vect_utxo, double& total);//btc

    bool sendBtcSignTx(const std::string& sign_rawtransaction_hex, std::string& txid);//btc

    bool sendEthSignTx(const std::string sign_rawtransaction_hex, std::string& txid);//eth

    bool getGasPrice(std::string& gas_price);//eth

    bool getNonce(const std::string &address, std::string& nonce);//eth

    bool estimatesmartfee(const int conf_target, double &fee);//btc

    bool walletPropose(std::string& address, std::string& priv_key);//aitd

    bool submit(const std::string& secret, const std::string &from_address, const std::string& to_address, uint64_t tag, uint64_t aitd_amount, uint64_t fee, std::string& txid);//

    bool accountInfo(const std::string& address, double& total);//aitd
public:
    bool addadress(const std::string& address);

protected:

    bool rpcProxy(const std::string &node_relay, const json& json_post, json& json_response);

    bool rpcProxy(const json& json_post, json& json_response);

    bool structRpc(const std::string& method, const json& json_params, json& json_post);

protected:
    static Rpc single_;
    std::string proxy_;
    std::string auth_;

};

#endif // RPC_H

