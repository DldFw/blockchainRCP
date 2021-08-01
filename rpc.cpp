#include "rpc.h"
//#include <glog/logging.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <curl/curl.h>
#include <qlogging.h>
#include <QString>

struct CurlParams
{
    std::string url;
    std::string auth;
    std::string data;
    std::string content_type;
    bool need_auth;
    CurlParams()
    {
        need_auth = true;
        content_type = "content-type:application/json";
    }
};

static size_t ReplyCallback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    std::string *str = (std::string*)stream;
    (*str).append((char*)ptr, size*nmemb);
    return size * nmemb;
}

static bool CurlPostParams(const CurlParams &params, std::string &response)
{
    CURL *curl = curl_easy_init();
    struct curl_slist *headers = NULL;
    CURLcode res;
    response.clear();
    std::string error_str ;
    if (curl)
    {
        headers = curl_slist_append(headers, params.content_type.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, params.url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)params.data.size());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ReplyCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

        if(params.need_auth)
        {
            curl_easy_setopt(curl, CURLOPT_USERPWD, params.auth.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
        }

        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 120);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120);
        res = curl_easy_perform(curl);
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK)
    {
        error_str = curl_easy_strerror(res);
        std::cerr << error_str.c_str() << std::endl;
        qWarning("url: ");
        qWarning(params.url.c_str());
        qWarning("error: ");
        qWarning(error_str.c_str());
        return false;
    }
    return true;
}

static bool CurlPost(const std::string& url, const json &json_post, const std::string& auth, json& json_response)
{
    CurlParams curl_params;
    curl_params.auth = auth;
    curl_params.url = url;
    curl_params.data = json_post.dump();
    std::string response;
    bool ret = CurlPostParams(curl_params,response);
    if (!ret)
        return ret;

    qInfo(response.c_str());
    json_response = json::parse(response);
    if (!json_response["error"].is_null())
    {
        qWarning(response.c_str());
        qWarning(curl_params.data.c_str());
        ret = false;
        return ret;
    }
    return ret;
}

bool Rpc::structRpc(const std::string& method, const json& json_params, json& json_post)
{
    json_post["jsonrpc"] = "2.0";
    json_post["id"] = "curltest";
    json_post["method"] = method;
    json_post["params"] = json_params;
    return true;
}

Rpc Rpc::single_;
Rpc &Rpc::instance()
{
    return single_;
}

bool Rpc::getBalance(EthAccount &eth_account, std::string& balance)
{
    bool ret = false;
    json json_post;
    json json_params;

    if (eth_account.token)
    {
        json params;
        params["to"] = eth_account.contract;
        params["data"] = "0x70a08231000000000000000000000000" + eth_account.address.substr(2);
        json_params.push_back(params);
        json_params.push_back("latest");
        structRpc("eth_call", json_params, json_post);
    }
    else
    {
        json_params.push_back(eth_account.address);
        json_params.push_back("latest");
        structRpc("eth_getBalance", json_params, json_post);
    }

    json json_response;
    ret = rpcProxy("ethrelay", json_post, json_response);
    if(!ret)
    {
        qWarning("eth rpc sendBtcSignTx error" );
        return ret;
    }

    balance = json_response["result"].get<std::string>();
    return ret;
}

bool Rpc::getUtxo(const std::string& address, std::vector<Rpc::Utxo> &vect_utxo, double &total)
{
    bool ret = false;
    json json_post;
    json json_params;
    json_params.push_back(address);
    json_post["params"] = json_params;

    structRpc("listunspent", json_params, json_post);
    json json_response;
    if ( !rpcProxy(json_post, json_response) )
    {
        qWarning("btc rpc getUtxo error" );
        return ret;
    }

    json json_unspent = json_response["result"];
    for(size_t i = 0; i < json_unspent.size(); i++)
    {
        Utxo unspent_tx;
        json json_vin;
        unspent_tx.txid = json_unspent[i][0].get<std::string>();
        unspent_tx.n = json_unspent[i][1].get<int>();
        unspent_tx.amount = json_unspent[i][2].get<double>();
        unspent_tx.pubkey_script = json_unspent[i][3].get<std::string>();
        total += unspent_tx.amount;
        vect_utxo.push_back(unspent_tx);
        ret = true;
    }
    return ret;
}

bool Rpc::sendBtcSignTx(const std::string& sign_rawtransaction_hex, std::string &txid)
{
    bool ret = false;
    json json_post;
    json json_params;
    json_params.push_back(sign_rawtransaction_hex);
    json_post["params"] = json_params;
    structRpc("sendrawtransaction", json_params, json_post);

    json json_response;
    ret = rpcProxy("btcrelay", json_post, json_response);
    if(!ret)
    {
        qWarning("btc rpc sendBtcSignTx error" );
        return ret;
    }

    txid = json_response["result"].get<std::string>();
    ret = true;
    return ret;
}

bool Rpc::sendEthSignTx(const std::string sign_rawtransaction_hex, std::string &txid)
{
    bool ret = false;
    json json_post;
    json json_params;
    json_params.push_back(sign_rawtransaction_hex);
    json_post["params"] = json_params;
    structRpc("eth_sendRawTransaction", json_params, json_post);
    json json_response;
    ret = rpcProxy("ethrelay", json_post, json_response);
    if(!ret)
    {
        qWarning("eth rpc sendEthSignTx error" );
        return ret;
    }

    txid = json_response["result"].get<std::string>();
    ret = true;
    return ret;
}

