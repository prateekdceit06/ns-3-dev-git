/*
 * Copyright (c) 2008-2009 Strasbourg University
 *               2013 Universita' di Firenze
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Sebastien Vincent <vincent@clarinet.u-strasbg.fr>
 *         Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */

#include "ipv6-interface-container.h"

#include "ipv6-static-routing-helper.h"

#include "ns3/names.h"
#include "ns3/node-list.h"

namespace ns3
{

Ipv6InterfaceContainer::Ipv6InterfaceContainer()
{
}

Ipv6InterfaceContainer::Iterator
Ipv6InterfaceContainer::Begin() const
{
    return m_interfaces.begin();
}

Ipv6InterfaceContainer::Iterator
Ipv6InterfaceContainer::End() const
{
    return m_interfaces.end();
}

uint32_t
Ipv6InterfaceContainer::GetN() const
{
    return m_interfaces.size();
}

uint32_t
Ipv6InterfaceContainer::GetInterfaceIndex(uint32_t i) const
{
    return m_interfaces[i].second;
}

Ipv6Address
Ipv6InterfaceContainer::GetAddress(uint32_t i, uint32_t j) const
{
    Ptr<Ipv6> ipv6 = m_interfaces[i].first;
    uint32_t interface = m_interfaces[i].second;
    return ipv6->GetAddress(interface, j).GetAddress();
}

void
Ipv6InterfaceContainer::Add(Ptr<Ipv6> ipv6, uint32_t interface)
{
    m_interfaces.emplace_back(ipv6, interface);
}

void
Ipv6InterfaceContainer::Add(std::string ipv6Name, uint32_t interface)
{
    Ptr<Ipv6> ipv6 = Names::Find<Ipv6>(ipv6Name);
    m_interfaces.emplace_back(ipv6, interface);
}

void
Ipv6InterfaceContainer::Add(const Ipv6InterfaceContainer& c)
{
    for (auto it = c.m_interfaces.begin(); it != c.m_interfaces.end(); it++)
    {
        m_interfaces.push_back(*it);
    }
}

std::pair<Ptr<Ipv6>, uint32_t>
Ipv6InterfaceContainer::Get(uint32_t i) const
{
    return m_interfaces[i];
}

void
Ipv6InterfaceContainer::SetForwarding(uint32_t i, bool router)
{
    Ptr<Ipv6> ipv6 = m_interfaces[i].first;
    ipv6->SetForwarding(m_interfaces[i].second, router);
}

void
Ipv6InterfaceContainer::SetDefaultRouteInAllNodes(uint32_t router)
{
    Ptr<Ipv6> ipv6 = m_interfaces[router].first;
    uint32_t other;

    Ipv6Address routerAddress = GetLinkLocalAddress(router);
    NS_ASSERT_MSG(routerAddress != Ipv6Address::GetAny(),
                  "No link-local address found on router, aborting");

    for (other = 0; other < m_interfaces.size(); other++)
    {
        if (other != router)
        {
            Ptr<Ipv6StaticRouting> routing = nullptr;
            Ipv6StaticRoutingHelper routingHelper;

            ipv6 = m_interfaces[other].first;
            routing = routingHelper.GetStaticRouting(ipv6);
            NS_ASSERT_MSG(
                routing,
                "Default router setup failed because no Ipv6StaticRouting was found on the node.");
            routing->SetDefaultRoute(routerAddress, m_interfaces[other].second);
        }
    }
}

void
Ipv6InterfaceContainer::SetDefaultRouteInAllNodes(Ipv6Address routerAddress)
{
    uint32_t routerIndex = 0;
    bool found = false;
    for (uint32_t index = 0; index < m_interfaces.size(); index++)
    {
        Ptr<Ipv6> ipv6 = m_interfaces[index].first;
        for (uint32_t i = 0; i < ipv6->GetNAddresses(m_interfaces[index].second); i++)
        {
            Ipv6Address addr = ipv6->GetAddress(m_interfaces[index].second, i).GetAddress();
            if (addr == routerAddress)
            {
                routerIndex = index;
                found = true;
                break;
            }
        }
        if (found)
        {
            break;
        }
    }
    NS_ASSERT_MSG(found != true, "No such address in the interfaces. Aborting.");

    for (uint32_t other = 0; other < m_interfaces.size(); other++)
    {
        if (other != routerIndex)
        {
            Ptr<Ipv6StaticRouting> routing = nullptr;
            Ipv6StaticRoutingHelper routingHelper;

            Ptr<Ipv6> ipv6 = m_interfaces[other].first;
            routing = routingHelper.GetStaticRouting(ipv6);
            NS_ASSERT_MSG(
                routing,
                "Default router setup failed because no Ipv6StaticRouting was found on the node.");
            routing->SetDefaultRoute(routerAddress, m_interfaces[other].second);
        }
    }
}

void
Ipv6InterfaceContainer::SetDefaultRoute(uint32_t i, uint32_t router)
{
    NS_ASSERT_MSG(i != router,
                  "A node shouldn't set itself as the default router, isn't it? Aborting.");

    Ptr<Ipv6> ipv6 = m_interfaces[i].first;

    Ipv6Address routerAddress = GetLinkLocalAddress(router);
    NS_ASSERT_MSG(routerAddress != Ipv6Address::GetAny(),
                  "No link-local address found on router, aborting");

    Ptr<Ipv6StaticRouting> routing = nullptr;
    Ipv6StaticRoutingHelper routingHelper;

    routing = routingHelper.GetStaticRouting(ipv6);
    NS_ASSERT_MSG(
        routing,
        "Default router setup failed because no Ipv6StaticRouting was found on the node.");
    routing->SetDefaultRoute(routerAddress, m_interfaces[i].second);
}

void
Ipv6InterfaceContainer::SetDefaultRoute(uint32_t i, Ipv6Address routerAddr)
{
    uint32_t routerIndex = 0;
    bool found = false;
    for (uint32_t index = 0; index < m_interfaces.size(); index++)
    {
        Ptr<Ipv6> ipv6 = m_interfaces[index].first;
        for (uint32_t i = 0; i < ipv6->GetNAddresses(m_interfaces[index].second); i++)
        {
            Ipv6Address addr = ipv6->GetAddress(m_interfaces[index].second, i).GetAddress();
            if (addr == routerAddr)
            {
                routerIndex = index;
                found = true;
                break;
            }
        }
        if (found)
        {
            break;
        }
    }
    NS_ASSERT_MSG(found != true, "No such address in the interfaces. Aborting.");

    NS_ASSERT_MSG(i != routerIndex,
                  "A node shouldn't set itself as the default router, isn't it? Aborting.");

    Ptr<Ipv6> ipv6 = m_interfaces[i].first;
    Ipv6Address routerLinkLocalAddress = GetLinkLocalAddress(routerIndex);
    Ptr<Ipv6StaticRouting> routing = nullptr;
    Ipv6StaticRoutingHelper routingHelper;

    routing = routingHelper.GetStaticRouting(ipv6);
    NS_ASSERT_MSG(
        routing,
        "Default router setup failed because no Ipv6StaticRouting was found on the node.");
    routing->SetDefaultRoute(routerLinkLocalAddress, m_interfaces[i].second);
}

Ipv6Address
Ipv6InterfaceContainer::GetLinkLocalAddress(uint32_t index)
{
    Ptr<Ipv6> ipv6 = m_interfaces[index].first;
    for (uint32_t i = 0; i < ipv6->GetNAddresses(m_interfaces[index].second); i++)
    {
        Ipv6InterfaceAddress iAddress;
        iAddress = ipv6->GetAddress(m_interfaces[index].second, i);
        if (iAddress.GetScope() == Ipv6InterfaceAddress::LINKLOCAL)
        {
            return iAddress.GetAddress();
        }
    }
    return Ipv6Address::GetAny();
}

Ipv6Address
Ipv6InterfaceContainer::GetLinkLocalAddress(Ipv6Address address)
{
    if (address.IsLinkLocal())
    {
        return address;
    }

    uint32_t nodeIndex = 0;
    bool found = false;
    for (uint32_t index = 0; index < m_interfaces.size(); index++)
    {
        Ptr<Ipv6> ipv6 = m_interfaces[index].first;
        for (uint32_t i = 0; i < ipv6->GetNAddresses(m_interfaces[index].second); i++)
        {
            Ipv6Address addr = ipv6->GetAddress(m_interfaces[index].second, i).GetAddress();
            if (addr == address)
            {
                nodeIndex = index;
                found = true;
                break;
            }
        }
        if (found)
        {
            break;
        }
    }
    NS_ASSERT_MSG(found != true, "No such address in the interfaces. Aborting.");

    Ptr<Ipv6> ipv6 = m_interfaces[nodeIndex].first;
    for (uint32_t i = 0; i < ipv6->GetNAddresses(m_interfaces[nodeIndex].second); i++)
    {
        Ipv6InterfaceAddress iAddress;
        iAddress = ipv6->GetAddress(m_interfaces[nodeIndex].second, i);
        if (iAddress.GetScope() == Ipv6InterfaceAddress::LINKLOCAL)
        {
            return iAddress.GetAddress();
        }
    }
    return Ipv6Address::GetAny();
}

} /* namespace ns3 */
