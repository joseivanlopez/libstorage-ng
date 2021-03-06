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


#include "storage/Devices/PartitionableImpl.h"
#include "storage/Devices/PartitionTableImpl.h"
#include "storage/Devices/PartitionImpl.h"
#include "storage/Holders/Subdevice.h"
#include "storage/Devicegraph.h"
#include "storage/SystemInfo/SystemInfo.h"
#include "storage/Utils/StorageTmpl.h"
#include "storage/Utils/XmlFile.h"
#include "storage/Utils/TopologyImpl.h"


namespace storage
{

    using namespace std;


    const vector<string> EnumTraits<PtType>::names({
	"unknown", "loop", "MS-DOS", "GPT", "DASD", "Mac"
    });


    const char* DeviceTraits<PartitionTable>::classname = "PartitionTable";


    PartitionTable::Impl::Impl(const xmlNode* node)
	: Device::Impl(node), read_only(false)
    {
    }


    void
    PartitionTable::Impl::probe_pass_1(Devicegraph* probed, SystemInfo& systeminfo)
    {
	Device::Impl::probe_pass_1(probed, systeminfo);

	const Partitionable* partitionable = get_partitionable();

	const Parted& parted = systeminfo.getParted(partitionable->get_name());

	if (parted.is_implicit())
	    read_only = true;

	for (const Parted::Entry& entry : parted.getEntries())
	{
	    string name = partitionable->get_impl().partition_name(entry.num);
	    Partition* p = create_partition(name, entry.region, entry.type);
	    p->get_impl().probe_pass_1(probed, systeminfo);
	}
    }


    void
    PartitionTable::Impl::save(xmlNode* node) const
    {
	Device::Impl::save(node);

	setChildValueIf(node, "read-only", read_only, read_only);
    }


    void
    PartitionTable::Impl::check() const
    {
	Device::Impl::check();

	const Device* parent = get_single_parent_of_type<const Device>();

	if (!is_disk(parent) && !is_md(parent))
	    ST_THROW(Exception("parent of partition table is not a disk or md"));
    }


    Partition*
    PartitionTable::Impl::create_partition(const string& name, const Region& region, PartitionType type)
    {
	const Region& partitionable_region = get_partitionable()->get_region();
	if (region.get_block_size() != partitionable_region.get_block_size())
	    ST_THROW(DifferentBlockSizes(region.get_block_size(), partitionable_region.get_block_size()));

	const Device* parent = type == PartitionType::LOGICAL ? get_extended() : get_device();

	Partition* partition = Partition::create(get_devicegraph(), name, region, type);
	Subdevice::create(get_devicegraph(), parent, partition);

	partition->get_impl().update_udev_paths_and_ids();

	return partition;
    }


    Partition*
    PartitionTable::Impl::get_partition(const string& name)
    {
	for (Partition* partition : get_partitions())
	{
	    if (partition->get_name() == name)
		return partition;
	}

	ST_THROW(Exception("partition not found"));
    }


    void
    PartitionTable::Impl::delete_partition(Partition* partition)
    {
	partition->remove_descendants();

	get_devicegraph()->remove_device(partition);
    }


    void
    PartitionTable::Impl::delete_partition(const string& name)
    {
	delete_partition(get_partition(name));
    }


    Partitionable*
    PartitionTable::Impl::get_partitionable()
    {
	Devicegraph::Impl::vertex_descriptor vertex = get_devicegraph()->get_impl().parent(get_vertex());

	return to_partitionable(get_devicegraph()->get_impl()[vertex]);
    }


    const Partitionable*
    PartitionTable::Impl::get_partitionable() const
    {
	Devicegraph::Impl::vertex_descriptor vertex = get_devicegraph()->get_impl().parent(get_vertex());

	return to_partitionable(get_devicegraph()->get_impl()[vertex]);
    }


    vector<Partition*>
    PartitionTable::Impl::get_partitions()
    {
	Devicegraph::Impl& devicegraph = get_devicegraph()->get_impl();
	Devicegraph::Impl::vertex_descriptor vertex = get_vertex();

	return devicegraph.filter_devices_of_type<Partition>(devicegraph.children(vertex),
							     compare_by_number);
    }


