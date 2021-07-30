#ifndef HDWALLET_H
#define HDWALLET_H
#include <string>
#include <vector>

class HDwallet
{
public:
    HDwallet()
    {}
    ~HDwallet()
    {}
    struct HdInfo
    {
      std::string mnemonic;
      std::string passwd;
      std::string seed;
    };

    enum CoinType
    {
        BTC = 0,
        ETH = 1,
        AITD = 2
    };

    struct Account
    {
        std::string address;
        std::string privkey;
    };


    struct Transfer
    {
        std::string from;
        std::string to;
        std::string amount;
        std::string fee;
        std::string contract;
        size_t decimal;
        bool is_token = false;
        CoinType coin_type;
    };

public:
    bool createMasterKey(const std::string& password);

    bool createChildKey(size_t chain_code);

    bool importMasterKey(const std::string& mnemonic, const std::string& passwd, bool reset = false);

    bool getHdInfo(HdInfo& hd_info);

    bool getAccount(Account& account, CoinType coin_type, const size_t& chain_code, const size_t& index);

    bool transferCoin(const Transfer& transfer, std::string& txid);

protected:
    bool transferBtc(const Transfer& transfer, std::string& txid);

    bool transferEth(const Transfer& transfer, std::string& txid);

protected:
    HdInfo hd_info_;
    std::vector<size_t> vect_child_code_;
    bool init_hd_ = false;
};

#endif // HDWALLET_H
