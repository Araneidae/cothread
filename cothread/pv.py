# A simple PV wrapper class.

import numpy

from . import cothread
from . import catools
from . import cadef

__all__ = ['PV', 'PV_waveform']


class PV(object):
    '''PV wrapper class.  Wraps access to a single PV as a persistent object
    with simple access methods.  Always contains the latest PV value.

    WARNING!  This API is a work in progress and will change in future releases
    in incompatible ways.'''

    def __init__(self, pv, on_update=None, **kargs):
        assert isinstance(pv, str), 'PV class only works for one PV at a time'

        self.name = pv
        self.__event = cothread.Event()
        self.__value = catools.ca_nothing(pv, cadef.ECA_DISCONN)
        self.monitor = catools.camonitor(pv, self.__on_update, **kargs)
        self.on_update = on_update

    def close(self):
        self.monitor.close()

    def __on_update(self, value):
        self.__value = value
        self.__event.Signal(value)
        if self.on_update:
            self.on_update(self)

    def get(self):
        '''Returns current value.'''
        return self.__value

    def get_next(self):
        '''Returns current value or blocks until next update.  Call .reset()
        first if more recent value required.'''
        return self.__event.Wait()

    def reset(self):
        '''Ensures .get_next() will block until an update occurs.'''
        self.__event.Reset()

    def put(self, value, **kargs):
        return catools.caput(self.name, value, **kargs)

    value = property(get, put)


class PV_waveform(object):
    '''PV waveform wrapper class.  Wraps access to a list of PVs as a single
    waveform with simple access methods.  This class will only work if all of
    the PVs are of the same datatype and the same length.

    WARNING!  This API is a work in progress and will change in future releases
    in incompatible ways.'''

    def __init__(self, pvs,
            dtype = float, count = 1, on_update = None, **kargs):

        assert not isinstance(pvs, str), \
            'PV_waveform class only works for an array of PVs'

        self.names = pvs
        self.on_update = on_update

        if count == 1:
            self.shape = len(pvs)
        else:
            self.shape = (len(pvs), count)
        self.__value = numpy.zeros(self.shape, dtype = dtype)
        self.__times = numpy.zeros(len(pvs), dtype = float)

        self.monitors = catools.camonitor(pvs,
            self.__on_update, count = count, datatype = dtype,
            format = catools.FORMAT_TIME, **kargs)

    def close(self):
        for monitor in self.monitors:
            monitor.close()

    def __on_update(self, value, index):
        self.__value[index] = value
        self.__times[index] = value.timestamp
        if self.on_update:
            self.on_update(self, index)

    def get(self):
        return self.__value

    def get_times(self):
        return self.__times

    def put(self, value, **kargs):
        return catools.caput(self.names, value, **kargs)

    value = property(get, put)
    times = property(get_times)