    vector<const Partition*>
    PartitionTable::Impl::get_partitions() const
    {
	const Devicegraph::Impl& devicegraph = get_devicegraph()->get_impl();
	Devicegraph::Impl::vertex_descriptor vertex = get_vertex();

	return devicegraph.filter_devices_of_type<Partition>(devicegraph.children(vertex),
							     compare_by_number);
    }


    bool
    PartitionTable::Impl::equal(const Device::Impl& rhs_base) const
    {
	const Impl& rhs = dynamic_cast<const Impl&>(rhs_base);

	if (!Device::Impl::equal(rhs))
	    return false;

	return read_only == rhs.read_only;
    }


    void
    PartitionTable::Impl::log_diff(std::ostream& log, const Device::Impl& rhs_base) const
    {
	const Impl& rhs = dynamic_cast<const Impl&>(rhs_base);

	Device::Impl::log_diff(log, rhs);

	storage::log_diff(log, "read-only", read_only, rhs.read_only);
    }


    void
    PartitionTable::Impl::print(std::ostream& out) const
    {
	Device::Impl::print(out);
    }


    unsigned int
    PartitionTable::Impl::num_primary() const
    {
	vector<const Partition*> partitions = get_partitions();
	return count_if(partitions.begin(), partitions.end(), [](const Partition* partition) {
	    return partition->get_type() == PartitionType::PRIMARY;
	});
    }


    const Partition*
    PartitionTable::Impl::get_extended() const
    {
	ST_THROW(Exception("has no extended partition"));
    }


    vector<PartitionSlot>
    PartitionTable::Impl::get_unused_partition_slots(AlignPolicy align_policy) const
    {
	const Partitionable* partitionable = get_partitionable();
	const Topology& topology = partitionable->get_topology();

	bool is_primary_possible = num_primary() + (has_extended() ? 1 : 0) < max_primary();
	bool is_extended_possible = is_primary_possible && extended_possible() && !has_extended();
	bool is_logical_possible = has_extended() && num_logical() < (max_logical() - max_primary());

	vector<PartitionSlot> slots;

	vector<const Partition*> partitions = get_partitions();
	sort(partitions.begin(), partitions.end(), compare_by_number);

	if (true)
	{
	    PartitionSlot slot;

	    if (true /* label != "dasd" */)
	    {
		vector<const Partition*>::const_iterator it = partitions.begin();
		unsigned start = 1; // label != "mac" ? 1 : 2;
		while (it != partitions.end() && (*it)->get_number() <= start &&
		       (*it)->get_number() <= max_primary())
		{
		    if ((*it)->get_number() == start)
			++start;
		    /*
		    if (label == "sun" && start == 3)
		        ++start;
		    */
		    ++it;
		}
		slot.nr = start;
	    }
	    else
	    {
		slot.nr = 1;
	    }

	    slot.name = partitionable->get_impl().partition_name(slot.nr);

	    slot.primary_slot = true;
	    slot.primary_possible = is_primary_possible;
	    slot.extended_slot = true;
	    slot.extended_possible = is_extended_possible;
	    slot.logical_slot = false;
	    slot.logical_possible = false;

	    vector<Region> used_regions;
	    for (const Partition* partition : partitions)
	    {
		if (partition->get_type() != PartitionType::LOGICAL)
		    used_regions.push_back(partition->get_region());
	    }

	    Region usable_region = get_usable_region();
	    for (const Region& unused_region : usable_region.unused_regions(used_regions))
	    {
		slot.region = unused_region;
		if (topology.get_impl().align_region_in_place(slot.region, align_policy))
		{
		    slots.push_back(slot);

		    /*
		      if (label == "dasd")
		      {
		      slot.nr++;
		      slot.device = getPartDevice(slot.nr);
		      }
		    */
		}
	    }
	}

	if (has_extended())
	{
	    PartitionSlot slot;

	    slot.nr = max_primary() + num_logical() + 1;
	    slot.name = partitionable->get_impl().partition_name(slot.nr);

	    slot.primary_slot = false;
	    slot.primary_possible = false;
	    slot.extended_slot = false;
	    slot.extended_possible = false;
	    slot.logical_slot = true;
	    slot.logical_possible = is_logical_possible;

	    vector<Region> used_regions;
	    for (const Partition* partition : partitions)
	    {
		if (partition->get_type() == PartitionType::LOGICAL)
		    used_regions.push_back(partition->get_region());
	    }

	    const Region& extended_region = get_extended()->get_region();
	    for (const Region& unused_region : extended_region.unused_regions(used_regions))
	    {
		// one sector is needed for the EBR, see
		// https://en.wikipedia.org/wiki/Extended_boot_record

		if (unused_region.get_length() <= 1)
		    continue;

		Region adjusted_region = unused_region;
		adjusted_region.adjust_start(+1);
		adjusted_region.adjust_length(-1);

		slot.region = adjusted_region;
		if (topology.get_impl().align_region_in_place(slot.region, align_policy))
		{
		    slots.push_back(slot);
		}
	    }
	}

	y2deb("slots:" << slots);

	return slots;
    }


