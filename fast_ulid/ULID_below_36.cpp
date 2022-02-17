static PyObject *ulid_py(PyObject *self, PyObject *args, PyObject *kwargs) {
    PyDateTime_IMPORT;
    static char *keywords[] = {"timestamp", NULL};

    PyObject *arg = nullptr;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O", keywords, &arg)) {
        return NULL;
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

static PyObject *decode_datetime_py(PyObject *self, PyObject *args, PyObject *kwargs) {
    static char *keywords[] = {"ulid", NULL};

    const char *ulid;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s", keywords, &ulid)) {
        return NULL;
    }

    try {
        double timestamp = decodeTime(ulid);
        if (timestamp < 0) return NULL;
        auto datetime = PyDateTime_FromTimestamp(Py_BuildValue("(d, O)", timestamp, UTC_SINGLETON));
        if (datetime == NULL) {
            PyErr_SetString(PyExc_OverflowError, "Timestamp is too large to create `datetime.datetime`. Please use `decode_timestamp` instead.");
        }
        return datetime;
    } catch (std::runtime_error &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
    }
}

static PyObject *decode_timestamp_py(PyObject *self, PyObject *args, PyObject *kwargs) {
    static char *keywords[] = {"ulid", NULL};

    const char *ulid;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s", keywords, &ulid)) {
        return NULL;
    }

    try {
        double timestamp = decodeTime(ulid);
        if (timestamp < 0) return NULL;
        return Py_BuildValue("d", timestamp);
    } catch (std::runtime_error &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
    }
}
