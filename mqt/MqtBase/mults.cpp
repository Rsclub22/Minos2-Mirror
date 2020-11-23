/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"

#include "MultsImpl.h"

MultLists *MultLists::getMultLists()
{
   return MultListsImpl::getMultLists();
}
MultLists::MultLists()
{}
MultLists::~MultLists()
{}


QString CountryEntry::getContinent() const
{
    return continent;
}

int CountryEntry::getITUZone() const
{
    return ITUZone;
}

int CountryEntry::getCQZone() const
{
    return CQZone;
}

QString CountryEntry::getBasePrefix() const
{
    return basePrefix;
}

QSharedPointer<CountryEntry> CountrySynonym::getCountry() const
{
    return country;
}

QString CountrySynonym::getBasePrefix() const
{
    return country->getBasePrefix();
}

QString CountrySynonym::getRealName() const
{
    return country->getRealName();
}

QString CountrySynonym::getContinent() const
{
    return continent;
}

int CountrySynonym::getITUZone() const
{
    return ITUZone;
}

int CountrySynonym::getCQZone() const
{
    return CQZone;
}

Locator CountrySynonym::getCentral() const
{
    return country->getCentral();
}

QString CountrySynonym::getSynPrefix() const
{
    return synPrefix;
}

QString MultEntry::getRealName() const
{
    return realName;
}

Locator MultEntry::getCentral() const
{
    return central;
}
