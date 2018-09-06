#ifndef _OLDPART_CONNECTION_MANAGER_H
#define _OLDPART_CONNECTION_MANAGER_H
#include <set>
#include <boost/noncopyable.hpp>
#include "common/TSocket.h"
#include "common/IConnectionManager.h"
class ConnectionManager: public IConnectionManager, private boost::noncopyable
{
public:
    void Start(std::shared_ptr<TSocket> pSocket) override;
    void Stop(std::shared_ptr<TSocket> pSocket) override;
    void StopAll() override;

private:
    std::set<std::shared_ptr<TSocket>> mConnections;
};

#endif /* end of include guard: _OLDPART_CONNECTION_MANAGER_H */
