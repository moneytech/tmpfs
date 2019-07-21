#!/usr/bin/env python3
import os
import pytest
import tempfile
from os.path import join as pjoin
import stat

MOUNTPOINT = '/tmp/mnt'

@pytest.fixture()
def workdir():
    # TODO switch to TemporaryDirectory and cleanup after implementing rmdir
    return tempfile.mkdtemp(dir=MOUNTPOINT)


def test_create(workdir):
    FILENAME = 'a'
    assert len(os.listdir(workdir)) == 0
    filename = pjoin(workdir, FILENAME)
    with open(filename, 'wb'):
        pass
    assert os.listdir(workdir) == [FILENAME]
    file_stat = os.stat(filename)
    assert stat.S_ISREG(file_stat.st_mode)