bool Rpc::getGasPrice(std::string &gas_price)
{
    bool ret = false;
    json json_post;
    json json_params = json::array();
    json_post["params"] = json_params;
    structRpc("eth_gasPrice", json_params, json_post);

    json json_response;
    ret = rpcProxy("ethrelay", json_post, json_response);
    if(!ret)
    {
        qWarning("eth rpc getGasPrice error" );
        return ret;
    }

    gas_price = json_response["result"].get<std::string>();
    ret = true;
    return ret;
}

bool Rpc::getNonce(const std::string &address, std::string &nonce)
{
    bool ret = false;
    json json_post;
    json json_params = json::array();
    json_params.push_back(address);
    json_params.push_back("latest");
    json_post["params"] = json_params;
    structRpc("eth_getTransactionCount", json_params, json_post);

    json json_response;
    ret = rpcProxy("ethrelay", json_post, json_response);
    if(!ret)
    {
        qWarning("eth rpc getNonce error" );
        return ret;
    }

    nonce = json_response["result"].get<std::string>();
    return ret;
}

bool Rpc::estimatesmartfee(const int conf_target, double& fee)
{
    bool ret = false;
    json json_post;
    json json_params;
    json_params.push_back(conf_target);
    json_post["params"] = json_params;
    structRpc("estimatesmartfee", json_params, json_post);

    json json_response;
    ret = rpcProxy("btcrelay", json_post, json_response);
    if(!ret)
    {
        qWarning("btc rpc estimatesmartfee error" );
        return ret;
    }

    fee = json_response["result"]["feerate"].get<double>();
    ret = true;
    return ret;
}

bool Rpc::walletPropose(std::string &address, std::string &priv_key)
{
    bool ret = false;
    json json_post;
    json json_params = json::array();
    json json_param = json::object();
    json_params.push_back(json_param);
    json_post["params"] = json_params;
    structRpc("wallet_propose", json_params, json_post);

    json json_response;
    ret = rpcProxy("aitdrelay", json_post, json_response);
    if(!ret)
    {
        qWarning("aitd rpc wallet_propose error" );
        return ret;
    }

    address = json_response["result"]["account_id"].get<std::string>();
    priv_key = json_response["result"]["master_seed"].get<std::string>();
    ret = true;
    return ret;

}

bool Rpc::submit(const std::string &secret, const std::string & from_address,const std::string &to_address, uint64_t tag, uint64_t aitd_amount, uint64_t fee, std::string &txid)
{
    bool ret = false;
    json json_post;
    json json_params = json::array();
    json json_param = json::object();
    json_param["offline"] = false;
    json_param["secret"] = secret;
    json_param["fee_mult_max"] = fee;
    json json_tx;
    json_tx["TransactionType"] = "Payment";
    json_tx["Account"] = from_address;
    json_tx["Destination"] = to_address;
    json_tx["DestinationTag"] = tag;
    json_tx["Amount"] = std::to_string(aitd_amount);
    json_param["tx_json"] = json_tx;

    json_params.push_back(json_param);
    json_post["params"] = json_params;
    structRpc("submit", json_params, json_post);
    std::cout << json_post.dump(2) << std::endl;

    json json_response;
    ret = rpcProxy("aitdrelay", json_post, json_response);
    if(!ret)
    {
        qWarning("aitd rpc wallet_propose error" );
        return ret;
    }

    std::string status  = json_response["result"]["status"].get<std::string>();
    if (status == "success")
    {
        txid = json_response["result"]["tx_json"]["hash"].get<std::string>();
    }
    else
    {
        qWarning(json_response.dump(2).c_str());
        ret = false;
    }

    return ret;
}

bool Rpc::accountInfo(const std::string &address, double &total)
{
    bool ret = false;
    json json_post;
    json json_params = json::array();
    json json_param = json::object();
    json_param["account"] = address;
    json_params.push_back(json_param);
    json_post["params"] = json_params;
    structRpc("account_info", json_params, json_post);

    json json_response;
    ret = rpcProxy("aitdrelay", json_post, json_response);
    if(!ret)
    {
        qWarning("aitd rpc wallet_propose error" );
        return ret;
    }
    std::string status = json_response["result"]["status"].get<std::string>();
    if(status == "error")
    {
        total =0.0;
    }
    else
    {
        uint64_t balance = std::atol(json_response["result"]["account_data"]["Balance"].get<std::string>().c_str());
        total = (double)balance /1000000.0;
    }
    return ret;
}

bool Rpc::addadress(const std::string &address)
{
    bool ret = false;
    json json_post;
    json json_params;
    json_params.push_back(address);
    json_post["params"] = json_params;

    structRpc("addaddress", json_params, json_post);
    json json_response;
    if ( !rpcProxy(json_post, json_response) )
    {
        qWarning("btc rpc addadress error" );
        return ret;
    }
    return true;
}

bool Rpc::rpcProxy(const std::string& node_relay ,const json &json_post, json &json_response)
{
    json json_relay;
    json json_params;
    json_params.push_back(json_post);
    structRpc(node_relay, json_params, json_relay);
    return CurlPost(proxy_, json_relay, auth_, json_response);
}

bool Rpc::rpcProxy(const json &json_post, json &json_response)
{
    return CurlPost(proxy_, json_post, auth_, json_response);
}







