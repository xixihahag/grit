// #pragma once
#include <string>

namespace grit {
class Mhash
{
  private:
    struct Info
    {
        Info(std::string k, int l)
            : key(k)
            , lsn(l)
            , next(nullptr)
        {}
        std::string key;
        int lsn;
        struct Info *next;

        bool operator==(const struct Info *r)
        {
            return key == r->key && lsn == r->lsn;
        }
    };

    int getPos(struct Info *info)
    {
        return std::hash<std::string>()(info->key) % primeNum[numPoint_];
    }

    void insert(struct Info *info)
    {
        int pos = getPos(info);
        if (head[pos] == nullptr)
            head[pos] = info;
        else {
            struct Info *t = head[pos];
            while (t->next)
                t = t->next;
            t->next = info;
        }

        if (++sum_ == primeNum[numPoint_] - 1) rehash();
    }

    void rehash()
    {
        ++numPoint_;

        struct Info **t = head;
        head =
            (struct Info **) calloc(primeNum[numPoint_], sizeof(struct Info *));

        for (int i = 0; i < primeNum[numPoint_ - 1]; i++)
            if (t[i] != nullptr) insert(t[i]);

        free(t);
    }

    // 用不到
    // struct Info *exist(struct Info *info)
    // {
    //     int pos = getPos(info);
    //     return head[pos];
    // }

    // void del(struct Info *info)
    // {
    //     int pos = getPos(info);

    //     struct Info *t = head[pos], *pre = nullptr;
    //     while (t != info) {
    //         pre = t;
    //         t = t->next;
    //     }

    //     pre->next = t->next;

    //     delete info;
    // };

    struct Info **head;

    int primeNum[17] =
        {23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97};

    int numPoint_;
    int sum_;

  public:
    Mhash()
        : numPoint_(0)
        , sum_(0)
    {
        head =
            (struct Info **) calloc(primeNum[numPoint_], sizeof(struct Info *));
    }

    int getPos(std::string key)
    {
        return std::hash<std::string>()(key) % primeNum[numPoint_];
    }

    void insert(std::string k, int l)
    {
        struct Info *t = new struct Info(k, l);
        return insert(t);
    }

    // 根据key来判断是否存在，不存在则返回nullptr
    bool exist(std::string key)
    {
        int pos = getPos(key);
        if (head[pos] != nullptr) return true;
        return false;
    }

    // 删除掉 相同key的情况下，lsn比当前传入值小的项
    void delLessThan(std::string k, int lsn)
    {
        int pos = getPos(k);
        struct Info *info = head[pos];
        while (info->lsn < lsn) {
            struct Info *t = info;
            info = info->next;
            delete t;
            --sum_;
        }

        head[pos] = info;
    }
};

} // namespace grit