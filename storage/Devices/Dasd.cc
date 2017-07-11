/*
 * Copyright (c) [2014-2015] Novell, Inc.
 * Copyright (c) [2016-2017] SUSE LLC
 *
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, contact Novell, Inc.
 *
 * To contact Novell about this file by physical or electronic mail, you may
 * find current contact information at www.novell.com.
 */


#include <boost/algorithm/string.hpp>

#include "storage/Utils/StorageDefines.h"
#include "storage/Devices/DasdImpl.h"
#include "storage/Devicegraph.h"
#include "storage/Utils/Enum.h"


namespace storage
{

    using namespace std;


    Dasd*
    Dasd::create(Devicegraph* devicegraph, const string& name)
    {
	Dasd* ret = new Dasd(new Dasd::Impl(name));
	ret->Device::create(devicegraph);
	return ret;
    }


    Dasd*
    Dasd::create(Devicegraph* devicegraph, const string& name, const Region& region)
    {
	Dasd* ret = new Dasd(new Dasd::Impl(name, region));
	ret->Device::create(devicegraph);
	return ret;
    }


    Dasd*
    Dasd::create(Devicegraph* devicegraph, const string& name, unsigned long long size)
    {
	Dasd* ret = new Dasd(new Dasd::Impl(name, Region(0, size / 512, 512)));
	ret->Device::create(devicegraph);
	return ret;
    }


    Dasd*
    Dasd::load(Devicegraph* devicegraph, const xmlNode* node)
    {
	Dasd* ret = new Dasd(new Dasd::Impl(node));
	ret->Device::load(devicegraph);
	return ret;
    }


    Dasd::Dasd(Impl* impl)
	: Partitionable(impl)
    {
    }


    Dasd*
    Dasd::clone() const
    {
	return new Dasd(get_impl().clone());
    }


    Dasd::Impl&
    Dasd::get_impl()
    {
	return dynamic_cast<Impl&>(Device::get_impl());
    }


    const Dasd::Impl&
    Dasd::get_impl() const
    {
	return dynamic_cast<const Impl&>(Device::get_impl());
    }


    Dasd*
    Dasd::find_by_name(Devicegraph* devicegraph, const string& name)
    {
	return to_dasd(BlkDevice::find_by_name(devicegraph, name));
    }


    const Dasd*
    Dasd::find_by_name(const Devicegraph* devicegraph, const string& name)
    {
	return to_dasd(BlkDevice::find_by_name(devicegraph, name));
    }


    bool
    Dasd::is_rotational() const
    {
	return get_impl().is_rotational();
    }


    DasdType
    Dasd::get_dasd_type() const
    {
	return get_impl().get_dasd_type();
    }


    void
    Dasd::set_dasd_type(DasdType dasd_type)
    {
	get_impl().set_dasd_type(dasd_type);
    }


    DasdFormat
    Dasd::get_dasd_format() const
    {
	return get_impl().get_dasd_format();
    }


    void
    Dasd::set_dasd_format(DasdFormat dasd_format)
    {
	get_impl().set_dasd_format(dasd_format);
    }


    vector<Dasd*>
    Dasd::get_all(Devicegraph* devicegraph)
    {
	return devicegraph->get_impl().get_devices_of_type<Dasd>(compare_by_name);
    }


    vector<const Dasd*>
    Dasd::get_all(const Devicegraph* devicegraph)
    {
	return devicegraph->get_impl().get_devices_of_type<const Dasd>(compare_by_name);
    }


    bool
    is_dasd(const Device* device)
    {
	return is_device_of_type<const Dasd>(device);
    }


    Dasd*
    to_dasd(Device* device)
    {
	return to_device_of_type<Dasd>(device);
    }


    const Dasd*
    to_dasd(const Device* device)
    {
	return to_device_of_type<const Dasd>(device);
    }

}
