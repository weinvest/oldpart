#ifndef _OLDPART_ICONNECTION_MANAGER_H
#define _OLDPART_ICONNECTION_MANAGER_H
#include <memory>
#include <unordered_map>

class TSocket;
class IConnectionManager
{
public:
    virtual ~IConnectionManager() {}

    virtual void Start(std::shared_ptr<TSocket> pSocket) = 0;
    virtual void Stop(std::shared_ptr<TSocket> pSocket) = 0;
    virtual void StopAll() = 0;

    bool AddMapping(int64_t ip, int32_t port, const std::string& name)
    {
         return mNameMapping.insert(std::make_pair(GetCompositeIP(ip, port), name)).second;
    }

    const std::string& FindInMapping(int32_t ip, int32_t port)
    {
        static std::string EMPTY("");
        int64_t cip = GetCompositeIP(ip, port);
        auto itName = mNameMapping.find(cip);
        if(itName == mNameMapping.end())
        {
            return EMPTY;
        }

        return itName->second;
    }

private:
    int32_t GetCompositeIP(int32_t ip, int32_t port)
    {
        int64_t cip = ip;
        cip <<= 32;
        cip += port;
        return cip;
    }

    std::unordered_map<uint64_t, std::string> mNameMapping;
};

#endif /* end of include guard: _OLDPART_ICONNECTION_MANAGER_H */
