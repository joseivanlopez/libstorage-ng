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


#ifndef STORAGE_NTFS_H
#define STORAGE_NTFS_H


#include "storage/Filesystems/Filesystem.h"


namespace storage
{

    class Ntfs : public Filesystem
    {
    public:

	static Ntfs* create(Devicegraph* devicegraph);
	static Ntfs* load(Devicegraph* devicegraph, const xmlNode* node);

	bool supports_label() const override { return true; }
	unsigned int max_labelsize() const override { return 128; }

	bool supports_uuid() const override { return true; }

	static std::vector<Ntfs*> get_all(Devicegraph* devicegraph);
	static std::vector<const Ntfs*> get_all(const Devicegraph* devicegraph);

    public:

	class Impl;

	Impl& get_impl();
	const Impl& get_impl() const;

	virtual Ntfs* clone() const override;

    protected:

	Ntfs(Impl* impl);

    };


    bool is_ntfs(const Device* device);

    Ntfs* to_ntfs(Device* device);
    const Ntfs* to_ntfs(const Device* device);

}

#endif
