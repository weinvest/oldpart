#ifndef _OLDPART_ICONNECTION_MANAGER_H
#define _OLDPART_ICONNECTION_MANAGER_H
#include <memory>
#include <unordered_map>

class IConnectionManager
{
public:
    virtual ~IConnectionManager() {}

    void Start(std::shared_ptr<TSocket> pSocket) = 0;
    void Stop(std::shared_ptr<TSocket> pSocket) = 0;
    void StopAll(std::shared_ptr<TSocket> pSocket) = 0;

    bool AddMapping(uint64_t ip, const std::string& name) { return mNameMapping.insert(std::make_pair(ip, name)); }
    const std::string& FindInMapping(uint64_t ip)
    {
        auto itName = mNameMapping.find(ip);
        if(itName == mNameMapping.end())
        {
            return "";
        }

        return itName->second;
    }

private:
    std::unordered_map<uint64_t, std::string> mNameMapping;
};

#endif /* end of include guard: _OLDPART_ICONNECTION_MANAGER_H */
