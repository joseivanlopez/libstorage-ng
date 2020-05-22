/*
 * Copyright (c) [2014-2015] Novell, Inc.
 * Copyright (c) [2017-2020] SUSE LLC
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


#ifndef STORAGE_HOLDER_H
#define STORAGE_HOLDER_H


#include <libxml/tree.h>
#include <memory>
#include <boost/noncopyable.hpp>

#include "storage/Devices/Device.h"
#include "storage/Utils/Swig.h"


namespace storage
{
    class Devicegraph;
    class Device;


    class HolderHasWrongType : public Exception
    {
    public:

	HolderHasWrongType(const char* seen, const char* expected);
    };


    /**
     * An abstract base class for storage holders. Storage holders are
     * edges in the devicegraph.
     *
     * @see Devicegraph
     */
    class Holder : private boost::noncopyable
    {
    public:

	virtual ~Holder();

	Device* get_source();
	const Device* get_source() const;

	/**
	 * Return the storage id (sid) of the source of the holder.
	 *
	 * @see sid_t
	 */
	sid_t get_source_sid() const;

	Device* get_target();
	const Device* get_target() const;

	/**
	 * Return the storage id (sid) of the target of the holder.
	 *
	 * @see sid_t
	 */
	sid_t get_target_sid() const;

	bool operator==(const Holder& rhs) const;
	bool operator!=(const Holder& rhs) const;

	/**
	 * Copies the holder to the devicegraph. Does not copy devices. The
	 * purpose of the function is to restore parts of the probed
	 * devicegraph in the staging devicegraph that were previously
	 * deleted.
	 *
	 * Source and target devices must already exist in devicegraph. Holder
	 * must not exist in devicegraph.
	 *
	 * @see Device::copy_to_devicegraph()
	 *
	 * @throw Exception
	 */
	Holder* copy_to_devicegraph(Devicegraph* devicegraph) const;

	/**
	 * Checks if the holder exists in the devicegraph.
	 */
	bool exists_in_devicegraph(const Devicegraph* devicegraph) const;

	/**
	 * Checks if the holder exists in the probed devicegraph.
	 */
	bool exists_in_probed() const;

	/**
	 * Checks if the holder exists in the staging devicegraph.
	 */
	bool exists_in_staging() const;

	/**
	 * Checks if the holder exists in the system devicegraph.
	 */
	bool exists_in_system() const;

	/**
	 * Return the userdata of the holder.
	 */
	const std::map<std::string, std::string>& get_userdata() const;

	/**
	 * Set the userdata of the holder.
	 */
	void set_userdata(const std::map<std::string, std::string>& userdata);

    public:

	class Impl;

	Impl& get_impl() { return *impl; }
	const Impl& get_impl() const { return *impl; }

	virtual Holder* clone() const = 0;

	void save(xmlNode* node) const ST_DEPRECATED;

	friend std::ostream& operator<<(std::ostream& out, const Holder& holder);

    protected:

	Holder(Impl* impl);

	/**
	 * Create a holder between source and target in the devicegraph. Duplicate holders
	 * of the same type are not allowed.
	 *
	 * @throw HolderAlreadyExists
	 */
	void create(Devicegraph* devicegraph, const Device* source, const Device* target);

	void load(Devicegraph* devicegraph, const xmlNode* node);

    private:

	void add_to_devicegraph(Devicegraph* devicegraph, const Device* source,
				const Device* target);

	const std::unique_ptr<Impl> impl;

    };

}

#endif
