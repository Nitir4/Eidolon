"""Write portable ZIPs with explicit Unix file modes and symlinks."""

import os
import shutil
import stat
import zipfile


def write_tree(archive, root, executables=()):
    for path in sorted(root.rglob('*')):
        name = path.relative_to(root).as_posix()
        mode = path.lstat().st_mode
        if stat.S_ISLNK(mode):
            member = zipfile.ZipInfo(name)
            member.create_system = 3
            member.external_attr = (stat.S_IFLNK | 0o777) << 16
            archive.writestr(member, os.fsencode(os.readlink(path)))
        elif name in executables:
            member = zipfile.ZipInfo.from_file(path, name)
            member.create_system = 3
            member.external_attr = (stat.S_IFREG | 0o755) << 16
            member.compress_type = archive.compression
            with path.open('rb') as source, archive.open(member, 'w') as destination:
                shutil.copyfileobj(source, destination)
        else:
            archive.write(path, name)
