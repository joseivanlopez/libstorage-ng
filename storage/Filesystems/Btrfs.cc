/*
 * Copyright (c) 2015 Novell, Inc.
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


#include "storage/Filesystems/BtrfsImpl.h"
#include "storage/Devicegraph.h"
#include "storage/Action.h"


namespace storage
{

    using namespace std;


    Btrfs*
    Btrfs::create(Devicegraph* devicegraph)
    {
	Btrfs* ret = new Btrfs(new Btrfs::Impl());
	ret->Device::create(devicegraph);
	return ret;
    }


    Btrfs*
    Btrfs::load(Devicegraph* devicegraph, const xmlNode* node)
    {
	Btrfs* ret = new Btrfs(new Btrfs::Impl(node));
	ret->Device::load(devicegraph);
	return ret;
    }


    Btrfs::Btrfs(Impl* impl)
	: Filesystem(impl)
    {
    }


    Btrfs*
    Btrfs::clone() const
    {
	return new Btrfs(get_impl().clone());
    }


    Btrfs::Impl&
    Btrfs::get_impl()
    {
	return dynamic_cast<Impl&>(Device::get_impl());
    }


    const Btrfs::Impl&
    Btrfs::get_impl() const
    {
	return dynamic_cast<const Impl&>(Device::get_impl());
    }


    bool
    is_btrfs(const Device* device)
    {
	return is_device_of_type<const Btrfs>(device);
    }


    Btrfs*
    to_btrfs(Device* device)
    {
	return to_device_of_type<Btrfs>(device);
    }


    const Btrfs*
    to_btrfs(const Device* device)
    {
	return to_device_of_type<const Btrfs>(device);
    }

}
