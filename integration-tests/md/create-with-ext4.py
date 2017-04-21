#!/usr/bin/python

# requirements: disk /dev/sdb with two empty and unused partitions (sdb1-sdb2)


from sys import exit
from storage import *
from storageitu import *


set_logger(get_logfile_logger())

environment = Environment(False)

storage = Storage(environment)

staging = storage.get_staging()

sdb1 = Partition.find_by_name(staging, "/dev/sdb1")
sdb2 = Partition.find_by_name(staging, "/dev/sdb2")

md = Md.create(staging, "/dev/md0")
md.set_md_level(RAID0)

md.add_device(sdb1)
sdb1.set_id(ID_RAID)

md.add_device(sdb2)
sdb2.set_id(ID_RAID)

ext4 = md.create_blk_filesystem(FsType_EXT4)
ext4.set_label("TEST")

mount_point = ext4.create_mount_point("/test")

print staging

commit(storage)

