#!/usr/bin/env python3
import os
import pytest
import tempfile
from os.path import join as pjoin
import stat

MOUNTPOINT = '/tmp/mnt'
FILENAME = 'file'


@pytest.fixture()
def workdir():
    # TODO switch to TemporaryDirectory and cleanup after implementing rmdir
    return tempfile.mkdtemp(dir=MOUNTPOINT)


def test_create(workdir):
    assert len(os.listdir(workdir)) == 0
    filename = pjoin(workdir, FILENAME)
    with open(filename, 'wb'):
        pass
    assert os.listdir(workdir) == [FILENAME]
    file_stat = os.stat(filename)
    assert stat.S_ISREG(file_stat.st_mode)


def test_read_noent(workdir):
    with pytest.raises(FileNotFoundError):
        open(pjoin(workdir, FILENAME))


def test_read_isdir(workdir):
    with pytest.raises(IsADirectoryError):
        open(workdir)


def test_readdir_notadir(workdir):
    with pytest.raises(NotADirectoryError):
        filename = pjoin(workdir, FILENAME)
        with open(filename, 'wb'):
            os.listdir(filename)


def test_read(workdir):
    DATA = 'Hello'
    filename = pjoin(workdir, FILENAME)
    with open(filename, 'w') as fd:
        fd.write(DATA)
    with open(filename, 'r') as fd:
        data = fd.read()
        assert data == DATA
