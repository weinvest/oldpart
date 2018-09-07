#include "ConnectionManager.h"

void ConnectionManager::Start(std::shared_ptr<TSocket> pSocket)
{
    mConnections.insert(pSocket);
    pSocket->Start();
}

void ConnectionManager::Stop(std::shared_ptr<TSocket> pSocket)
{
    mConnections.erase(pSocket);
    pSocket->Stop();
}

void ConnectionManager::StopAll()
{
    for(auto pConnection : mConnections)
    {
        pConnection->Stop();
    }

    mConnections.clear();
}
