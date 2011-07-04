"""check the installed Prosilica GigE SDK version number"""

import ctypes, sys

global _libprosilica
_libprosilica = None

ULong = ctypes.c_long # XXX should be unsigned long

def _load_libprosilica():
    """load the prosilica shared library"""
    global _libprosilica
    if _libprosilica is None:
        if sys.platform.startswith('linux'):
            _libprosilica = ctypes.cdll.LoadLibrary('libPvAPI.so')
        elif sys.platform.startswith('win'):
            _libprosilica = ctypes.CDLL('PvAPI.dll')
        else:
            raise NotImplementedError('unsupported platform')

        argtype = ctypes.POINTER(ULong)
        _libprosilica.PvVersion.argtypes = [argtype,argtype]

    return _libprosilica

def get_prosilica_version():
    """get the version number of the prosilica shared library"""
    libprosilica = _load_libprosilica()
    major = ULong()
    minor = ULong()
    libprosilica.PvVersion(ctypes.byref(major), ctypes.byref(minor))
    return major.value, minor.value

if __name__=='__main__':
    print 'you have Prosilica SDK %s.%s installed'%get_prosilica_version()