    Region
    PartitionTable::Impl::align(const Region& region, AlignPolicy align_policy) const
    {
	return get_partitionable()->get_topology().align(region, align_policy);
    }


    /**
     * Returns input sorted by the default comparison of the return value of
     * the key function fnc of each value in input.
     */
    template<typename ValueType, typename KeyType>
    vector<ValueType>
    sort_by_key(const vector<ValueType>& input, std::function<KeyType(ValueType)> fnc)
    {
	typedef typename vector<ValueType>::size_type size_type;

	map<KeyType, size_type> tmp;

	for (size_type i = 0; i < input.size(); ++i)
	    tmp[fnc(input[i])] = i;

	vector<ValueType> output;

	for (const auto& t : tmp)
	    output.push_back(input[t.second]);

	return output;
    }


    void
    PartitionTable::Impl::run_dependency_manager(Actiongraph::Impl& actiongraph)
    {
	// To speed up things this fuctions finds the actions for all
	// partition tables. Going through the actions for every partition
	// table individually is slow.

	struct AllActions
	{
	    vector<Actiongraph::Impl::vertex_descriptor> shrink_actions;
	    vector<Actiongraph::Impl::vertex_descriptor> delete_actions;
	    vector<Actiongraph::Impl::vertex_descriptor> rename_in_actions;
	    vector<Actiongraph::Impl::vertex_descriptor> create_actions;
	    vector<Actiongraph::Impl::vertex_descriptor> grow_actions;
	};

	// Build map with all actions for all partition tables.

	map<sid_t, AllActions> all_actions_per_partition_table;

	for (Actiongraph::Impl::vertex_descriptor vertex : actiongraph.vertices())
	{
	    const Action::Base* action = actiongraph[vertex];

	    const Action::Create* create_action = dynamic_cast<const Action::Create*>(action);
	    if (create_action)
	    {
		const Device* device = create_action->get_device(actiongraph);
		if (is_partition(device))
		{
		    const Partition* partition = to_partition(device);
		    sid_t sid = partition->get_partition_table()->get_sid();

		    all_actions_per_partition_table[sid].create_actions.push_back(vertex);
		}
	    }

	    const Action::Delete* delete_action = dynamic_cast<const Action::Delete*>(action);
            if (delete_action)
            {
                const Device* device = delete_action->get_device(actiongraph);
                if (is_partition(device))
                {
                    const Partition* partition = to_partition(device);
                    sid_t sid = partition->get_partition_table()->get_sid();

                    all_actions_per_partition_table[sid].delete_actions.push_back(vertex);
                }
            }

	    const Action::Resize* resize_action = dynamic_cast<const Action::Resize*>(action);
	    if (resize_action)
	    {
		const Device* device = resize_action->get_device(actiongraph, RHS);
		if (is_partition(device))
		{
		    const Partition* partition = to_partition(device);
		    sid_t sid = partition->get_partition_table()->get_sid();

		    if (resize_action->resize_mode == ResizeMode::GROW)
			all_actions_per_partition_table[sid].grow_actions.push_back(vertex);
		    else
			all_actions_per_partition_table[sid].shrink_actions.push_back(vertex);
		}
	    }

	    const Action::RenameIn* rename_in_action = dynamic_cast<const Action::RenameIn*>(action);
	    if (rename_in_action)
	    {
		const Partition* partition = to_partition(rename_in_action->get_renamed_blk_device(actiongraph, RHS));
		sid_t sid = partition->get_partition_table()->get_sid();

		all_actions_per_partition_table[sid].rename_in_actions.push_back(vertex);
	    }
	}

	// Some functions used for sorting actions by partition number.

        const Devicegraph* devicegraph_rhs = actiongraph.get_devicegraph(RHS);
        const Devicegraph* devicegraph_lhs = actiongraph.get_devicegraph(LHS);

	std::function<unsigned int(Actiongraph::Impl::vertex_descriptor)> key_fnc1 =
	    [&actiongraph, &devicegraph_lhs](Actiongraph::Impl::vertex_descriptor vertex) {
	    const Action::Base* action = actiongraph[vertex];
	    const Partition* partition = to_partition(devicegraph_lhs->find_device(action->sid));
	    return partition->get_number();
	};

	std::function<unsigned int(Actiongraph::Impl::vertex_descriptor)> key_fnc2 =
	    [&actiongraph, &devicegraph_lhs](Actiongraph::Impl::vertex_descriptor vertex) {
	    const Action::RenameIn* action = dynamic_cast<const Action::RenameIn*>(actiongraph[vertex]);
	    const Partition* partition = to_partition(action->get_renamed_blk_device(actiongraph, RHS));
	    return partition->get_number();
	};

	std::function<unsigned int(Actiongraph::Impl::vertex_descriptor)> key_fnc3 =
	    [&actiongraph, &devicegraph_rhs](Actiongraph::Impl::vertex_descriptor vertex) {
	    const Action::Base* action = actiongraph[vertex];
	    const Partition* partition = to_partition(devicegraph_rhs->find_device(action->sid));
	    return partition->get_number();
	};

	// Iterate over all partition tables with actions and build chain of
	// dependencies.

	for (auto& tmp : all_actions_per_partition_table)
	{
	    AllActions& all_actions = tmp.second;

	    vector<Actiongraph::Impl::vertex_descriptor> actions;

	    all_actions.shrink_actions = sort_by_key(all_actions.shrink_actions, key_fnc1);
	    actions.insert(actions.end(), all_actions.shrink_actions.begin(),
			   all_actions.shrink_actions.end());

	    all_actions.delete_actions = sort_by_key(all_actions.delete_actions, key_fnc1);
	    actions.insert(actions.end(), all_actions.delete_actions.rbegin(),
			   all_actions.delete_actions.rend());

	    all_actions.rename_in_actions = sort_by_key(all_actions.rename_in_actions, key_fnc2);
	    actions.insert(actions.end(), all_actions.rename_in_actions.begin(),
			   all_actions.rename_in_actions.end());

	    all_actions.create_actions = sort_by_key(all_actions.create_actions, key_fnc3);
	    actions.insert(actions.end(), all_actions.create_actions.begin(),
			   all_actions.create_actions.end());

	    all_actions.grow_actions = sort_by_key(all_actions.grow_actions, key_fnc3);
	    actions.insert(actions.end(), all_actions.grow_actions.begin(),
			   all_actions.grow_actions.end());

	    actiongraph.add_chain(actions);
	}
    }


    std::ostream&
    operator<<(std::ostream& s, const PartitionSlot& partition_slot)
    {
	s << "region:" << partition_slot.region << " nr:" << partition_slot.nr
	  << " name:" << partition_slot.name
	  << " primary_slot:" << partition_slot.primary_slot
	  << " primary_possible:" << partition_slot.primary_possible
	  << " extended_slot:" << partition_slot.extended_slot
	  << " extended_possible:" << partition_slot.extended_possible
	  << " logical_slot:" << partition_slot.logical_slot
	  << " logical_possible:" << partition_slot.logical_possible;

	return s;
    }

}
