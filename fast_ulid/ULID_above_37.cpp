static PyObject *ulid_py(PyObject *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames) {
    static char *keywords[] = {"timestamp", NULL};

    PyObject *arg = nullptr;
    if (nargs == 1){
        arg = args[0];
    } else if (kwnames) {
        if (PyTuple_Size(kwnames) == 1) {
            PyObject *pyArgName = PyTuple_GetItem(kwnames, 0);
            auto argName = PyUnicode_AsUTF8(pyArgName);
            if (strcmp(keywords[0], argName) != 0 ) {
                PyErr_SetString(PyExc_TypeError, "Invalid keyword argument for ulid()");
                return NULL;
            }
            arg = args[0];
        } else if (PyTuple_Size(kwnames) > 1) {
            PyErr_SetString(PyExc_TypeError, "ulid() takes at most 1 keyword argument");
            return NULL;
        }
    }

    if (arg == nullptr) {
        auto result = ULID(NULL);
        if (result == nullptr) {
            return NULL;
        }
        return Py_BuildValue("s", result);
    }

    double timestamp;
    if (PyDateTime_Check(arg)) {
        PyObject *py_timestamp = PyObject_CallMethod(arg, "timestamp", NULL);
        timestamp = PyFloat_AsDouble(py_timestamp);
        Py_DECREF(py_timestamp);
    } else if (PyFloat_Check(arg)) {
        timestamp = PyFloat_AsDouble(arg);
    } else {
        PyErr_SetString(PyExc_TypeError, "Argument must be a instance of `datetime.datetime` or `float`");
        return NULL;
    }

    auto result = ULID(timestamp * 1000);
    if (result == nullptr) {
        return NULL;
    }
    return Py_BuildValue("s", result);
}

static PyObject *decode_datetime_py(PyObject *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames) {
    static char *keywords[] = {"ulid", NULL};

    const char *ulid;

    PyObject *arg = nullptr;
    if (nargs == 1){
        arg = args[0];
    } else if (kwnames) {
        if (PyTuple_Size(kwnames) == 1) {
            PyObject *pyArgName = PyTuple_GetItem(kwnames, 0);
            auto argName = PyUnicode_AsUTF8(pyArgName);
            if (strcmp(keywords[0], argName) != 0 ) {
                PyErr_SetString(PyExc_TypeError, "Invalid keyword argument for decode_datetime()");
                return NULL;
            }
            PyArg_ParseTuple(*args, "s", &ulid);
        } else if (PyTuple_Size(kwnames) > 1) {
            PyErr_SetString(PyExc_TypeError, "decode_datetime() takes at most 1 keyword argument");
            return NULL;
        }
    }

    if (!PyUnicode_AsUTF8(arg)){
        PyErr_SetString(PyExc_TypeError, "Argument must be a instance of `str`");
        return NULL;
    }
    ulid = PyUnicode_AsUTF8(arg);

    try {
        double timestamp = decodeTime(ulid);
        if (timestamp < 0) return NULL;
        auto datetime = PyDateTime_FromTimestamp(Py_BuildValue("(d, O)", timestamp, PyDateTime_TimeZone_UTC));
        if (datetime == NULL) {
            PyErr_SetString(PyExc_OverflowError, "Timestamp is too large to create `datetime.datetime`. Please use `decode_timestamp` instead.");
        }
        return datetime;
    } catch (std::runtime_error &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
    }
}

static PyObject *decode_timestamp_py(PyObject *self, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames) {
    static char *keywords[] = {"ulid", NULL};

    const char *ulid;

    PyObject *arg = nullptr;
    if (nargs == 1){
        arg = args[0];
    } else if (kwnames) {
        if (PyTuple_Size(kwnames) == 1) {
            PyObject *pyArgName = PyTuple_GetItem(kwnames, 0);
            auto argName = PyUnicode_AsUTF8(pyArgName);
            if (strcmp(keywords[0], argName) != 0 ) {
                PyErr_SetString(PyExc_TypeError, "Invalid keyword argument for decode_datetime()");
                return NULL;
            }
            PyArg_ParseTuple(*args, "s", &ulid);
        } else if (PyTuple_Size(kwnames) > 1) {
            PyErr_SetString(PyExc_TypeError, "decode_datetime() takes at most 1 keyword argument");
            return NULL;
        }
    }

    if (!PyUnicode_AsUTF8(arg)){
        PyErr_SetString(PyExc_TypeError, "Argument must be a instance of `str`");
        return NULL;
    }
    ulid = PyUnicode_AsUTF8(arg);

    try {
        double timestamp = decodeTime(ulid);
        if (timestamp < 0) return NULL;
        return Py_BuildValue("d", timestamp);
    } catch (std::runtime_error &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
    }
}
